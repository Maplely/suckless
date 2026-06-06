/* See LICENSE file for license details. */
#define _XOPEN_SOURCE 500
#if HAVE_SHADOW_H
#include <shadow.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>

#include "arg.h"
#include "util.h"

char *argv0;

enum {
	INIT,
	INPUT,
	FAILED,
	NUMCOLS
};

struct lock {
	int screen;
	Window root, win;
	Pixmap pmap;
	unsigned long colors[NUMCOLS];
};

struct xrandr {
	int active;
	int evbase;
	int errbase;
};

#include "config.h"

static void
die(const char *errstr, ...)
{
	va_list ap;
	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(1);
}

#ifdef __linux__
#include <fcntl.h>
#include <linux/oom.h>

static void
dontkillme(void)
{
	FILE *f;
	const char oomfile[] = "/proc/self/oom_score_adj";
	if (!(f = fopen(oomfile, "w"))) {
		if (errno == ENOENT)
			return;
		die("slock: fopen %s: %s\n", oomfile, strerror(errno));
	}
	fprintf(f, "%d", OOM_SCORE_ADJ_MIN);
	if (fclose(f)) {
		if (errno == EACCES)
			die("slock: unable to disable OOM killer. "
			    "Make sure to suid or sgid slock.\n");
		else
			die("slock: fclose %s: %s\n", oomfile, strerror(errno));
	}
}
#endif

static void
writedots(Display *dpy, Window win, unsigned int len, int screen)
{
	int s_width, s_height, i, dot_radius, dot_spacing, start_x, y;
	XGCValues gr_values;
	XColor col, dummy;
	GC gc;

	if (!XAllocNamedColor(dpy, DefaultColormap(dpy, screen),
	                      dot_color, &col, &dummy))
		return;
	gr_values.foreground = col.pixel;
	gc = XCreateGC(dpy, win, GCForeground, &gr_values);
	if (!gc)
		return;

	s_width = DisplayWidth(dpy, screen);
	s_height = DisplayHeight(dpy, screen);

	if (len > (unsigned int)max_dots)
		len = max_dots;

	dot_radius = 6;
	dot_spacing = 24;
	y = s_height * 3 / 5 + 40;
	start_x = s_width / 2 - (len * dot_spacing) / 2;

	for (i = 0; i < (int)len; i++)
		XFillArc(dpy, win, gc, start_x + i * dot_spacing,
		         y - dot_radius, dot_radius * 2, dot_radius * 2,
		         0, 360 * 64);

	XFreeColors(dpy, DefaultColormap(dpy, screen), &col.pixel, 1, 0);
	XFreeGC(dpy, gc);
}

static void
writetime(Display *dpy, Window win, int screen)
{
	time_t rawtime;
	struct tm *timeinfo;
	char timebuf[64];
	int s_width, s_height, x, y;
	XGlyphInfo ext;
	XftColor col;
	XftFont *font;
	XftDraw *draw;
	Visual *vis;
	Colormap cm;

	vis = DefaultVisual(dpy, screen);
	cm = DefaultColormap(dpy, screen);

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	if (!timeinfo)
		return;
	strftime(timebuf, sizeof(timebuf), "%H:%M:%S", timeinfo);

	font = XftFontOpen(dpy, screen,
	                   XFT_FAMILY, XftTypeString, clock_font,
	                   XFT_SIZE, XftTypeDouble, (double)clock_font_size,
	                   NULL);
	if (!font)
		return;

	if (!XftColorAllocName(dpy, vis, cm, clock_color, &col)) {
		XftFontClose(dpy, font);
		return;
	}

	draw = XftDrawCreate(dpy, win, vis, cm);
	if (!draw) {
		XftColorFree(dpy, vis, cm, &col);
		XftFontClose(dpy, font);
		return;
	}

	s_width = DisplayWidth(dpy, screen);
	s_height = DisplayHeight(dpy, screen);

	XftTextExtentsUtf8(dpy, font, (const FcChar8 *)timebuf,
	                   strlen(timebuf), &ext);
	x = (s_width - ext.xOff) / 2;
	y = s_height / 2;

	XftDrawStringUtf8(draw, &col, font, x, y,
	                  (const FcChar8 *)timebuf, strlen(timebuf));

	XftDrawDestroy(draw);
	XftColorFree(dpy, vis, cm, &col);
	XftFontClose(dpy, font);
}

static const char *
gethash(void)
{
	const char *hash;
	struct passwd *pw;

	errno = 0;
	if (!(pw = getpwuid(getuid()))) {
		if (errno)
			die("slock: getpwuid: %s\n", strerror(errno));
		else
			die("slock: cannot retrieve password entry\n");
	}
	hash = pw->pw_passwd;

#if HAVE_SHADOW_H
	if (!strcmp(hash, "x")) {
		struct spwd *sp;
		if (!(sp = getspnam(pw->pw_name)))
			die("slock: getspnam: cannot retrieve shadow entry. "
			    "Make sure to suid or sgid slock.\n");
		hash = sp->sp_pwdp;
	}
#else
	if (!strcmp(hash, "*")) {
#ifdef __OpenBSD__
		if (!(pw = getpwuid_shadow(getuid())))
			die("slock: getpwnam_shadow: cannot retrieve shadow entry. "
			    "Make sure to suid or sgid slock.\n");
		hash = pw->pw_passwd;
#else
		die("slock: getpwuid: cannot retrieve shadow entry. "
		    "Make sure to suid or sgid slock.\n");
#endif
	}
#endif

	return hash;
}

static void
readpw(Display *dpy, struct xrandr *rr, struct lock **locks, int nscreens,
       const char *hash)
{
	XRRScreenChangeNotifyEvent *rre;
	char buf[32], passwd[256], *inputhash;
	int num, screen, running;
	unsigned int len;
	KeySym ksym;
	XEvent ev;
	time_t last_time_draw;

	len = 0;
	running = 1;
	last_time_draw = 0;

	for (screen = 0; screen < nscreens; screen++)
		XClearWindow(dpy, locks[screen]->win);

	while (running) {
		while (XPending(dpy)) {
			XNextEvent(dpy, &ev);
			if (ev.type == KeyPress) {
				explicit_bzero(&buf, sizeof(buf));
				num = XLookupString(&ev.xkey, buf, sizeof(buf), &ksym, 0);
				if (IsKeypadKey(ksym)) {
					if (ksym == XK_KP_Enter) ksym = XK_Return;
					else if (ksym >= XK_KP_0 && ksym <= XK_KP_9)
						ksym = (ksym - XK_KP_0) + XK_0;
				}
				if (IsFunctionKey(ksym) || IsKeypadKey(ksym) ||
				    IsMiscFunctionKey(ksym) || IsPFKey(ksym) ||
				    IsPrivateKeypadKey(ksym))
					continue;
				switch (ksym) {
				case XK_Return:
					passwd[len] = '\0';
					errno = 0;
					if (!(inputhash = crypt(passwd, hash)))
						fprintf(stderr, "slock: crypt: %s\n", strerror(errno));
					else
						running = !!strcmp(inputhash, hash);
					explicit_bzero(&passwd, sizeof(passwd));
					len = 0;
					break;
				case XK_Escape:
					explicit_bzero(&passwd, sizeof(passwd));
					len = 0;
					break;
				case XK_BackSpace:
					if (len) {
						len--;
						passwd[len] = '\0';
						explicit_bzero(&passwd[len], sizeof(passwd) - len);
					}
					break;
				default:
					if (num && !iscntrl((unsigned char)buf[0]) &&
					    (len + num < sizeof(passwd))) {
						memcpy(passwd + len, buf, num);
						len += num;
					} else if (buf[0] == '\025') {
						explicit_bzero(&passwd, sizeof(passwd));
						len = 0;
					}
					break;
				}
				for (screen = 0; screen < nscreens; screen++) {
					XClearWindow(dpy, locks[screen]->win);
					writetime(dpy, locks[screen]->win, locks[screen]->screen);
					writedots(dpy, locks[screen]->win, len, locks[screen]->screen);
				}
				XFlush(dpy);
			} else if (rr->active && ev.type == rr->evbase + RRScreenChangeNotify) {
				rre = (XRRScreenChangeNotifyEvent*)&ev;
				for (screen = 0; screen < nscreens; screen++) {
					if (locks[screen]->win == rre->window) {
						if (rre->rotation == RR_Rotate_90 ||
						    rre->rotation == RR_Rotate_270)
							XResizeWindow(dpy, locks[screen]->win,
							              rre->height, rre->width);
						else
							XResizeWindow(dpy, locks[screen]->win,
							              rre->width, rre->height);
						XClearWindow(dpy, locks[screen]->win);
						break;
					}
				}
			} else {
				for (screen = 0; screen < nscreens; screen++)
					XRaiseWindow(dpy, locks[screen]->win);
			}
		}
		time_t now = time(NULL);
		if (now != last_time_draw) {
			last_time_draw = now;
			for (screen = 0; screen < nscreens; screen++) {
				XClearWindow(dpy, locks[screen]->win);
				writetime(dpy, locks[screen]->win, locks[screen]->screen);
				writedots(dpy, locks[screen]->win, len, locks[screen]->screen);
			}
			XFlush(dpy);
		}
		usleep(100000);
	}
}

static struct lock *
lockscreen(Display *dpy, struct xrandr *rr, int screen)
{
	char curs[] = {0, 0, 0, 0, 0, 0, 0, 0};
	int i, ptgrab, kbgrab;
	struct lock *lock;
	XColor color = {0}, dummy;
	XSetWindowAttributes wa;
	Cursor invisible;

	if (dpy == NULL || screen < 0 || !(lock = malloc(sizeof(struct lock))))
		return NULL;

	lock->screen = screen;
	lock->root = RootWindow(dpy, lock->screen);

	for (i = 0; i < NUMCOLS; i++) {
		if (!XAllocNamedColor(dpy, DefaultColormap(dpy, lock->screen),
		                      colorname[i], &color, &dummy))
			lock->colors[i] = BlackPixel(dpy, lock->screen);
		else
			lock->colors[i] = color.pixel;
		}

	wa.override_redirect = 1;
	wa.background_pixel = lock->colors[INIT];
	lock->win = XCreateWindow(dpy, lock->root, 0, 0,
	                          DisplayWidth(dpy, lock->screen),
	                          DisplayHeight(dpy, lock->screen),
	                          0, DefaultDepth(dpy, lock->screen),
	                          CopyFromParent,
	                          DefaultVisual(dpy, lock->screen),
	                          CWOverrideRedirect | CWBackPixel, &wa);
	XStoreName(dpy, lock->win, "slock");
	lock->pmap = XCreateBitmapFromData(dpy, lock->win, curs, 8, 8);
	if (!lock->pmap)
		lock->pmap = XCreatePixmap(dpy, lock->win, 8, 8, 1);
	invisible = XCreatePixmapCursor(dpy, lock->pmap, lock->pmap,
	                                &color, &color, 0, 0);
	if (invisible)
		XDefineCursor(dpy, lock->win, invisible);

	for (i = 0, ptgrab = kbgrab = -1; i < 6; i++) {
		if (ptgrab != GrabSuccess) {
			ptgrab = XGrabPointer(dpy, lock->root, False,
			                      ButtonPressMask | ButtonReleaseMask |
			                      PointerMotionMask, GrabModeAsync,
			                      GrabModeAsync, None, invisible, CurrentTime);
		}
		if (kbgrab != GrabSuccess) {
			kbgrab = XGrabKeyboard(dpy, lock->root, True,
			                       GrabModeAsync, GrabModeAsync, CurrentTime);
		}
		if (ptgrab == GrabSuccess && kbgrab == GrabSuccess) {
			XMapRaised(dpy, lock->win);
			if (rr->active)
				XRRSelectInput(dpy, lock->win, RRScreenChangeNotifyMask);
			XSelectInput(dpy, lock->root, SubstructureNotifyMask);
			return lock;
		}
		if ((ptgrab != AlreadyGrabbed && ptgrab != GrabSuccess) ||
		    (kbgrab != AlreadyGrabbed && kbgrab != GrabSuccess))
			break;
		usleep(100000);
	}

	if (ptgrab != GrabSuccess)
		fprintf(stderr, "slock: unable to grab mouse pointer for screen %d\n", screen);
	if (kbgrab != GrabSuccess)
		fprintf(stderr, "slock: unable to grab keyboard for screen %d\n", screen);
	return NULL;
}

static void
usage(void)
{
	die("usage: slock [-v] [cmd [arg ...]]\n");
}

int
main(int argc, char **argv) {
	struct xrandr rr;
	struct lock **locks;
	struct passwd *pwd;
	struct group *grp;
	uid_t duid;
	gid_t dgid;
	const char *hash;
	Display *dpy;
	int s, nlocks, nscreens;

	ARGBEGIN {
	case 'v':
		puts("slock-"VERSION);
		return 0;
	default:
		usage();
	} ARGEND

	errno = 0;
	if (!(pwd = getpwnam(user)))
		die("slock: getpwnam %s: %s\n", user,
		    errno ? strerror(errno) : "user entry not found");
	duid = pwd->pw_uid;
	errno = 0;
	if (!(grp = getgrnam(group)))
		die("slock: getgrnam %s: %s\n", group,
		    errno ? strerror(errno) : "group entry not found");
	dgid = grp->gr_gid;

#ifdef __linux__
	dontkillme();
#endif

	hash = gethash();
	errno = 0;
	if (!crypt("", hash))
		die("slock: crypt: %s\n", strerror(errno));

	if (!(dpy = XOpenDisplay(NULL)))
		die("slock: cannot open display\n");

	if (setgroups(0, NULL) < 0)
		die("slock: setgroups: %s\n", strerror(errno));
	if (setgid(dgid) < 0)
		die("slock: setgid: %s\n", strerror(errno));
	if (setuid(duid) < 0)
		die("slock: setuid: %s\n", strerror(errno));

	rr.active = XRRQueryExtension(dpy, &rr.evbase, &rr.errbase);

	nscreens = ScreenCount(dpy);
	if (!(locks = calloc(nscreens, sizeof(struct lock *))))
		die("slock: out of memory\n");
	for (nlocks = 0, s = 0; s < nscreens; s++) {
		if ((locks[s] = lockscreen(dpy, &rr, s)) != NULL)
			nlocks++;
		else
			break;
	}
	XSync(dpy, 0);

	if (nlocks != nscreens)
		return 1;

	if (argc > 0) {
		pid_t pid;
		extern char **environ;
		int err = posix_spawnp(&pid, argv[0], NULL, NULL, argv, environ);
		if (err)
			die("slock: failed to execute post-lock command: %s: %s\n",
			    argv[0], strerror(err));
		/* reap child to avoid zombie */
		int wstatus;
		waitpid(pid, &wstatus, WNOHANG);
	}

	readpw(dpy, &rr, locks, nscreens, hash);
	return 0;
}

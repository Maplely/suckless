static void
run_script(const char *path)
{
	pid_t pid = fork();
	if (pid == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execl("/bin/sh", "sh", "-c", path, NULL);
		_exit(EXIT_FAILURE);
	}
}

void
runautostart(void)
{
	char *pathpfx;
	char *path;
	char *xdgdatahome;
	char *home;
	struct stat sb;

	if ((home = getenv("HOME")) == NULL)
		return;

	xdgdatahome = getenv("XDG_DATA_HOME");
	if (xdgdatahome != NULL && *xdgdatahome != '\0') {
		pathpfx = ecalloc(1, strlen(xdgdatahome) + strlen(dwmdir) + 2);
		sprintf(pathpfx, "%s/%s", xdgdatahome, dwmdir);
	} else {
		pathpfx = ecalloc(1, strlen(home) + strlen(localshare)
							 + strlen(dwmdir) + 3);
		sprintf(pathpfx, "%s/%s/%s", home, localshare, dwmdir);
	}

	if (!(stat(pathpfx, &sb) == 0 && S_ISDIR(sb.st_mode))) {
		char *pathpfx_new = realloc(pathpfx, strlen(home) + strlen(dwmdir) + 3);
		if (pathpfx_new == NULL) {
			free(pathpfx);
			return;
		}
		pathpfx = pathpfx_new;
		sprintf(pathpfx, "%s/.%s", home, dwmdir);
	}

	path = ecalloc(1, strlen(pathpfx) + strlen(autostartblocksh) + 2);
	sprintf(path, "%s/%s", pathpfx, autostartblocksh);
	if (access(path, X_OK) == 0)
		run_script(path);

	sprintf(path, "%s/%s", pathpfx, autostartsh);
	if (access(path, X_OK) == 0)
		run_script(path);

	free(pathpfx);
	free(path);
}

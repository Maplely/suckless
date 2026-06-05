static int restart = 0;
static volatile sig_atomic_t sig_quit_requested = 0;
static volatile sig_atomic_t sig_quit_type = 0;

void
sighup(int unused)
{
	sig_quit_requested = 1;
	sig_quit_type = 1;
}

void
sigterm(int unused)
{
	sig_quit_requested = 1;
	sig_quit_type = 0;
}


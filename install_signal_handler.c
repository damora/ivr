#include <stdlib.h>
#include <signal.h>

static void handler(int sig)
{
   if (sig == SIGKILL) exit(-1);
}

void install_signal_handler(void)
{
   struct sigaction sa;
   sa.sa_handler = handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   sigaction(SIGKILL, &sa, NULL);
}

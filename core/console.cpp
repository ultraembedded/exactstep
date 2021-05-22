//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <signal.h>

#include "console.h"

//-----------------------------------------------------------------
// Locals
//-----------------------------------------------------------------
static struct sigaction _sigaction;
static struct termios   _term_settings;

//-----------------------------------------------------------------
// console_sigint_handler
//-----------------------------------------------------------------
static void console_sigint_handler(int s)
{
    // Jump to exit handler!
    exit(1);
}
//-----------------------------------------------------------------
// console_exit_handler
//-----------------------------------------------------------------
static void console_exit_handler(void)
{
    // Restore original terminal settings
    tcsetattr(fileno(stdin), TCSANOW, &_term_settings);
}
//-----------------------------------------------------------------
// console: Simple polled console
//-----------------------------------------------------------------
console::console()
{
    struct termios term;

    // Backup terminal settings
    tcgetattr(fileno(stdin), &_term_settings);

    // Don't buffer character entry
    memcpy(&term, &_term_settings, sizeof(struct termios));
    term.c_lflag &= ~(ECHO|ICANON);
    term.c_cc[VTIME] = 0;
    term.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &term);    

    // Catch SIGINT to restore terminal settings on exit
    signal(SIGINT, console_sigint_handler);

    // Register exit() handler
    atexit(console_exit_handler);
}
//-----------------------------------------------------------------
// putchar:
//-----------------------------------------------------------------
int console::putchar(int ch)
{
    fprintf(stderr, "%c", ch);
    return 0;
}
//-----------------------------------------------------------------
// getchar:
//-----------------------------------------------------------------
int console::getchar(void)
{
    char ch;
    if (read(STDIN_FILENO,&ch,1) == 1) 
        return ch;
    return -1;
}

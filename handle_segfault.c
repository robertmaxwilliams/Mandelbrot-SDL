/*
  Credits to:
    - https://linux.die.net/man/2/setcontext
    - https://stackoverflow.com/questions/8456085/why-cant-i-ignore-sigsegv-signal
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <setjmp.h>

bool debug_catch_sigsegv_mode = true;
static jmp_buf jbuf;

void debug_signal_handler(int signo)
{
    if(debug_catch_sigsegv_mode) {
        printf("\nsignal_handler: received %d\n", signo);
        siglongjmp(jbuf, 1);
    } else {
        printf("oops");
        signal(SIGSEGV, debug_signal_handler);
    }
}

void debug_enable_sigsev_handler() {
    if(signal(SIGSEGV, debug_signal_handler) == SIG_ERR) {
        printf("enable_sigsev_handler: ERROR, can't catch SIGSEGV\n");
    }
}

void debug_print_pointer(const void* ptr)
{
    long ptr_deref;

    printf("debug_print_pointer(%p) = ", ptr);
    debug_catch_sigsegv_mode = true;
    if(ptr != NULL && sigsetjmp(jbuf, !0) == 0) {
        ptr_deref = *(long*)ptr;
        printf("[%016lx]", ptr_deref);
    } else {
        printf("<my segmentation fault>");
    }
    debug_catch_sigsegv_mode = false;

    printf("\n");
}


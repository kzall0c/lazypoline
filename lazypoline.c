#include "lazypoline.h"

#include "sud.h"
#include "zpoline.h"
#include "signals.h"
#include <sysdeps/kernel_sigaction.h>
#include "gsreldata.h"

#include <syscall.h>
#include <sys/signal.h>
#include <sched.h>
#include <unistd.h>
#include <immintrin.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#ifndef CLONE_CLEAR_SIGHAND
#define CLONE_CLEAR_SIGHAND 0x100000000ULL
#endif

void init_lazypoline(void) {
    fprintf(stderr, "Initializing lazypoline!\n");

    init_sud();
#if REWRITE_TO_ZPOLINE
    init_zpoline();
#endif

    enable_sud();
    set_privilege_level(SYSCALL_DISPATCH_FILTER_BLOCK);
}

long handle_clone_thread(int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t a6);

static void do_postfork_handling(int64_t result) {
    if (result < 0) {
        /* error */
    } else if (result > 0) {
        /* parent */
    } else {
        enable_sud();
    }
}

long syscall_emulate(const int64_t syscall_no, int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t a6, uint64_t *const should_emulate) {
#if RETURN_IMMEDIATELY
    return 0;
#endif

    assert(*should_emulate == false);
#if PRINT_SYSCALLS
    fprintf(stderr, "\e[31m[%d] syscall(%s [%ld], 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx, 0x%lx)\e[m\n", getpid(), get_syscall_name(syscall_no), syscall_no, a1, a2, a3, a4, a5, a6);
#endif

    assert(syscall_no != __NR_unshare);

    if (syscall_no == __NR_clone3) {
        return -ENOSYS;
    }

    if (syscall_no == __NR_fork) {
        long result = inline_syscall6(syscall_no, a1, a2, a3, a4, a5, a6);
        do_postfork_handling(result);
        return result;
    }

    if (syscall_no == __NR_vfork) {
        *should_emulate = true;
        return __NR_vfork;
    }

    if (syscall_no == __NR_clone) {
        int64_t flags = a1;
        int64_t stack = a2;

        if (flags & CLONE_THREAD) {
            assert(flags & CLONE_VM);
            assert(!(flags & CLONE_VFORK));
            assert(!(flags & CLONE_CLEAR_SIGHAND));
            assert(stack);
            *should_emulate = true;
            return __NR_clone;
        } else if (flags & CLONE_VFORK) {
            assert(stack);
            assert(flags & CLONE_VM);
            assert(!(flags & CLONE_THREAD));
            assert(!(flags & CLONE_CLEAR_SIGHAND));
            *should_emulate = true;
            return __NR_clone;
        } else {
            assert((void *) stack == NULL);
            assert(!(flags & CLONE_CLEAR_SIGHAND));
            assert(!(flags & CLONE_THREAD) && !(flags & CLONE_DETACHED));
            assert(!(flags & CLONE_SIGHAND));
            assert(!(flags & CLONE_VFORK));
            assert(!(flags & CLONE_VM));
            long result = inline_syscall6(syscall_no, a1, a2, a3, a4, a5, a6);
            do_postfork_handling(result);
            return result;
        }

        assert(!"Unreachable");
    }

    if (syscall_no == __NR_rt_sigprocmask) {
        int how = a1;
        sigset_t *set = (sigset_t *) a2;
        int64_t sigsetsize = a4;
        assert(sigsetsize <= (long long) sizeof(sigset_t));
        assert(sigsetsize == SIGSETSIZE);

        char modifiable_mask[SIGSETSIZE];
        memset(modifiable_mask, 0, SIGSETSIZE);
        if (set && (how == SIG_BLOCK || how == SIG_SETMASK)) {
            memcpy(modifiable_mask, set, sigsetsize);
            ASSERT_ELSE_PERROR(sigdelset((sigset_t *) modifiable_mask, SIGSYS) == 0);
            a2 = (int64_t) &modifiable_mask[0];
        }

        long result = inline_syscall6(syscall_no, a1, a2, a3, a4, a5, a6);
        return result;
    }

    if (syscall_no == __NR_rt_sigaction) {
        int64_t result = signal_handlers_handle_app_sigaction(gsreldata->signal_handlers, a1, (const struct kernel_sigaction *) a2, (struct kernel_sigaction *) a3);
        return result;
    }

    if (syscall_no == __NR_rt_sigreturn) {
        *should_emulate = true;
        return __NR_rt_sigreturn;
    }

    if (syscall_no == __NR_exit) {
        teardown_thread_metadata();
    }

    return inline_syscall6(syscall_no, a1, a2, a3, a4, a5, a6);
}

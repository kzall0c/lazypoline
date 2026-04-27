# lazypoline

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.10372035.svg)](https://doi.org/10.5281/zenodo.10372035)

**lazypoline** is a fast, exhaustive, and expressive syscall interposer for user-space Linux applications. It uses a _hybrid interposition_ mechanism based on Syscall User Dispatch (SUD) and binary rewriting to exhaustively interpose all syscalls with maximal efficiency. You can find more details in our DSN'24 paper, ["System Call Interposition Without Compromise"](https://adriaanjacobs.github.io/files/dsn24lazypoline.pdf).

```bibtex
@inproceedings{jacobs2024lazypoline,
  title={System Call Interposition Without Compromise},
  author={Jacobs, Adriaan and G{\"u}lmez, Merve and Andries, Alicia and Volckaert, Stijn and Voulimeneas, Alexios},
  booktitle={54th Annual IEEE/IFIP International Conference on Dependable Systems and Networks (DSN)}, 
  year={2024},
  pages={183-194},
  doi={10.1109/DSN58291.2024.00030}
}
```

## Building
We use a plain Makefile. Clang is required (the code uses Clang-only builtins such as `__builtin_align_up`). From the project root:
```bash
make -j$(nproc)
```
Artifacts are placed in `build/`: `liblazypoline.so`, `libbootstrap.so`, `libsetup_thread.a`, and the `main` test binary. Run `make clean` to remove the build directory.

## Running
lazypoline can hook syscalls in precompiled binaries by setting the appropriate environment variables when launching. Example:
```bash
# from project source
LIBLAZYPOLINE="$(realpath build)/liblazypoline.so" LD_PRELOAD="$(realpath build)/libbootstrap.so" <some binary>
```

Note that this way of launching will miss syscalls performed before and while the dynamic loader loads lazypoline.
Just like zpoline, lazypoline requires permissions to `mmap` at low virtual addresses, i.e., the 0 page. You can permit this via:
```bash
echo 0 | sudo tee /proc/sys/vm/mmap_min_addr
```

## Example

Target program execution with out hooking:
```
$ ./main
Hello world!
Hello world!
Hello world!
Bye bye now!
Got a SIGFPE!
Got a SIGFPE!
Got a SIGFPE!
Got a SIGFPE!
Got a SIGFPE!
Got a SIGFPE!
Good times!
Got a SIGILL!
current thread time: 0s, 2045246ns
current thread time: 0s, 2076689ns
current thread time: 0s, 2079988ns
Current cpu: 7, current NUMA node: 0
[357587] Parent going to sleep!
[357588] Child going to sleep!
[357587] Parent woke up!
Hello from thread!
Hello from thread!
Hello from thread!
Hello from thread!
Hello from thread!
[357588] Child woke up!
```

With the hooking:
```
$ LIBLAZYPOLINE=./liblazypoline.so LD_PRELOAD=./libbootstrap.so ./main
Initializing lazypoline!
[358382] syscall(write [1], 0x2, 0x5af9d7023004, 0xd, 0x0, 0x0, 0x0)
Hello world!
[358382] syscall(write [1], 0x2, 0x5af9d7023004, 0xd, 0x0, 0x0, 0x0)
Hello world!
[358382] syscall(write [1], 0x2, 0x5af9d7023004, 0xd, 0x0, 0x0, 0x0)
Hello world!
[358382] syscall(write [1], 0x2, 0x5af9d7023012, 0xd, 0x0, 0x0, 0x0)
Bye bye now!
[358382] syscall(rt_sigaction [13], 0x8, 0x7ffe3be7d300, 0x0, 0x8, 0x0, 0x1)
[358382] syscall(rt_sigprocmask [14], 0x2, 0x0, 0x7ffe3be7d5b0, 0x8, 0x0, 0x1)
[358382] syscall(gettid [186], 0x7d038b8ee780, 0x8, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(getpid [39], 0x7d038b8ee780, 0x8, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(tgkill [234], 0x577ee, 0x577ee, 0x8, 0x0, 0x0, 0x1)
[358382] syscall(write [1], 0x2, 0x5af9d702328f, 0xe, 0x0, 0x0, 0x0)
Got a SIGFPE!
[358382] syscall(rt_sigreturn [15], 0x1, 0x7d038badc780, 0x7d038badc780, 0x0, 0xe, 0x1)
[358382] syscall(gettid [186], 0x7d038b8ee780, 0x8, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(getpid [39], 0x7d038b8ee780, 0x8, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(tgkill [234], 0x577ee, 0x577ee, 0x8, 0x0, 0x0, 0x1)
[358382] syscall(write [1], 0x2, 0x5af9d702328f, 0xe, 0x0, 0x0, 0x0)
Got a SIGFPE!
[358382] syscall(rt_sigreturn [15], 0x1, 0x7d038badc780, 0x7d038badc780, 0x0, 0xe, 0x1)
[358382] syscall(gettid [186], 0x7d038b8ee780, 0x8, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(getpid [39], 0x7d038b8ee780, 0x8, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(tgkill [234], 0x577ee, 0x577ee, 0x8, 0x0, 0x0, 0x1)
[358382] syscall(write [1], 0x2, 0x5af9d702328f, 0xe, 0x0, 0x0, 0x0)
Got a SIGFPE!
[358382] syscall(rt_sigreturn [15], 0x1, 0x7d038badc780, 0x7d038badc780, 0x0, 0xe, 0x1)
[358382] syscall(gettid [186], 0x7d038b8ee780, 0x8, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(getpid [39], 0x7d038b8ee780, 0x8, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(tgkill [234], 0x577ee, 0x577ee, 0x8, 0x0, 0x0, 0x1)
[358382] syscall(write [1], 0x2, 0x5af9d702328f, 0xe, 0x0, 0x0, 0x0)
Got a SIGFPE!
[358382] syscall(rt_sigreturn [15], 0x1, 0x7d038badc780, 0x7d038badc780, 0x0, 0xe, 0x1)
[358382] syscall(gettid [186], 0x7d038b8ee780, 0x8, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(getpid [39], 0x7d038b8ee780, 0x8, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(tgkill [234], 0x577ee, 0x577ee, 0x8, 0x0, 0x0, 0x1)
[358382] syscall(write [1], 0x2, 0x5af9d702328f, 0xe, 0x0, 0x0, 0x0)
Got a SIGFPE!
[358382] syscall(rt_sigreturn [15], 0x1, 0x7d038badc780, 0x7d038badc780, 0x0, 0xe, 0x1)
[358382] syscall(gettid [186], 0x7d038b8ee780, 0x8, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(getpid [39], 0x7d038b8ee780, 0x8, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(tgkill [234], 0x577ee, 0x577ee, 0x8, 0x0, 0x0, 0x1)
[358382] syscall(write [1], 0x2, 0x5af9d702328f, 0xe, 0x0, 0x0, 0x0)
Got a SIGFPE!
[358382] syscall(rt_sigreturn [15], 0x1, 0x7d038badc780, 0x7d038badc780, 0x0, 0xe, 0x1)
[358382] syscall(write [1], 0x2, 0x5af9d70230c0, 0xc, 0x0, 0x0, 0x0)
Good times!
[358382] syscall(rt_sigaction [13], 0x4, 0x7ffe3be7d300, 0x0, 0x8, 0x0, 0x1)
[358382] syscall(rt_sigaction [13], 0x4, 0x0, 0x7ffe3be7d3a0, 0x8, 0x7ffe3be7d510, 0x1)
[358382] syscall(write [1], 0x2, 0x5af9d70232db, 0xe, 0x0, 0x0, 0x0)
Got a SIGILL!
[358382] syscall(rt_sigreturn [15], 0x1, 0x7d038badc780, 0x7d038badc780, 0x0, 0xe, 0x1)
[358382] syscall(rt_sigaction [13], 0x4, 0x7ffe3be7d300, 0x0, 0x8, 0x0, 0x1)
[358382] syscall(rt_sigaction [13], 0x8, 0x7ffe3be7d300, 0x0, 0x8, 0x0, 0x1)
[358382] syscall(clock_gettime [228], 0x3, 0x7ffe3be7d450, 0x0, 0x0, 0x0, 0x1)
[358382] syscall(write [1], 0x2, 0x7ffe3be7d2a0, 0x23, 0x0, 0x0, 0x0)
current thread time: 0s, 4562495ns
[358382] syscall(clock_gettime [228], 0x3, 0x7ffe3be7d450, 0x0, 0x0, 0x23, 0x0)
[358382] syscall(write [1], 0x2, 0x7ffe3be7d2a0, 0x23, 0x0, 0x0, 0x0)
current thread time: 0s, 4619692ns
[358382] syscall(clock_gettime [228], 0x3, 0x7ffe3be7d450, 0x0, 0x0, 0x23, 0x0)
[358382] syscall(write [1], 0x2, 0x7ffe3be7d2a0, 0x23, 0x0, 0x0, 0x0)
current thread time: 0s, 4645447ns
[358382] syscall(write [1], 0x2, 0x7ffe3be7d2a0, 0x25, 0x0, 0x0, 0x0)
Current cpu: 4, current NUMA node: 0
[358382] syscall(rt_sigprocmask [14], 0x0, 0x7d038baa86d8, 0x7ffe3be7d2f0, 0x8, 0x25, 0x0)
[358382] syscall(clone [56], 0x1200011, 0x0, 0x0, 0x7d038b8eea50, 0x0, 0x0)
[358382] syscall(rt_sigprocmask [14], 0x2, 0x7ffe3be7d2f0, 0x0, 0x8, 0x7d038badbb60, 0x0)
[358382] syscall(getpid [39], 0x0, 0x0, 0x0, 0x0, 0x7d038badbb60, 0x0)
[358382] syscall(write [1], 0x2, 0x7ffe3be7d2a0, 0x20, 0x0, 0x0, 0x0)
[358382] Parent going to sleep!
[358382] syscall(clock_nanosleep [230], 0x0, 0x0, 0x7ffe3be7d420, 0x7ffe3be7d420, 0x0, 0x0)
[358383] syscall(set_robust_list [273], 0x7d038b8eea60, 0x18, 0x0, 0x7d038b8eea50, 0x0, 0x0)
[358383] syscall(rt_sigprocmask [14], 0x2, 0x7ffe3be7d2f0, 0x0, 0x8, 0x7d038badbb60, 0x0)
[358383] syscall(getpid [39], 0x0, 0x0, 0x0, 0x0, 0x7d038badbb60, 0x0)
[358383] syscall(write [1], 0x2, 0x7ffe3be7d2a0, 0x1f, 0x0, 0x0, 0x0)
[358383] Child going to sleep!
[358383] syscall(clock_nanosleep [230], 0x0, 0x0, 0x7ffe3be7d420, 0x7ffe3be7d420, 0x0, 0x0)
[358382] syscall(getpid [39], 0x0, 0x0, 0x0, 0x7ffe3be7d420, 0x0, 0x0)
[358382] syscall(write [1], 0x2, 0x7ffe3be7d2a0, 0x19, 0x0, 0x0, 0x0)
[358382] Parent woke up!
[358382] syscall(rt_sigaction [13], 0x21, 0x7ffe3be7d1b0, 0x0, 0x8, 0x0, 0x0)
[358382] syscall(rt_sigprocmask [14], 0x1, 0x7ffe3be7d378, 0x0, 0x8, 0x0, 0x0)
[358382] syscall(mmap [9], 0x0, 0x801000, 0x3, 0x20022, 0xffffffff, 0x0)
[358382] syscall(madvise [28], 0x7d038aee5000, 0x1000, 0x66, 0xffffffffffffffc0, 0xffffffff, 0x0)
[358382] syscall(rt_sigprocmask [14], 0x0, 0x7d038baa86d8, 0x7ffe3be7d368, 0x8, 0x7fff40, 0x120)
[358382] syscall(clone3 [435], 0x7ffe3be7d270, 0x58, 0x7d038b986b00, 0x8, 0x7d038b6e56c0, 0x7ffe3be7d367)
[358382] syscall(clone [56], 0x3d0f00, 0x7d038b6e4f30, 0x7d038b6e5990, 0x7d038b6e5990, 0x7d038b6e56c0, 0x7d038b6e56c0)
[358382] syscall(rt_sigprocmask [14], 0x2, 0x7ffe3be7d368, 0x0, 0x8, 0x7d038b6e56c0, 0x7d038b6e56c0)
[358382] syscall(futex [202], 0x7d038b6e5990, 0x109, 0x577f0, 0x0, 0x0, 0xffffffff)
[358382] syscall(rseq [334], 0x7d038b6e55c0, 0x21, 0x0, 0x53053053, 0x80, 0x21)
[358382] syscall(set_robust_list [273], 0x7d038b6e59a0, 0x18, 0x0, 0x53053053, 0x80, 0x21)
[358382] syscall(rt_sigprocmask [14], 0x2, 0x7d038b6e5fb0, 0x0, 0x8, 0x0, 0x21)
[358382] syscall(write [1], 0x2, 0x5af9d70232ea, 0x13, 0x0, 0x0, 0x0)
Hello from thread!
[358382] syscall(clock_nanosleep [230], 0x0, 0x0, 0x7d038b6e4e20, 0x7d038b6e4e20, 0x0, 0x0)
[358382] syscall(write [1], 0x2, 0x5af9d70232ea, 0x13, 0x0, 0x0, 0x0)
Hello from thread!
[358382] syscall(clock_nanosleep [230], 0x0, 0x0, 0x7d038b6e4e20, 0x7d038b6e4e20, 0x0, 0x0)
[358382] syscall(write [1], 0x2, 0x5af9d70232ea, 0x13, 0x0, 0x0, 0x0)
Hello from thread!
[358382] syscall(clock_nanosleep [230], 0x0, 0x0, 0x7d038b6e4e20, 0x7d038b6e4e20, 0x0, 0x0)
[358382] syscall(write [1], 0x2, 0x5af9d70232ea, 0x13, 0x0, 0x0, 0x0)
Hello from thread!
[358382] syscall(clock_nanosleep [230], 0x0, 0x0, 0x7d038b6e4e20, 0x7d038b6e4e20, 0x0, 0x0)
[358382] syscall(write [1], 0x2, 0x5af9d70232ea, 0x13, 0x0, 0x0, 0x0)
Hello from thread!
[358382] syscall(clock_nanosleep [230], 0x0, 0x0, 0x7d038b6e4e20, 0x7d038b6e4e20, 0x0, 0x0)
[358383] syscall(getpid [39], 0x0, 0x0, 0x0, 0x7ffe3be7d420, 0x0, 0x0)
[358383] syscall(write [1], 0x2, 0x7ffe3be7d2a0, 0x18, 0x0, 0x0, 0x0)
[358383] Child woke up!
[358383] syscall(exit_group [231], 0x0, 0xffffffffffffffa0, 0x7d038b8eea88, 0x7ffe3be7d280, 0x0, 0x0)
[358382] syscall(rt_sigprocmask [14], 0x0, 0x7d038b6e5fb0, 0x0, 0x8, 0x0, 0x0)
[358382] syscall(madvise [28], 0x7d038aee5000, 0x7fb000, 0x4, 0x8, 0x0, 0x0)
[358382] syscall(exit [60], 0x0, 0x801000, 0x0, 0x8, 0x0, 0x0)
[358382] syscall(wait4 [61], 0xffffffffffffffff, 0x0, 0x0, 0x0, 0x0, 0x0)
[358382] syscall(exit_group [231], 0x0, 0xffffffffffffffa0, 0x7d038b8eea88, 0x7ffe3be7d480, 0x0, 0x0)
```

## Extending
You can modify lazypoline to better fit your needs. `syscall_emulate` in [lazypoline.cpp](/lazypoline.cpp) is your main entry point. 

We recommend including lazypoline into your application through the small [bootstrap](/bootstrap_runtime.cpp), which uses `dlmopen` to load the main lazypoline library in a new dynamic library namespace. This ensures that the interposer links to a separate copy of all libraries it uses, instead of re-using those from the application it is interposing. Application libraries may have re-written syscalls, which would lead to recursive interposer invocations. In addition, many library functions that perform syscalls are not re-entrancy safe, which can lead to hard-to-diagnose bugs when they are invoked from the interposer. 

## Debugging and testing
We include a `main` binary that contains a number of testcases for lazypoline, e.g., multi-threading/multi-processing, signal delivery during application/interposer execution, etc. You can run this binary in the normal way, i.e., by setting the `LIBLAZYPOLINE` and `LD_PRELOAD` environment variables. To run under the debugger, we typically set these variables through the `env` binary, e.g.: 

```bash
# from the project root
gdb --args env LIBLAZYPOLINE=./build/liblazypoline.so LD_PRELOAD=./build/libbootstrap.so ./build/main 
```
When debugging with gdb, gdb will pause execution whenever SUD intercepts a system call. You can manually continue each time by entering the `c`(continue) command, or permanently disable gdb from stopping on this signal by executing:

```bash
handle SIGSYS nostop noprint
```
## Configuration
[config.h](/config.h) contains some options to control lazypoline's behavior. Most are self-explanatory. Enable `COMPAT_NONDEP_APP` to restore page permissions to `RWX` instead of `RX` (default). Some old JIT engines, like [`tcc`](https://bellard.org/tcc/) require this.

## Compatibility
lazypoline is well-tested on Ubuntu 20.04 and 22.04. Its primary compatibility requirement is kernel version >= 5.11 (needs SUD). In addition, the `SIGSETSIZE` in [util.h](/util.h) and everything under [sysdeps/](/sysdeps/) should be kept in sync with whatever glibc version you're running. 

## Related DSN 2024 artifacts
You can find the benchmarks for the performance evaluation in the DSN paper at [https://github.com/lazypoline/benchmarks](https://github.com/lazypoline/benchmarks). The Pintool to track application's register preservation expectations across syscalls is located at [https://github.com/lazypoline/pintool-syscall-abi-expectations](https://github.com/lazypoline/pintool-syscall-abi-expectations). 

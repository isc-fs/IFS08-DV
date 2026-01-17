#include <sys/time.h>

extern "C" {

int __cxa_guard_acquire(int* guard) { return !(*guard); }
void __cxa_guard_release(int* guard) { *guard = 1; }
void __cxa_guard_abort(int*) {}

int _gettimeofday(struct timeval *tv, void *tz) {
    (void)tz;
    if (tv) {
        tv->tv_sec = 0;
        tv->tv_usec = 0;
    }
    return 0;
}

}

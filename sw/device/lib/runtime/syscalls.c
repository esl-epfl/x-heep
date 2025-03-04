/* An extremely minimalist syscalls.c for newlib
 * Based on riscv newlib libgloss/riscv/sys_*.c
 *
 * Copyright 2019 Clifford Wolf
 * Copyright 2019 ETH Zürich and University of Bologna
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/stat.h>
#include <string.h> 
#include <sys/reent.h>
#include <newlib.h>
#include <unistd.h>
#include <reent.h>
#include <errno.h>
#include "uart.h"
#include "soc_ctrl.h"
#include "core_v_mini_mcu.h"
#include "error.h"
#include "x-heep.h"
#include <stdio.h>

#undef errno
extern int errno;

#define STDOUT_FILENO 1

#ifndef _LIBC
/* Provide prototypes for most of the _<systemcall> names that are
   provided in newlib for some compilers.  */
int     _close (int __fildes);
pid_t   _fork (void);
pid_t   _getpid (void);
int     _isatty (int __fildes);
int     _link (const char *__path1, const char *__path2);
_off_t  _lseek (int __fildes, _off_t __offset, int __whence);
ssize_t _read (int __fd, void *__buf, size_t __nbyte);
void *  _sbrk (ptrdiff_t __incr);
int     _brk(void *addr);
int     _unlink (const char *__path);
ssize_t _write (int __fd, const void *__buf, size_t __nbyte);
int     _execve (const char *__path, char * const __argv[], char * const __envp[]);
int     _kill (pid_t pid, int sig);
#endif


void unimplemented_syscall()
{
    const char *p = "Unimplemented system call called!\n";
    _write(STDOUT_FILENO, p, strlen(p));
}

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
    errno = ENOSYS;
    return -1;
}

int _access(const char *file, int mode)
{
    errno = ENOSYS;
    return -1;
}

int _chdir(const char *path)
{
    errno = ENOSYS;
    return -1;
}

int _chmod(const char *path, mode_t mode)
{
    errno = ENOSYS;
    return -1;
}

int _chown(const char *path, uid_t owner, gid_t group)
{
    errno = ENOSYS;
    return -1;
}

int _close(int file)
{
    return -1;
}

int _execve(const char *name, char *const argv[], char *const env[])
{
    errno = ENOMEM;
    return -1;
}

void _exit(int exit_status)
{
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
    soc_ctrl_set_exit_value(&soc_ctrl, exit_status);
    soc_ctrl_set_valid(&soc_ctrl, (uint8_t)1);

    asm volatile("wfi");
}

int _faccessat(int dirfd, const char *file, int mode, int flags)
{
    errno = ENOSYS;
    return -1;
}

pid_t _fork(void)
{
    errno = EAGAIN;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
    // errno = -ENOSYS;
    // return -1;
}

int _fstatat(int dirfd, const char *file, struct stat *st, int flags)
{
    errno = ENOSYS;
    return -1;
}

int _ftime(struct timeb *tp)
{
    errno = ENOSYS;
    return -1;
}

char *_getcwd(char *buf, size_t size)
{
    errno = -ENOSYS;
    return NULL;
}

pid_t _getpid()
{
    return 1;
}

int _gettimeofday(struct timeval *tp, void *tzp)
{
    errno = -ENOSYS;
    return -1;
}

int _isatty(int file)
{
    return (file == STDOUT_FILENO);
}

int _kill(pid_t pid, int sig)
{
    errno = EINVAL;
    return -1;
}

int _link(const char *old_name, const char *new_name)
{
    errno = EMLINK;
    return -1;
}

off_t _lseek(int file, off_t ptr, int dir)
{
    return 0;
}

int _lstat(const char *file, struct stat *st)
{
    errno = ENOSYS;
    return -1;
}

int _open(const char *name, int flags, int mode)
{
    return -1;
}

int _openat(int dirfd, const char *name, int flags, int mode)
{
    errno = ENOSYS;
    return -1;
}

ssize_t _read(int file, void *ptr, size_t len)
{
    return 0;
}

int _stat(const char *file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
    // errno = ENOSYS;
    // return -1;
}

long _sysconf(int name)
{

    return -1;
}

clock_t _times(struct tms *buf)
{
    return -1;
}

int _unlink(const char *name)
{
    errno = ENOENT;
    return -1;
}

int _utime(const char *path, const struct utimbuf *times)
{
    errno = ENOSYS;
    return -1;
}

int _wait(int *status)
{
    errno = ECHILD;
    return -1;
}

ssize_t _write(int file, const void *ptr, size_t len)
{
    if (file != STDOUT_FILENO) {
        errno = ENOSYS;
        return -1;
    }

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);

    uart_t uart;
    uart.base_addr   = mmio_region_from_addr((uintptr_t)UART_START_ADDRESS);
    uart.baudrate    = UART_BAUDRATE;
    uart.clk_freq_hz = soc_ctrl_get_frequency(&soc_ctrl);

    if (uart_init(&uart) != kErrorOk) {
        errno = ENOSYS;
        return -1;
    }
    return uart_write(&uart,(uint8_t *)ptr,len);
}


_ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t cnt)
{
    return _write(fd,buf,cnt);
}

extern char __heap_start[];
extern char __heap_end[];
static char *brk = __heap_start;

int _brk(void *addr)
{
    if (addr >= (void *)__heap_start && addr <= (void *)__heap_end) {
        brk = addr;
        return 0; 
    } else {
        return -1; 
    }
}

void *_sbrk(ptrdiff_t incr)
{
    char *old_brk = brk;

    if (__heap_start == __heap_end) {
        return NULL; 
    }

    if (brk + incr < __heap_end && brk + incr >= __heap_start) {
        brk += incr;
    } else {
        return (void *)-1; 
    }
    return old_brk;
}

int raise(int sig)
{
    return _kill(_getpid(), sig);
}

void abort(void)
{
    _exit(-1);
}

#ifdef __cplusplus
}
#endif
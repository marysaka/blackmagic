/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2016  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Provides an abstract 'target object', the 'methods' of which must be
 * implemented by a target driver when a supported device is detected.
 */

#ifndef INCLUDE_TARGET_H
#define INCLUDE_TARGET_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#if defined(_MSC_VER)
#include <basetsd.h>
typedef SSIZE_T ssize_t;
typedef int32_t mode_t;
#endif /* _MSC_VER */

typedef struct target target_s;
typedef uint32_t target_addr32_t;
typedef uint64_t target_addr64_t;
typedef target_addr32_t target_addr_t;
typedef struct target_controller target_controller_s;

#if CONFIG_BMDA == 1
bool bmda_swd_scan(uint32_t targetid);
bool bmda_jtag_scan(void);
#endif
bool adiv5_swd_scan(uint32_t targetid);
bool jtag_scan(void);

size_t target_foreach(void (*callback)(size_t index, target_s *target, void *context), void *context);
target_s *target_list_get_last();
void target_list_free(void);

target_s *target_new(void);

/* Attach/detach functions */
target_s *target_attach(target_s *target, target_controller_s *controller);
target_s *target_attach_n(size_t n, target_controller_s *controller);
void target_detach(target_s *target);

/* Memory access functions */
bool target_mem_map(target_s *target, char *buf, size_t len);
bool target_mem32_read(target_s *target, void *dest, target_addr_t src, size_t len);
bool target_mem64_read(target_s *target, void *dest, target_addr64_t src, size_t len);
bool target_mem32_write(target_s *target, target_addr_t dest, const void *src, size_t len);
bool target_mem64_write(target_s *target, target_addr64_t dest, const void *src, size_t len);
bool target_mem_access_needs_halt(target_s *target);
/* Flash memory access functions */
bool target_flash_erase(target_s *target, target_addr_t addr, size_t len);
bool target_flash_write(target_s *target, target_addr_t dest, const void *src, size_t len);
bool target_flash_complete(target_s *target);
bool target_flash_mass_erase(target_s *target);

/* Register access functions */
size_t target_regs_size(target_s *target);
const char *target_regs_description(target_s *target);
void target_regs_read(target_s *target, void *data);
void target_regs_write(target_s *target, const void *data);
size_t target_reg_read(target_s *target, uint32_t reg, void *data, size_t max);
size_t target_reg_write(target_s *target, uint32_t reg, const void *data, size_t size);

/* Halt/resume functions */
typedef enum target_halt_reason {
	TARGET_HALT_RUNNING = 0, /* Target not halted */
	TARGET_HALT_ERROR,       /* Failed to read target status */
	TARGET_HALT_REQUEST,
	TARGET_HALT_STEPPING,
	/*
	 * Used to both indicate that the target hit a breakpoint, and to
	 * indicate that the target hit a watchpoint but we can't figure out which
	 */
	TARGET_HALT_BREAKPOINT,
	/* Used to indicate the target hit a watchpoint and we know which */
	TARGET_HALT_WATCHPOINT,
	TARGET_HALT_FAULT,
} target_halt_reason_e;

void target_reset(target_s *target);
void target_halt_request(target_s *target);
target_halt_reason_e target_halt_poll(target_s *target, target_addr64_t *watch);
void target_halt_resume(target_s *target, bool step);
void target_set_cmdline(target_s *target, const char *cmdline, size_t cmdline_len);
void target_set_heapinfo(target_s *target, target_addr_t heap_base, target_addr_t heap_limit, target_addr_t stack_base,
	target_addr_t stack_limit);

/* Break-/watchpoint functions */
typedef enum target_breakwatch {
	TARGET_BREAK_SOFT,
	TARGET_BREAK_HARD,
	TARGET_WATCH_WRITE,
	TARGET_WATCH_READ,
	TARGET_WATCH_ACCESS,
} target_breakwatch_e;

int target_breakwatch_set(target_s *target, target_breakwatch_e type, target_addr_t addr, size_t len);
int target_breakwatch_clear(target_s *target, target_breakwatch_e type, target_addr_t addr, size_t len);

/* Command interpreter */
void target_command_help(target_s *target);
int target_command(target_s *target, int argc, const char *argv[]);

/* Defined per GDB's File I/O errno values from gdbsupport/fileio.h */
typedef enum semihosting_errno {
	TARGET_SUCCESS = 0,
	TARGET_EPERM = 1,
	TARGET_ENOENT = 2,
	TARGET_EINTR = 4,
	TARGET_EIO = 5,
	TARGET_EBADF = 9,
	TARGET_EACCES = 13,
	TARGET_EFAULT = 14,
	TARGET_EBUSY = 16,
	TARGET_EEXIST = 17,
	TARGET_ENODEV = 19,
	TARGET_ENOTDIR = 20,
	TARGET_EISDIR = 21,
	TARGET_EINVAL = 22,
	TARGET_ENFILE = 23,
	TARGET_EMFILE = 24,
	TARGET_EFBIG = 27,
	TARGET_ENOSPC = 28,
	TARGET_ESPIPE = 29,
	TARGET_EROFS = 30,
	TARGET_ENOSYS = 88,
	TARGET_ENAMETOOLONG = 91,
	TARGET_EUNKNOWN = 9999,
} semihosting_errno_e;

struct target_controller {
	void (*destroy_callback)(target_controller_s *, target_s *target);
	void (*printf)(target_controller_s *, const char *fmt, va_list);

	void *semihosting_buffer_ptr;
	size_t semihosting_buffer_len;
	semihosting_errno_e gdb_errno;
	bool interrupted;
};

#endif /* INCLUDE_TARGET_H */

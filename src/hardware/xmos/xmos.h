/*
 * This file is part of the libsigrok project.
 *
 * Copyright (C) 2014-2017 Kumar Abhishek <abhishek@theembeddedkitchen.net>
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

#ifndef XMOS_H_
#define XMOS_H_

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

/* For all the functions below:
 * Parameters:
 * 	devc : Device context structure to operate on
 * Returns:
 * 	SR_OK or SR_ERR
 */

typedef struct {
    uint64_t timestamp;
    uint64_t dataval;
    int64_t  host_time;
} xmos_record_t;

typedef struct {
	void* data;
	void* last_data;
} xmos_iter_t;

struct xmos_ops {
	int (*open)(struct dev_context *devc);
	int (*close)(struct dev_context *devc);

	int (*detect)(struct dev_context *devc);

	void (*xmos_record_iter_init)(xmos_iter_t* iter, unsigned channel);
	void (*xmos_record_iter_done)(xmos_iter_t* iter, unsigned channel);
	void (*xmos_record_iter_undonext)(xmos_iter_t* iter);
	int  (*xmos_record_iter_next)(xmos_iter_t* iter, unsigned channel, xmos_record_t** record);
};

SR_PRIV extern const struct xmos_ops xmos_tcp_ops;

#endif

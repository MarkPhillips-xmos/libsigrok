/*
 * This file is part of the libsigrok project.
 *
 * Copyright (C) 2010 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Olivier Fauchon <olivier@aixmarseille.com>
 * Copyright (C) 2012 Alexandru Gagniuc <mr.nuke.me@gmail.com>
 * Copyright (C) 2015 Bartosz Golaszewski <bgolaszewski@baylibre.com>
 * Copyright (C) 2019 Frank Stettner <frank-stettner@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <libsigrok/libsigrok.h>
#include "libsigrok-internal.h"
#include "protocol.h"
#include "xmos.h"

#define ANALOG_SAMPLES_PER_PERIOD 20

#if 0
static const uint8_t pattern_sigrok[] = {
	0x4c, 0x92, 0x92, 0x92, 0x64, 0x00, 0x00, 0x00,
	0x82, 0xfe, 0xfe, 0x82, 0x00, 0x00, 0x00, 0x00,
	0x7c, 0x82, 0x82, 0x92, 0x74, 0x00, 0x00, 0x00,
	0xfe, 0x12, 0x12, 0x32, 0xcc, 0x00, 0x00, 0x00,
	0x7c, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x00,
	0xfe, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xbe, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t pattern_squid[128][128 / 8] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0xe0, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xe1, 0x01, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xe1, 0x01, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xe3, 0x03, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe3, 0x03, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xc3, 0x03, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcf, 0xc7, 0x03, },
	{ 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8f, 0xc7, 0x03, },
	{ 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8f, 0x87, 0x03, },
	{ 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8f, 0xc7, 0x03, },
	{ 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8f, 0xcf, 0x03, },
	{ 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xcf, 0x03, },
	{ 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x03, },
	{ 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0xff, 0x03, },
	{ 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0xfe, 0x01, },
	{ 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0xfe, 0x01, },
	{ 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xfc, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0xc0, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x80, 0x01, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0xfe, 0xff, 0x03, },
	{ 0x00, 0x00, 0x07, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xfe, 0xff, 0x03, },
	{ 0x00, 0x00, 0x1c, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xfe, 0xff, 0x03, },
	{ 0x00, 0x00, 0x78, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0xfe, 0xff, 0x03, },
	{ 0x00, 0x00, 0xe0, 0x00, 0x80, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0xfe, 0xff, 0x03, },
	{ 0x00, 0x00, 0xc0, 0x03, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x07, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x1c, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0xf8, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, },
	{ 0x00, 0x00, 0x00, 0xf0, 0x01, 0x38, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x01, 0x00, 0xf0, 0x1f, 0x1c, },
	{ 0x00, 0x00, 0x00, 0xe0, 0x07, 0x70, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x0f, 0x00, 0xfc, 0x3f, 0x3c, },
	{ 0x80, 0x03, 0x00, 0xc0, 0x0f, 0xe0, 0x00, 0x00, 0x80, 0xff, 0xff, 0x3f, 0x00, 0xfc, 0x7f, 0x7c, },
	{ 0x00, 0x1e, 0x00, 0x00, 0x1f, 0xc0, 0x01, 0x00, 0xc0, 0xff, 0xff, 0x7f, 0x00, 0xfe, 0xff, 0x7c, },
	{ 0x00, 0xf0, 0x01, 0x00, 0x7c, 0x80, 0x03, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x00, 0xfe, 0xff, 0x7c, },
	{ 0x00, 0xc0, 0x0f, 0x00, 0xf0, 0x00, 0x07, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x00, 0x3f, 0xf8, 0x78, },
	{ 0x00, 0x00, 0x3e, 0x00, 0xc0, 0x03, 0x0e, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x00, 0x0f, 0xf0, 0xf0, },
	{ 0x00, 0x00, 0xf0, 0x07, 0x80, 0x07, 0x3c, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x00, 0x0f, 0xf0, 0xf0, },
	{ 0x00, 0x00, 0x80, 0x3f, 0x00, 0x1e, 0x78, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x00, 0x0f, 0xe0, 0xf0, },
	{ 0x00, 0x00, 0x00, 0xff, 0x00, 0x7c, 0xe0, 0x01, 0xe0, 0xff, 0xff, 0xff, 0x00, 0x0f, 0xe0, 0xf0, },
	{ 0x00, 0x00, 0x00, 0xfc, 0x03, 0xf0, 0xc1, 0x07, 0xf0, 0xff, 0xff, 0xff, 0x00, 0x0f, 0xf0, 0xf0, },
	{ 0x00, 0x00, 0x00, 0xe0, 0x1f, 0xc0, 0x03, 0x1f, 0xe0, 0xff, 0xff, 0xff, 0x00, 0x0f, 0xf0, 0x78, },
	{ 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x1f, 0xfc, 0xe0, 0xff, 0xff, 0xff, 0x01, 0xff, 0xff, 0x7f, },
	{ 0xf8, 0x03, 0x00, 0x00, 0xf0, 0x07, 0xfe, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x01, 0xff, 0xff, 0x7f, },
	{ 0x80, 0xff, 0x01, 0x00, 0x80, 0x3f, 0xf0, 0x8f, 0xff, 0xff, 0xff, 0xff, 0x00, 0xfe, 0xff, 0x3f, },
	{ 0x00, 0xf0, 0xff, 0x07, 0x00, 0xfc, 0x83, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xfe, 0xff, 0x3f, },
	{ 0x00, 0x00, 0xfc, 0x7f, 0x00, 0xe0, 0x7f, 0xfc, 0xff, 0xff, 0xff, 0xff, 0x00, 0xfe, 0xff, 0x1f, },
	{ 0x00, 0x00, 0xc0, 0xff, 0x1f, 0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xfc, 0xff, 0x03, },
	{ 0x00, 0x00, 0x00, 0xf8, 0xff, 0x07, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x0f, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xff, 0xff, 0x03, },
	{ 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xff, 0xff, 0x03, },
	{ 0x00, 0x10, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x03, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x03, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x03, },
	{ 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x0f, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x00, 0x7c, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0xf8, 0xff, 0x07, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x3c, 0x00, 0x00, },
	{ 0x00, 0x00, 0xc0, 0xff, 0x1f, 0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x1e, 0x00, 0x00, },
	{ 0x00, 0x00, 0xf8, 0x7f, 0x00, 0xe0, 0x7f, 0xfc, 0xff, 0xff, 0xff, 0xff, 0x00, 0x1e, 0x00, 0x00, },
	{ 0x00, 0xf0, 0xff, 0x07, 0x00, 0xfc, 0x83, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x0f, 0x00, 0x00, },
	{ 0x80, 0xff, 0x01, 0x00, 0x80, 0x3f, 0xf0, 0x8f, 0xff, 0xff, 0xff, 0xff, 0x01, 0x0f, 0x00, 0x00, },
	{ 0xf8, 0x03, 0x00, 0x00, 0xf0, 0x07, 0xfe, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0f, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x1f, 0xfc, 0xe0, 0xff, 0xff, 0xff, 0x00, 0x0f, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0xe0, 0x1f, 0xc0, 0x07, 0x1f, 0xf0, 0xff, 0xff, 0xff, 0x00, 0x06, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0xfc, 0x03, 0xf0, 0xc1, 0x07, 0xf0, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0xff, 0x00, 0x7c, 0xe0, 0x01, 0xf0, 0xff, 0xff, 0xff, 0x01, 0xe0, 0x1f, 0x00, },
	{ 0x00, 0x00, 0x80, 0x3f, 0x00, 0x1e, 0x78, 0x00, 0xf0, 0xff, 0xff, 0xff, 0x01, 0xf0, 0x7f, 0x00, },
	{ 0x00, 0x00, 0xf0, 0x0f, 0x80, 0x07, 0x3c, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x01, 0xfc, 0xff, 0x00, },
	{ 0x00, 0x00, 0x3e, 0x00, 0xc0, 0x03, 0x0e, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x01, 0xfc, 0xff, 0x01, },
	{ 0x00, 0xc0, 0x0f, 0x00, 0xf0, 0x00, 0x07, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x01, 0xfe, 0xff, 0x01, },
	{ 0x00, 0xf0, 0x01, 0x00, 0x7c, 0x80, 0x03, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x03, },
	{ 0x00, 0x3e, 0x00, 0x00, 0x1f, 0xc0, 0x01, 0x00, 0x80, 0xff, 0xff, 0xff, 0x00, 0x1f, 0xe0, 0x03, },
	{ 0x80, 0x03, 0x00, 0xc0, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x0f, 0xc0, 0x03, },
	{ 0x00, 0x00, 0x00, 0xe0, 0x07, 0x70, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0x00, 0x0f, 0xc0, 0x03, },
	{ 0x00, 0x00, 0x00, 0xf0, 0x01, 0x38, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x01, 0x00, 0x0f, 0x80, 0x03, },
	{ 0x00, 0x00, 0x00, 0xf8, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x03, },
	{ 0x00, 0x00, 0x00, 0x1c, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x03, },
	{ 0x00, 0x00, 0x00, 0x07, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x03, },
	{ 0x00, 0x00, 0xc0, 0x03, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf8, 0x03, },
	{ 0x00, 0x00, 0xe0, 0x00, 0x80, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x01, },
	{ 0x00, 0x00, 0x78, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x01, },
	{ 0x00, 0x00, 0x1c, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0x00, },
	{ 0x00, 0x00, 0x06, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x7f, 0x00, },
	{ 0x00, 0x80, 0x03, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1f, 0x00, },
	{ 0x00, 0xc0, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x01, },
	{ 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x03, },
	{ 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x03, },
	{ 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x03, },
	{ 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x03, },
	{ 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x03, },
	{ 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0x00, },
	{ 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0x00, },
	{ 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, },
	{ 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1f, 0x00, },
	{ 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x3f, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x01, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfc, 0x03, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf8, 0x03, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf0, 0x03, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x03, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
};

SR_PRIV void xmos_generate_analog_pattern(struct dev_context *devc)
{
	double t, frequency;
	float amplitude, offset;
	struct analog_pattern *pattern;
	unsigned int num_samples, i;
	float value;
	int last_end;

	num_samples = ANALOG_BUFSIZE / sizeof(float);
	frequency = (double) devc->cur_samplerate / ANALOG_SAMPLES_PER_PERIOD;
	amplitude = DEFAULT_ANALOG_AMPLITUDE;
	offset = DEFAULT_ANALOG_OFFSET;

	/*
	 * FIXME: We actually need only one period. A ringbuffer would be
	 * useful here.
	 * Make sure the number of samples we put out is an integer
	 * multiple of our period size.
	 */

	/* PATTERN_SQUARE: */
	sr_dbg("Generating %s pattern.", analog_pattern_str[PATTERN_SQUARE]);
	pattern = g_malloc(sizeof(struct analog_pattern));
	value = amplitude;
	last_end = 0;
	for (i = 0; i < num_samples; i++) {
		if (i % 5 == 0)
			value = -value;
		if (i % 10 == 0)
			last_end = i;
		pattern->data[i] = value + offset;
	}
	pattern->num_samples = last_end;
	devc->analog_patterns[PATTERN_SQUARE] = pattern;

	/* Readjusting num_samples for all other patterns. */
	while (num_samples % ANALOG_SAMPLES_PER_PERIOD != 0)
		num_samples--;

	/* PATTERN_SINE: */
	sr_dbg("Generating %s pattern.", analog_pattern_str[PATTERN_SINE]);
	pattern = g_malloc(sizeof(struct analog_pattern));
	for (i = 0; i < num_samples; i++) {
		t = (double) i / (double) devc->cur_samplerate;
		pattern->data[i] = sin(2 * G_PI * frequency * t) * amplitude + offset;
	}
	pattern->num_samples = last_end;
	devc->analog_patterns[PATTERN_SINE] = pattern;

	/* PATTERN_TRIANGLE: */
	sr_dbg("Generating %s pattern.", analog_pattern_str[PATTERN_TRIANGLE]);
	pattern = g_malloc(sizeof(struct analog_pattern));
	for (i = 0; i < num_samples; i++) {
		t = (double) i / (double) devc->cur_samplerate;
		pattern->data[i] = (2 / G_PI) * asin(sin(2 * G_PI * frequency * t)) *
			amplitude + offset;
	}
	pattern->num_samples = last_end;
	devc->analog_patterns[PATTERN_TRIANGLE] = pattern;

	/* PATTERN_SAWTOOTH: */
	sr_dbg("Generating %s pattern.", analog_pattern_str[PATTERN_SAWTOOTH]);
	pattern = g_malloc(sizeof(struct analog_pattern));
	for (i = 0; i < num_samples; i++) {
		t = (double) i / (double) devc->cur_samplerate;
		pattern->data[i] = 2 * ((t * frequency) - floor(0.5f + t * frequency)) *
			amplitude + offset;
	}
	pattern->num_samples = last_end;
	devc->analog_patterns[PATTERN_SAWTOOTH] = pattern;

	/* PATTERN_ANALOG_RANDOM */
	/* Data not filled here, will be generated in send_analog_packet(). */
	pattern = g_malloc(sizeof(struct analog_pattern));
	pattern->num_samples = last_end;
	devc->analog_patterns[PATTERN_ANALOG_RANDOM] = pattern;
}

SR_PRIV void xmos_free_analog_pattern(struct dev_context *devc)
{
	g_free(devc->analog_patterns[PATTERN_SQUARE]);
	g_free(devc->analog_patterns[PATTERN_SINE]);
	g_free(devc->analog_patterns[PATTERN_TRIANGLE]);
	g_free(devc->analog_patterns[PATTERN_SAWTOOTH]);
	g_free(devc->analog_patterns[PATTERN_ANALOG_RANDOM]);
}

static uint64_t encode_number_to_gray(uint64_t nr)
{
	return nr ^ (nr >> 1);
}

static void set_logic_data(uint64_t bits, uint8_t *data, size_t len)
{
	while (len--) {
		*data++ = bits & 0xff;
		bits >>= 8;
	}
}
#endif
#if 0
static void logic_generator(struct sr_dev_inst *sdi, uint64_t size)
{
	struct dev_context *devc;
	uint64_t i, j;
	uint8_t pat;
	uint8_t *sample;
	const uint8_t *image_col;
	size_t col_count, col_height;
	uint64_t gray;

	devc = sdi->priv;

	switch (devc->logic_pattern) {
	case PATTERN_SIGROK:
		memset(devc->logic_data, 0x00, size);
		for (i = 0; i < size; i += devc->logic_unitsize) {
			for (j = 0; j < devc->logic_unitsize; j++) {
				pat = pattern_sigrok[(devc->step + j) % sizeof(pattern_sigrok)] >> 1;
				devc->logic_data[i + j] = ~pat;
			}
			devc->step++;
		}
		break;
	case PATTERN_RANDOM:
		for (i = 0; i < size; i++)
			devc->logic_data[i] = (uint8_t)(rand() & 0xff);
		break;
	case PATTERN_INC:
		for (i = 0; i < size; i++) {
			for (j = 0; j < devc->logic_unitsize; j++)
				devc->logic_data[i + j] = devc->step;
			devc->step++;
		}
		break;
	case PATTERN_WALKING_ONE:
		/* j contains the value of the highest bit */
		j = 1 << (devc->num_logic_channels - 1);
		for (i = 0; i < size; i++) {
			devc->logic_data[i] = devc->step;
			if (devc->step == 0)
				devc->step = 1;
			else
				if (devc->step == j)
					devc->step = 0;
				else
					devc->step <<= 1;
		}
		break;
	case PATTERN_WALKING_ZERO:
		/* Same as walking one, only with inverted output */
		/* j contains the value of the highest bit */
		j = 1 << (devc->num_logic_channels - 1);
		for (i = 0; i < size; i++) {
			devc->logic_data[i] = ~devc->step;
			if (devc->step == 0)
				devc->step = 1;
			else
				if (devc->step == j)
					devc->step = 0;
				else
					devc->step <<= 1;
		}
		break;
	case PATTERN_ALL_LOW:
	case PATTERN_ALL_HIGH:
		/* These were set when the pattern mode was selected. */
		break;
	case PATTERN_SQUID:
		memset(devc->logic_data, 0x00, size);
		col_count = ARRAY_SIZE(pattern_squid);
		col_height = ARRAY_SIZE(pattern_squid[0]);
		for (i = 0; i < size; i += devc->logic_unitsize) {
			sample = &devc->logic_data[i];
			image_col = pattern_squid[devc->step];
			for (j = 0; j < devc->logic_unitsize; j++) {
				pat = image_col[j % col_height];
				sample[j] = pat;
			}
			devc->step++;
			devc->step %= col_count;
		}
		break;
	case PATTERN_GRAYCODE:
		for (i = 0; i < size; i += devc->logic_unitsize) {
			devc->step++;
			devc->step &= devc->all_logic_channels_mask;
			gray = encode_number_to_gray(devc->step);
			gray &= devc->all_logic_channels_mask;
			set_logic_data(gray, &devc->logic_data[i], devc->logic_unitsize);
		}
		break;
	default:
		sr_err("Unknown pattern: %d.", devc->logic_pattern);
		break;
	}
}
#endif

/*
 * Fixup a memory image of generated logic data before it gets sent to
 * the session's datafeed. Mask out content from disabled channels.
 *
 * TODO: Need we apply a channel map, and enforce a dense representation
 * of the enabled channels' data?
 */
#if 0
static void logic_fixup_feed(struct dev_context *devc,
		struct sr_datafeed_logic *logic)
{
	size_t fp_off;
	uint8_t fp_mask;
	size_t off, idx;
	uint8_t *sample;

	fp_off = devc->first_partial_logic_index;
	fp_mask = devc->first_partial_logic_mask;
	if (fp_off == logic->unitsize)
		return;

	for (off = 0; off < logic->length; off += logic->unitsize) {
		sample = logic->data + off;
		sample[fp_off] &= fp_mask;
		for (idx = fp_off + 1; idx < logic->unitsize; idx++)
			sample[idx] = 0x00;
	}
}
#endif

#if 0
static void send_analog_packet(struct analog_gen *ag,
		struct sr_dev_inst *sdi, uint64_t *analog_sent,
		uint64_t analog_pos, uint64_t analog_todo)
{
	struct sr_datafeed_packet packet;
	struct dev_context *devc;
	struct analog_pattern *pattern;
	uint64_t sending_now, to_avg;
	int ag_pattern_pos;
	unsigned int i;
	float amplitude, offset, value;
	float *data;

	if (!ag->ch || !ag->ch->enabled)
		return;

	devc = sdi->priv;
	packet.type = SR_DF_ANALOG;
	packet.payload = &ag->packet;

	pattern = devc->analog_patterns[ag->pattern];

	ag->packet.meaning->channels = g_slist_append(NULL, ag->ch);
	ag->packet.meaning->mq = ag->mq;
	ag->packet.meaning->mqflags = ag->mq_flags;

	/* Set a unit for the given quantity. */
	if (ag->mq == SR_MQ_VOLTAGE)
		ag->packet.meaning->unit = SR_UNIT_VOLT;
	else if (ag->mq == SR_MQ_CURRENT)
		ag->packet.meaning->unit = SR_UNIT_AMPERE;
	else if (ag->mq == SR_MQ_RESISTANCE)
		ag->packet.meaning->unit = SR_UNIT_OHM;
	else if (ag->mq == SR_MQ_CAPACITANCE)
		ag->packet.meaning->unit = SR_UNIT_FARAD;
	else if (ag->mq == SR_MQ_TEMPERATURE)
		ag->packet.meaning->unit = SR_UNIT_CELSIUS;
	else if (ag->mq == SR_MQ_FREQUENCY)
		ag->packet.meaning->unit = SR_UNIT_HERTZ;
	else if (ag->mq == SR_MQ_DUTY_CYCLE)
		ag->packet.meaning->unit = SR_UNIT_PERCENTAGE;
	else if (ag->mq == SR_MQ_CONTINUITY)
		ag->packet.meaning->unit = SR_UNIT_OHM;
	else if (ag->mq == SR_MQ_PULSE_WIDTH)
		ag->packet.meaning->unit = SR_UNIT_PERCENTAGE;
	else if (ag->mq == SR_MQ_CONDUCTANCE)
		ag->packet.meaning->unit = SR_UNIT_SIEMENS;
	else if (ag->mq == SR_MQ_POWER)
		ag->packet.meaning->unit = SR_UNIT_WATT;
	else if (ag->mq == SR_MQ_GAIN)
		ag->packet.meaning->unit = SR_UNIT_UNITLESS;
	else if (ag->mq == SR_MQ_SOUND_PRESSURE_LEVEL)
		ag->packet.meaning->unit = SR_UNIT_DECIBEL_SPL;
	else if (ag->mq == SR_MQ_CARBON_MONOXIDE)
		ag->packet.meaning->unit = SR_UNIT_CONCENTRATION;
	else if (ag->mq == SR_MQ_RELATIVE_HUMIDITY)
		ag->packet.meaning->unit = SR_UNIT_HUMIDITY_293K;
	else if (ag->mq == SR_MQ_TIME)
		ag->packet.meaning->unit = SR_UNIT_SECOND;
	else if (ag->mq == SR_MQ_WIND_SPEED)
		ag->packet.meaning->unit = SR_UNIT_METER_SECOND;
	else if (ag->mq == SR_MQ_PRESSURE)
		ag->packet.meaning->unit = SR_UNIT_HECTOPASCAL;
	else if (ag->mq == SR_MQ_PARALLEL_INDUCTANCE)
		ag->packet.meaning->unit = SR_UNIT_HENRY;
	else if (ag->mq == SR_MQ_PARALLEL_CAPACITANCE)
		ag->packet.meaning->unit = SR_UNIT_FARAD;
	else if (ag->mq == SR_MQ_PARALLEL_RESISTANCE)
		ag->packet.meaning->unit = SR_UNIT_OHM;
	else if (ag->mq == SR_MQ_SERIES_INDUCTANCE)
		ag->packet.meaning->unit = SR_UNIT_HENRY;
	else if (ag->mq == SR_MQ_SERIES_CAPACITANCE)
		ag->packet.meaning->unit = SR_UNIT_FARAD;
	else if (ag->mq == SR_MQ_SERIES_RESISTANCE)
		ag->packet.meaning->unit = SR_UNIT_OHM;
	else if (ag->mq == SR_MQ_DISSIPATION_FACTOR)
		ag->packet.meaning->unit = SR_UNIT_UNITLESS;
	else if (ag->mq == SR_MQ_QUALITY_FACTOR)
		ag->packet.meaning->unit = SR_UNIT_UNITLESS;
	else if (ag->mq == SR_MQ_PHASE_ANGLE)
		ag->packet.meaning->unit = SR_UNIT_DEGREE;
	else if (ag->mq == SR_MQ_DIFFERENCE)
		ag->packet.meaning->unit = SR_UNIT_UNITLESS;
	else if (ag->mq == SR_MQ_COUNT)
		ag->packet.meaning->unit = SR_UNIT_PIECE;
	else if (ag->mq == SR_MQ_POWER_FACTOR)
		ag->packet.meaning->unit = SR_UNIT_UNITLESS;
	else if (ag->mq == SR_MQ_APPARENT_POWER)
		ag->packet.meaning->unit = SR_UNIT_VOLT_AMPERE;
	else if (ag->mq == SR_MQ_MASS)
		ag->packet.meaning->unit = SR_UNIT_GRAM;
	else if (ag->mq == SR_MQ_HARMONIC_RATIO)
		ag->packet.meaning->unit = SR_UNIT_UNITLESS;
	else
		ag->packet.meaning->unit = SR_UNIT_UNITLESS;

	if (!devc->avg) {
		ag_pattern_pos = analog_pos % pattern->num_samples;
		sending_now = MIN(analog_todo, pattern->num_samples - ag_pattern_pos);
		if (ag->amplitude != DEFAULT_ANALOG_AMPLITUDE ||
			ag->offset != DEFAULT_ANALOG_OFFSET ||
			ag->pattern == PATTERN_ANALOG_RANDOM) {
			/*
			 * Amplitude or offset changed (or we are generating
			 * random data), modify each sample.
			 */
			if (ag->pattern == PATTERN_ANALOG_RANDOM) {
				amplitude = ag->amplitude / 500.0;
				offset = ag->offset - DEFAULT_ANALOG_OFFSET - ag->amplitude;
			} else {
				amplitude = ag->amplitude / DEFAULT_ANALOG_AMPLITUDE;
				offset = ag->offset - DEFAULT_ANALOG_OFFSET;
			}
			data = ag->packet.data;
			for (i = 0; i < sending_now; i++) {
				if (ag->pattern == PATTERN_ANALOG_RANDOM)
					data[i] = (rand() % 1000) * amplitude + offset;
				else
					data[i] = pattern->data[ag_pattern_pos + i] * amplitude + offset;
			}
		} else {
			/* Amplitude and offset unchanged, use the fast way. */
			ag->packet.data = pattern->data + ag_pattern_pos;
		}
		ag->packet.num_samples = sending_now;
		sr_session_send(sdi, &packet);

		/* Whichever channel group gets there first. */
		*analog_sent = MAX(*analog_sent, sending_now);
	} else {
		ag_pattern_pos = analog_pos % pattern->num_samples;
		to_avg = MIN(analog_todo, pattern->num_samples - ag_pattern_pos);
		if (ag->pattern == PATTERN_ANALOG_RANDOM) {
			amplitude = ag->amplitude / 500.0;
			offset = ag->offset - DEFAULT_ANALOG_OFFSET - ag->amplitude;
		} else {
			amplitude = ag->amplitude / DEFAULT_ANALOG_AMPLITUDE;
			offset = ag->offset - DEFAULT_ANALOG_OFFSET;
		}

		for (i = 0; i < to_avg; i++) {
			if (ag->pattern == PATTERN_ANALOG_RANDOM)
				value = (rand() % 1000) * amplitude + offset;
			else
				value = *(pattern->data + ag_pattern_pos + i) * amplitude + offset;
			ag->avg_val = (ag->avg_val + value) / 2;
			ag->num_avgs++;
			/* Time to send averaged data? */
			if ((devc->avg_samples > 0) && (ag->num_avgs >= devc->avg_samples))
				goto do_send;
		}

		if (devc->avg_samples == 0) {
			/*
			 * We're averaging all the samples, so wait with
			 * sending until the very end.
			 */
			*analog_sent = ag->num_avgs;
			return;
		}

do_send:
		ag->packet.data = &ag->avg_val;
		ag->packet.num_samples = 1;

		sr_session_send(sdi, &packet);
		*analog_sent = ag->num_avgs;

		ag->num_avgs = 0;
		ag->avg_val = 0.0f;
	}
}

#endif

#if 0
	static float    last_value = 0.0;
	static unsigned next_entry = 0;
	static uint64_t sample_timestamp = 0;
	static FILE*    data_log = NULL;

//
// xmos_send_analog_record()
//
// Sends the xmos_record_t, putting padding samples in as required
// (using the last sent value) to meet the timestamp of the record
//
static unsigned xmos_send_analog_record(xmos_record_t* rec, struct analog_gen *ag,
		struct sr_dev_inst *sdi, uint64_t *analog_sent,
		uint64_t analog_pos, uint64_t analog_todo)

// static void xmos_send_analogue(xmos_record_t* rec, struct dev_context *devc, uint64_t* samples_done)
{
//printf("xmos_send_analog_packet: ch %s, ts %llu, data %llu\n", rec->channel, rec->timestamp, rec->dataval);	

	struct sr_datafeed_packet packet;
	struct dev_context *devc;
//	struct analog_pattern *pattern;
//	uint64_t sending_now, to_avg;
//	int ag_pattern_pos;
//	unsigned int i;
//	float amplitude, offset, value;
	unsigned sending_now = 0;

	analog_pos++;
	analog_todo++;

	if (!ag->ch || !ag->ch->enabled)
		goto done;


//if (NULL == data_log) {
//	data_log = fopen("/tmp/datalog.txt", "wt");
//}

	devc = sdi->priv;
	packet.type = SR_DF_ANALOG;
	packet.payload = &ag->packet;

	const unsigned long sammple_period_ps = UINT64_C(1000000000000)/ devc->cur_samplerate;


//	pattern = devc->analog_patterns[ag->pattern];

//	ag->packet.meaning->channels = g_slist_append(NULL, ag->ch);
//	ag->packet.meaning->mq = ag->mq;
//	ag->packet.meaning->mqflags = ag->mq_flags;

	/* Set a unit for the given quantity. */
//	ag->packet.meaning->unit = SR_UNIT_VOLT;


// HACK - only channel 0 for now
//	if (0 != rec->channel) {
//		return;
//	}
	if (rec->channel != ag->ch->index)
		goto done;

	if (rec->timestamp < sample_timestamp) {
		// drop the packet - the sample rate is too low
		goto done;
	}

	if (0 == next_entry) {
		// start of new buffer
		last_value = (float)((signed long long)(rec->dataval)); // HACK - to revisit
		last_timestamp = rec->timestamp;

		data[next_entry] = last_value;
		next_entry++;
		goto done;
	}

	//  extrapolation of previous value
//	while (last_timestamp < rec->timestamp) {
	while (sample_timestamp < rec->timestamp) {	
		data[next_entry] = last_value;
	    next_entry++;
	    last_timestamp += sammple_period_ps;
        sample_timestamp += sammple_period_ps;

	    if (MAX_SAMPLES == next_entry) {
	    	// reached the buffer end - send the buffer

	    	ag->packet.data = data;
	    	ag->packet.num_samples = MAX_SAMPLES;

//printf("XMOS send num %d\n", ag->packet.num_samples);
			sr_session_send(sdi, &packet);

			sending_now += ag->packet.num_samples;

	    	next_entry = 0;

#if 0
fprintf(data_log, "PAD: last_timestamp %lld, rec->timestamp %lld, sammple_period_ps %lld, num_samples %d\n", last_timestamp, rec->timestamp, sammple_period_ps, MAX_SAMPLES);
for (int i=0; i<MAX_SAMPLES; i++) {
  fprintf(data_log, "data[%d] = %f\n", i, data[i]);
}
#endif
	    }
	}

	last_value = (float)((signed long long)(rec->dataval)); // HACK - to revisit
	last_timestamp = rec->timestamp;

	data[next_entry] = last_value;
	next_entry++;

    sample_timestamp += sammple_period_ps;

    if (MAX_SAMPLES == next_entry) {
    	// reached the buffer end - send the buffer

    	ag->packet.data = data;
    	ag->packet.num_samples = MAX_SAMPLES;


//printf("XMOS send num %d\n", ag->packet.num_samples);
		sr_session_send(sdi, &packet);
		sending_now += ag->packet.num_samples;
    	next_entry = 0;

#if 0
fprintf(data_log, "REA: last_timestamp %lld, rec->timestamp %lld, sammple_period_ps %lld, num_samples %d\n", last_timestamp, rec->timestamp, sammple_period_ps, MAX_SAMPLES);
for (int i=0; i<MAX_SAMPLES; i++) {
  fprintf(data_log, "data[%d] = %f\n", i, data[i]);
}
#endif

    }
done:
	return sending_now;
}

#endif

static xmos_iter_t xmos_record_iter;

#if 0
static void xmos_prepare_send_analog_packets(struct sr_dev_inst *sdi) {

	struct dev_context *devc = sdi->priv;
	devc->xmos_tcp_ops->xmos_record_iter_init(&xmos_record_iter);
}

static void xmos_done_send_analog_packets(struct sr_dev_inst *sdi) {

	struct dev_context *devc = sdi->priv;
	devc->xmos_tcp_ops->xmos_record_iter_done(&xmos_record_iter);
}

static void xmos_send_analog_packets(struct analog_gen *ag,
		struct sr_dev_inst *sdi, uint64_t *analog_sent,
		uint64_t analog_pos, uint64_t analog_todo)
{
	xmos_record_t  *record;
	struct dev_context *devc = sdi->priv;
	unsigned sent = 0;




//	devc->xmos_tcp_ops->xmos_record_iter_init(&iter);
//	devc->xmos_tcp_ops->xmos_record_iter_reset(&iter);
#if 0	
	while (devc->xmos_tcp_ops->xmos_record_iter_next(&iter, &record) && 
			(sent < analog_todo) ) {
		sent += xmos_send_analog_record(record, ag, sdi, analog_sent, analog_pos, analog_todo);
	}
#else
	if (devc->xmos_tcp_ops->xmos_record_iter_next(&xmos_record_iter, &record)) {
		sent += xmos_send_analog_record(record, ag, sdi, analog_sent, analog_pos, analog_todo);
	}
#endif	
	*analog_sent = MAX(*analog_sent, sent);

//	devc->xmos_tcp_ops->xmos_record_iter_done(&iter);
}

#endif


#if 0
	struct sr_datafeed_packet packet;
	struct dev_context *devc;
//	struct analog_pattern *pattern;
//	uint64_t sending_now, to_avg;
//	int ag_pattern_pos;
//	unsigned int i;
//	float amplitude, offset, value;
	unsigned sending_now = 0;

	analog_pos++;
	analog_todo++;

	if (!ag->ch || !ag->ch->enabled)
		goto done;


//if (NULL == data_log) {
//	data_log = fopen("/tmp/datalog.txt", "wt");
//}

#endif

//  #define  FLOG
// TODO
uint64_t xmos_get_acquisition_start_timestamp();
static uint64_t latest_timestamp = 0;
static uint64_t last_timestamp = 0xffffffffffffffff;
#define MAX_CHANNELS  	256
#define MAX_SAMPLES   16384
static float    data[MAX_CHANNELS][MAX_SAMPLES];
//static unsigned long samples_just_done = 0;

static void xmos_send_analog_channel(struct analog_gen *ag,
		struct sr_dev_inst *sdi) {

// TODO
#ifdef FLOG
static FILE* flog;
if (!flog) {
flog = fopen("/tmp/log.txt", "wt");
}
#endif

	static float last_sample_value[MAX_CHANNELS];

	struct sr_datafeed_packet packet;
	struct dev_context *devc;

	xmos_record_t* record;

	unsigned long sample_timestamp;
	unsigned long samples_done = 0;

	if (!ag->ch || !ag->ch->enabled)
		goto done;

	devc = sdi->priv;

	const unsigned long sammple_period_ps = UINT64_C(1000000000000)/ devc->cur_samplerate;

	packet.type = SR_DF_ANALOG;
	packet.payload = &ag->packet;

	unsigned long samples_todo = MIN(MAX_SAMPLES, ag->samples_todo);

	unsigned channel;
	channel = ag->ch->index;

	if (0xffffffffffffffff == last_timestamp) {
		last_timestamp = xmos_get_acquisition_start_timestamp();
//printf("channel %u, xmos_get_acquisition_start_timestamp %lu\n", channel, last_timestamp);
	}

	sample_timestamp = last_timestamp;
#ifdef FLOG
{
fprintf(flog, "channel %u, samples_todo %lu\n", channel, samples_todo);
}
unsigned datarecs = 0;
#endif

	devc->xmos_tcp_ops->xmos_record_iter_init(&xmos_record_iter, channel);

	while  (devc->xmos_tcp_ops->xmos_record_iter_next(&xmos_record_iter, channel, &record)) {

#ifdef FLOG
{
fprintf(flog, "channel %u, sammple_period_ps %lu, sample_timestamp %lu, record->timestamp %lu, record->dataval %f\n", 
	channel, sammple_period_ps, sample_timestamp, record->timestamp, (float)((signed long long)(record->dataval)) );
}
unsigned padding = 0;
#endif


#if 0 
		if (record->timestamp > sample_timestamp + sammple_period_ps) {
			// Missed the record ignore ?
		}
#endif
//		while (sample_timestamp < (record->timestamp - sammple_period_ps) &&
		while (sample_timestamp < record->timestamp &&
			   samples_done < samples_todo) {

			data[channel][samples_done] = last_sample_value[channel];
//data[channel][samples_done] = -1;

			sample_timestamp += sammple_period_ps;
			samples_done++;
#ifdef FLOG
padding++;
#endif
			if (samples_todo == samples_done) {
				// reached the buffer end - send the buffer

		    	ag->packet.data = data[channel];
		    	ag->packet.num_samples = samples_done;

				sr_session_send(sdi, &packet);

#ifdef FLOG
{
//for (unsigned i=0; i<samples_done; i++) {

//fwrite(data[channel], sizeof(float), samples_done, flog);
fprintf(flog, "pad %ld, datarecs %u, sample_timestamp %lu\n", samples_done, datarecs, sample_timestamp);
}
#endif

				// Put the record back for next time
				devc->xmos_tcp_ops->xmos_record_iter_undonext(&xmos_record_iter);
				goto done;
			}
		}

#ifdef FLOG
{
fprintf(flog, "EMIT: channel %u, padding %u, sample_timestamp %lu, record->timestamp %lu, record->dataval %f\n", 
	channel, padding, sample_timestamp, record->timestamp, (float)((signed long long)(record->dataval)) );
}
#endif
		// emit record
		data[channel][samples_done] = (float)((signed long long)(record->dataval));
//data[channel][samples_done] = 1;
		last_sample_value[channel] = data[channel][samples_done];

		sample_timestamp += sammple_period_ps;
		samples_done++;
#ifdef FLOG
datarecs++;
#endif
// DO NOT 
//		sample_timestamp = record->timestamp;

//printf("channel %u, buffer_pos %lu, value %f\n", channel, samples_done, (float)((signed long long)(record->dataval)) );

		if (samples_todo == samples_done) {
			// reached the buffer end - send the buffer

	    	ag->packet.data = data[channel];
	    	ag->packet.num_samples = samples_done;

			sr_session_send(sdi, &packet);
#ifdef FLOG
{
//for (unsigned i=0; i<samples_done; i++) {
//fwrite(data[channel], sizeof(float), samples_done, flog);
fprintf(flog, "data %ld, datarecs %u, val %lu\n", samples_done, datarecs, record->dataval);
}
#endif
			goto done;
		}
	}

    // There may be samples remaning (no packets from target available)
	while (samples_done < samples_todo) {
		data[channel][samples_done] = last_sample_value[channel];
		sample_timestamp += sammple_period_ps;
		samples_done++;
	}
	ag->packet.data = data[channel];
   	ag->packet.num_samples = samples_done;
	sr_session_send(sdi, &packet);


done:

//printf("channel %u, last_timestamp %lu, samples_done %lu\n", channel, last_timestamp, samples_done);
	devc->xmos_tcp_ops->xmos_record_iter_done(&xmos_record_iter, channel);

	ag->samples_todo -= samples_done;
//	samples_just_done = MAX(samples_just_done, samples_done);
	latest_timestamp = MAX(latest_timestamp, sample_timestamp);
}

/* Callback handling data */
SR_PRIV int xmos_prepare_data(int fd, int revents, void *cb_data)
{
//printf(">>>> xmos_prepare_data()\n");
	struct sr_dev_inst *sdi;
	struct dev_context *devc;
	struct sr_datafeed_packet packet;
//	struct sr_datafeed_logic logic;
	struct analog_gen *ag;
	GHashTableIter iter;
	void *value;
//	uint64_t samples_todo, logic_done, analog_done, analog_sent; //, sending_now;
	uint64_t samples_todo, analog_done; //, sending_now;	
	int64_t elapsed_us, limit_us, todo_us;
//	int64_t trigger_offset;
//	int pre_trigger_samples;

	(void)fd;
	(void)revents;

	sdi = cb_data;
	devc = sdi->priv;

	/* Just in case. */
	if (devc->cur_samplerate <= 0
			|| (devc->num_logic_channels <= 0
			&& devc->num_analog_channels <= 0)) {
		sr_dev_acquisition_stop(sdi);
printf("<<<< xmos_prepare_data(): devc->cur_samplerate %lu\n", devc->cur_samplerate);
		return G_SOURCE_CONTINUE;
	}

	/* What time span should we send samples for? */
	elapsed_us = g_get_monotonic_time() - devc->start_us;
	limit_us = 1000 * devc->limit_msec;
	if (limit_us > 0 && limit_us < elapsed_us)
		todo_us = MAX(0, limit_us - devc->spent_us);
	else
		todo_us = MAX(0, elapsed_us - devc->spent_us);

	/* How many samples are outstanding since the last round? */
	samples_todo = (todo_us * devc->cur_samplerate + G_USEC_PER_SEC - 1)
			/ G_USEC_PER_SEC;
//	samples_todo = (todo_us * devc->cur_samplerate) / G_USEC_PER_SEC;

	if (devc->limit_samples > 0) {
		if (devc->limit_samples < devc->sent_samples)
			samples_todo = 0;
		else if (devc->limit_samples - devc->sent_samples < samples_todo)
			samples_todo = devc->limit_samples - devc->sent_samples;
	}

	if (samples_todo == 0) {
printf("<<<< xmos_prepare_data(): samples_todo %lu\n", samples_todo);
		return G_SOURCE_CONTINUE;		
	}

	if (devc->limit_frames) {
		/* Never send more samples than a frame can fit... */
		samples_todo = MIN(samples_todo, SAMPLES_PER_FRAME);
		/* ...or than we need to finish the current frame. */
		samples_todo = MIN(samples_todo,
			SAMPLES_PER_FRAME - devc->sent_frame_samples);
	}

	/* Calculate the actual time covered by this run back from the sample
	 * count, rounded towards zero. This avoids getting stuck on a too-low
	 * time delta with no samples being sent due to round-off.
	 */
	todo_us = samples_todo * G_USEC_PER_SEC / devc->cur_samplerate;

// printf("samples_todo: %lu, todo_us: %ld\n", samples_todo, todo_us);

	unsigned channels_to_complete = 0;
	g_hash_table_iter_init(&iter, devc->ch_ag);
	while (g_hash_table_iter_next(&iter, NULL, &value)) {
		ag = value;
		ag->samples_todo = samples_todo;
		channels_to_complete++;
	}

	while (channels_to_complete > 0) {
		//samples_just_done = 0;

		g_hash_table_iter_init(&iter, devc->ch_ag);
		while (g_hash_table_iter_next(&iter, NULL, &value)) {
			ag = value;

			if (ag->samples_todo>0) {
				xmos_send_analog_channel(ag, sdi);
			}	

			if (0 == ag->samples_todo) {
				channels_to_complete--;
			}
		}
		// assume all channels sent the same number of samples - use count from the last channel
		
		last_timestamp = latest_timestamp;


//		if (channels_to_complete>0) {
//			// wait for more data to arrive from target, then go back and try to send any present
//			g_usleep(100);
//		}
	}
	analog_done = samples_todo;


#if 0

	logic_done = devc->num_logic_channels > 0 ? 0 : samples_todo;
	if (!devc->enabled_logic_channels)
		logic_done = samples_todo;

	analog_done = devc->num_analog_channels > 0 ? 0 : samples_todo;
	if (!devc->enabled_analog_channels)
		analog_done = samples_todo;

	while (logic_done < samples_todo || analog_done < samples_todo) {
		/* Logic */
		if (logic_done < samples_todo) {
			sending_now = MIN(samples_todo - logic_done,
					LOGIC_BUFSIZE / devc->logic_unitsize);
			logic_generator(sdi, sending_now * devc->logic_unitsize);
			/* Check for trigger and send pre-trigger data if needed */
			if (devc->stl && (!devc->trigger_fired)) {
				trigger_offset = soft_trigger_logic_check(devc->stl,
						devc->logic_data, sending_now * devc->logic_unitsize,
						&pre_trigger_samples);
				if (trigger_offset > -1) {
					devc->trigger_fired = TRUE;
					logic_done = pre_trigger_samples;
				}
			} else
				trigger_offset = 0;

			/* Send logic samples if needed */
			packet.type = SR_DF_LOGIC;
			packet.payload = &logic;
			logic.unitsize = devc->logic_unitsize;

			if (devc->stl) {
				if (devc->trigger_fired && (trigger_offset < (int)sending_now)) {
					/* Send after-trigger data */
					logic.length = (sending_now - trigger_offset) * devc->logic_unitsize;
					logic.data = devc->logic_data + trigger_offset * devc->logic_unitsize;
					logic_fixup_feed(devc, &logic);
					sr_session_send(sdi, &packet);
					logic_done += sending_now - trigger_offset;
					/* End acquisition */
					sr_dbg("Triggered, stopping acquisition.");
					sr_dev_acquisition_stop(sdi);
					break;
				} else {
					/* Send nothing */
					logic_done += sending_now;
				}
			} else if (!devc->stl) {
				/* No trigger defined, send logic samples */
				logic.length = sending_now * devc->logic_unitsize;
				logic.data = devc->logic_data;
				logic_fixup_feed(devc, &logic);
				sr_session_send(sdi, &packet);
				logic_done += sending_now;
			}
		}

		/* Analog, one channel at a time */
		if (analog_done < samples_todo) {
			analog_sent = 0;

			g_hash_table_iter_init(&iter, devc->ch_ag);
			while (g_hash_table_iter_next(&iter, NULL, &value)) {
				send_analog_packet(value, sdi, &analog_sent,
						devc->sent_samples + analog_done,
						samples_todo - analog_done);
			}
			analog_done += analog_sent;
		}
	}
#endif

//	uint64_t min = MIN(logic_done, analog_done);

	uint64_t min = analog_done;
	devc->sent_samples += min;
	devc->sent_frame_samples += min;
	devc->spent_us += todo_us;

	if (devc->limit_frames && devc->sent_frame_samples >= SAMPLES_PER_FRAME) {
#if 0
		std_session_send_frame_end(sdi);
#endif		
		devc->sent_frame_samples = 0;
		devc->limit_frames--;
		if (!devc->limit_frames) {
			sr_dbg("Requested number of frames reached.");
			sr_dev_acquisition_stop(sdi);
		}
	}

	if ((devc->limit_samples > 0 && devc->sent_samples >= devc->limit_samples)
			|| (limit_us > 0 && devc->spent_us >= limit_us)) {

		/* If we're averaging everything - now is the time to send data */
		if (devc->avg && devc->avg_samples == 0) {
			g_hash_table_iter_init(&iter, devc->ch_ag);
			while (g_hash_table_iter_next(&iter, NULL, &value)) {
				ag = value;
				packet.type = SR_DF_ANALOG;
				packet.payload = &ag->packet;
				ag->packet.data = &ag->avg_val;
				ag->packet.num_samples = 1;
				sr_session_send(sdi, &packet);
			}
		}
		sr_dbg("Requested number of samples reached.");
		sr_dev_acquisition_stop(sdi);
	} else if (devc->limit_frames) {
#if 0
		if (devc->sent_frame_samples == 0)
			std_session_send_frame_begin(sdi);
#endif
	}

//printf("analog_done: %lu\n", analog_done);


	return G_SOURCE_CONTINUE;
}

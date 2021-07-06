/*
 * This file is part of the libsigrok project.
 *
 * Copyright (C) 2017 Kumar Abhishek <abhishek@theembeddedkitchen.net>
 * Portions of the code are adapted from scpi_tcp.c and scpi.c, their
 * copyright notices are listed below:
 *
 * Copyright (C) 2013 Martin Ling <martin-sigrok@earth.li>
 * Copyright (C) 2013 poljar (Damir Jelić) <poljarinho@gmail.com>
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

#include <assert.h>

#include <config.h>
#ifdef _WIN32
#define _WIN32_WINNT 0x0501
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <glib.h>
#include <string.h>
#include <unistd.h>
#ifndef _WIN32
#define __USE_XOPEN2K
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <errno.h>
#include <libsigrok/libsigrok.h>
#include "libsigrok-internal.h"



#include "protocol.h"
#include "xmos.h"


#ifdef _WIN32
	#define MUTEX_TYPE(m) HANDLE (m)
	#define MUTEX_INIT(m) (m) = CreateMutex(NULL, FALSE, NULL)
	#define MUTEX_LOCK(m) WaitForSingleObject(m, INFINITE)
	#define MUTEX_UNLOCK(m) ReleaseMutex(m)

  #include <windows.h>
  #define THREAD_HND(h) HANDLE (h)
  #define THREAD_FUNC_DEC(f, arg) long WINAPI f (_In_ LPVOID arg)
  #define CREATE_THREAD(func, hdl, arg) hdl = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(func), (__drv_aliasesMem LPVOID)arg, 0, 0)
  #define JOIN_THREAD(func, hdl) WaitForSingleObject(hdl, INFINITE)

#else
	#define MUTEX_TYPE(m) pthread_mutex_t (m) = PTHREAD_MUTEX_INITIALIZER
	#define MUTEX_INIT(m)
	#define MUTEX_LOCK(m) pthread_mutex_lock(&(m))
	#define MUTEX_UNLOCK(m) pthread_mutex_unlock(&(m))

  #include <pthread.h>
  #define THREAD_HND(h) pthread_t (h)
  #define THREAD_FUNC_DEC(f, arg) void *f (void *arg)
  #define CREATE_THREAD(func, hdl, arg) pthread_create(&hdl, NULL, func, arg)
  #define JOIN_THREAD(func, hdl) pthread_join(hdl, NULL)
#endif

// Internal library functions
//static int xscope_ep_check_for_data();
static int xscope_ep_process(struct dev_context *devc);

// Internal thread state
static THREAD_HND(xscope_ep_data_thread_handle);
static int xscope_ep_data_thread_run = 0;

// xSCOPE message types
#define XSCOPE_SOCKET_MSG_EVENT_REGISTER 1
#define XSCOPE_SOCKET_MSG_EVENT_RECORD 2

//
// ******* FIFO buffering
//

typedef enum {
    XSCOPE_NONE=0,
    XSCOPE_UINT,
    XSCOPE_INT,
    XSCOPE_FLOAT,
} xscope_datatype_t;

#define MAX_CHANNELS 256
#define NUM_BUF_ENTRIES (1024 * 1024)

static xscope_datatype_t channel_type[MAX_CHANNELS];

static xmos_record_t* buffer_pool[MAX_CHANNELS];

static xmos_record_t* write_ptr[MAX_CHANNELS];
static xmos_record_t* read_ptr[MAX_CHANNELS];

static MUTEX_TYPE(buffer_mutex);

static int xmos_recv(struct dev_context *devc, void* buffer, size_t length)
{
    char* ptr = buffer;
    int todo = length;

	while (todo > 0) {
    	int len = recv(devc->socket, ptr, todo, 0);

	    if (len < 0) {
			#if defined(__LINUX__) || defined(__linux__) || defined (__CYGWIN__) || defined (__APPLE__)
				if (EINTR == errno) {
					continue;
				}
			#endif

		    sr_err("Receive error: %s", g_strerror(errno));
		    return SR_ERR;
	    }
	    todo -= len;
	    ptr  += len;

	}
	return length;
}

static int xmos_send(struct dev_context *devc, const void* buffer, size_t length)
{
    const char* ptr = buffer;
    int todo = length;

	while (todo > 0) {
	    int len = send(devc->socket, buffer, todo, 0);

   	    if (len < 0) {
			#if defined(__LINUX__) || defined(__linux__) || defined (__CYGWIN__) || defined (__APPLE__)
				if (EINTR == errno) {
					continue;
				}
			#endif
		    sr_err("Receive error: %s", g_strerror(errno));
		    return SR_ERR;
	    }
	    todo -= len;
	    ptr  += len;
	}
	return length;
}


static uint64_t starting_timestamp = UINT64_MAX;

// TODO - remove below

static int start_collecting = 0;
// TODO 
void add_analogue_channel(unsigned ch_id, const char* name);
void done_adding_analogue_channel(void);
void collect_data(void);
void collect_data(void) {
	start_collecting = 1;
}

static void xscope_register(struct dev_context *devc,
							unsigned int id, unsigned int type,
							unsigned int r, unsigned int g, unsigned int b,
							char *name, char *unit,
							unsigned int data_type, char* data_name) {

	printf("xSCOPE register event (id [%d] type [%d], name [%s], unit [%s], data_type[%d], data_name[%s])\n", id, type, name, unit, data_type, data_name);
	// TODO - rmove params
	devc++;
    r++;
    g++;
    b++;
    data_name++;
    
	assert(id < MAX_CHANNELS);

    channel_type[id] = data_type;

   	add_analogue_channel(id, name);
}

int unsigned long record_count_in = 0;
int unsigned long record_count_out = 0;

static void xscope_record(unsigned int id, unsigned long long timestamp, unsigned int length, 
                   unsigned long long dataval, unsigned char *databytes) {

	static int records_available = 0;
	if (0 == records_available) {
		// Assume all registration is complete first time this function is called
		// - is thie safe to assume with multiple tiles?
		done_adding_analogue_channel();
		records_available++;

		while (0 == start_collecting) {
			// Wait until we are told to start
			g_usleep(100);
		}
printf("xscope_record() : unblocked and collecting records\n");		
	}

	databytes++;

    if (length != 0) {
        // ignore - probably emitted by xscope_bytes()
        return;
    }

    MUTEX_LOCK(buffer_mutex);

record_count_in++;

	int64_t t = g_get_monotonic_time();

    if (UINT64_MAX == starting_timestamp) {
    	// Record the stamp of the first packet received
    	starting_timestamp = timestamp;
printf("xscope_record() : update starting_timestamp = %lu\n", starting_timestamp);
    }

    write_ptr[id]->timestamp = timestamp;
    write_ptr[id]->dataval = dataval;

    if (write_ptr[id] == &buffer_pool[id][NUM_BUF_ENTRIES-1] ) {
        // wrap back to start
        write_ptr[id] = buffer_pool[id];
    } else {
        write_ptr[id]++;
    }

    // If the buffer is full, move the read_ptr on
    if (write_ptr[id] == read_ptr[id]) {
        if (read_ptr[id] == &buffer_pool[id][NUM_BUF_ENTRIES-1] ) {
            // wrap back to start
            read_ptr[id] = buffer_pool[id];
        } else {
            read_ptr[id]++;
        }
    }
    MUTEX_UNLOCK(buffer_mutex);
}


void xmos_acquisition_start(void);
uint64_t xmos_get_acquisition_start_timestamp(void);

void xmos_acquisition_start(void) {
	unsigned i;

printf(">>>> xmos_acquisition_start\n");

	// ignore any data captured before the acquisition was started
	MUTEX_LOCK(buffer_mutex);
record_count_in = 0;	
	starting_timestamp = UINT64_MAX;

printf("xscope_record() : reset starting_timestamp = %lu\n", starting_timestamp);

	for (i=0; i<MAX_CHANNELS; i++) {
		read_ptr[i] = write_ptr[i] = buffer_pool[i];
	}
	MUTEX_UNLOCK(buffer_mutex);

printf("<<<< xmos_acquisition_start\n");
}

uint64_t xmos_get_acquisition_start_timestamp() {
	return starting_timestamp;
}

#if 0
static void xmos_record_iter_reset(xmos_iter_t* iter, unsigned channel) {
	iter->data = read_ptr[channel];
}
#endif

static void xmos_record_iter_init(xmos_iter_t* iter, unsigned channel) {

	MUTEX_LOCK(buffer_mutex);

	iter->data = read_ptr[channel];
}

static void xmos_record_iter_done(xmos_iter_t* iter, unsigned channel) {

	read_ptr[channel] = iter->data;

	MUTEX_UNLOCK(buffer_mutex);
}

static int xmos_record_iter_next(xmos_iter_t* iter, unsigned channel, xmos_record_t** record) {

    xmos_record_t* next_ptr = (xmos_record_t*)iter->data;
    iter->last_data = next_ptr;

	if (next_ptr != write_ptr[channel]) {
		*record = next_ptr;
		if (next_ptr == &buffer_pool[channel][NUM_BUF_ENTRIES-1] ) {
			// wrap back to start
			next_ptr = buffer_pool[channel];
		} else {
			next_ptr++;
		}
		iter->data = next_ptr;
record_count_out++;
		return 1;
	} else {
		return 0;
	}

}

static void xmos_record_iter_undonext(xmos_iter_t* iter) {
record_count_out--;
	iter->data = iter->last_data;
}


static THREAD_FUNC_DEC(xscope_ep_data_thread, arg) {
	struct dev_context *devc = (struct dev_context *)arg;

	while (xscope_ep_data_thread_run) {
    	if (xscope_ep_process(devc) == -1) 
    		break;
	}
	return 0;
}

static int xscope_ep_process(struct dev_context *devc) {

	uint8_t header = 0;

	if (xmos_recv(devc, (char *)&header, 1) < 1) {
		sr_err("XMOS: failed to read header\n");
		return SR_ERR;
	}

	switch (header) {
	default:
	  	sr_err("XMOS: unexpected header %d\n", (int)header);
    	return SR_ERR;

	case XSCOPE_SOCKET_MSG_EVENT_REGISTER: {
		typedef struct {
			uint32_t id;
			uint32_t type;
			uint32_t r;
			uint32_t g;
			uint32_t b;
			uint32_t namelen;
		} xscope_register_p1;
		typedef struct {
			uint32_t datatype;
			uint32_t datanamelen;
		} xscope_register_p2;

		xscope_register_p1 p1;
		xscope_register_p2 p2;
		char* name;
		uint32_t unitlen;
		char* unit;
		char* dataname;

		if (xmos_recv(devc, (char*)&p1, sizeof(p1))<(int)sizeof(p1)) {
			sr_err("XMOS: Failed read p1");
			return SR_ERR;
		}		
		name = g_malloc0(p1.namelen);

		if (xmos_recv(devc, name, p1.namelen)<(int)p1.namelen) {
			sr_err("XMOS: Failed read name");
			return SR_ERR;
		}
		if (xmos_recv(devc, (char *)&unitlen, sizeof(unitlen))<(int)sizeof(unitlen)) {
			sr_err("XMOS: Failed read unitlen");
			return SR_ERR;
		}

		unit = g_malloc0(unitlen);
		if (xmos_recv(devc, (char *)unit, unitlen)<(int)unitlen) {
			sr_err("XMOS: Failed read unit");
			return SR_ERR;
		}

		if (xmos_recv(devc, (char*)&p2, sizeof(p2))<(int)sizeof(p2)) {
			sr_err("XMOS: Failed read p2");
			return SR_ERR;
		}		
		dataname = g_malloc0(p2.datanamelen);
		if (xmos_recv(devc, dataname, p2.datanamelen)<(int)p2.datanamelen) {
			sr_err("XMOS: Failed read dataname");
			return SR_ERR;
		}		
		xscope_register(devc, p1.id, p1.type, p1.r, p1.g, p1.b, name, unit, 
							p2.datatype, dataname);

		g_free(name);
		g_free(unit);
		g_free(dataname);
		break;
	}
	case XSCOPE_SOCKET_MSG_EVENT_RECORD: {
		uint8_t  id;
		uint8_t  flagA;
		uint8_t  flagB;
		uint32_t length;

		uint64_t dataval = 0;
		uint8_t *databytes = NULL;
		uint64_t timestamp = 0;

		if (xmos_recv(devc, (char *)&id, sizeof(id))<(int)sizeof(id)) {
			sr_err("XMOS: Failed read id\n");
			return SR_ERR;
		}
		if (xmos_recv(devc, (char *)&flagA, sizeof(flagA))<(int)sizeof(flagA)) {
			sr_err("XMOS: Failed read flagA\n");
			return SR_ERR;
		}
		if (xmos_recv(devc, (char *)&flagB, sizeof(flagB))<(int)sizeof(flagB)) {
			sr_err("XMOS: Failed read flagB\n");
			return SR_ERR;
		}

		if (xmos_recv(devc, (char *)&length, sizeof(length))<(int)sizeof(length)) {
			sr_err("XMOS: Failed read flagB\n");
			return SR_ERR;
		}
		if (length > 0) {
			databytes = g_malloc0(length);
			if (xmos_recv(devc, databytes, length)<(int)length) {
				sr_err("XMOS: Failed read databytes\n");
				return SR_ERR;
			}
		} else {
			if (xmos_recv(devc, (char *)&dataval, sizeof(dataval))<(int)sizeof(dataval)) {
				sr_err("XMOS: Failed read dataval\n");
				return SR_ERR;
			}
		}
		if (xmos_recv(devc, (char *)&timestamp, sizeof(timestamp))<(int)sizeof(timestamp)) {
			sr_err("XMOS: Failed read timestamp\n");
			return SR_ERR;
		}

		xscope_record(id, timestamp, length, dataval, databytes);

        g_free(databytes);
        break;
      }
  }

  return 0;
}

static int xmos_open(struct dev_context *devc)
{
printf(">>>> xmos_open\n");


	struct addrinfo hints;
	struct addrinfo *results, *res;
	int err, i;

	for (i=0; i<MAX_CHANNELS; i++) {
		buffer_pool[i] = g_malloc(sizeof(xmos_record_t) * NUM_BUF_ENTRIES);

		read_ptr[i]  = buffer_pool[i];
		write_ptr[i] = buffer_pool[i];
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	err = getaddrinfo(devc->address, devc->port, &hints, &results);

	if (err < 0) {
		sr_err("Address lookup failed: %s:%s: %s", devc->address,
			devc->port, gai_strerror(err));
		return SR_ERR;
	}

	for (res = results; res; res = res->ai_next) {
		if ((devc->socket = socket(res->ai_family, res->ai_socktype,
						res->ai_protocol)) < 0)
			continue;
		if (connect(devc->socket, res->ai_addr, res->ai_addrlen) != 0) {
			close(devc->socket);
			devc->socket = -1;
			continue;
		}

		#if defined(__LINUX__) || defined(__linux__)
			// Set TCP_NODELAY to disable the NAGLE algorithm
			int flag = 1;
			err = setsockopt(devc->socket,
								IPPROTO_TCP, TCP_NODELAY, 
								(char *)&flag, sizeof(flag));

			if (err < 0) {
				sr_err("Couldn't setsockopt(TCP_NODELAY) errno=%d\n", errno);
				close(devc->socket);
				devc->socket = -1;
				continue;
			}
		#endif

		// request is the "flags" byte:
		// #define XSCOPE_SERVER_REGISTRATION 1
		// #define XSCOPE_SERVER_RECORDS 2
		// #define XSCOPE_SERVER_STATS 4
		// #define XSCOPE_SERVER_PRINT 8

		// We need XSCOPE_SERVER_RECORDS | XSCOPE_SERVER_REGISTRATION
		char request = 0x3;

		int bytesSent = xmos_send(devc, &request, 1);
		if (bytesSent == 1) {
			// Start xscope data thread
			xscope_ep_data_thread_run = 1;
			CREATE_THREAD(xscope_ep_data_thread, 
							xscope_ep_data_thread_handle, devc);

		} else {
			sr_err("Failed: xscope_send()\n");
		}
  	}

	freeaddrinfo(results);

	if (devc->socket < 0) {
		sr_err("Failed to connect to %s:%s: %s", devc->address,
			devc->port, g_strerror(errno));
		return SR_ERR;
	}

    MUTEX_INIT(buffer_mutex);

printf("<<<< xmos_open\n");

	return SR_OK;
}


static int xmos_close(struct dev_context *devc)
{
printf(">>>> xmos_close\n");
	xscope_ep_data_thread_run = 0;
    JOIN_THREAD(xscope_ep_data_thread, xscope_ep_data_thread_handle);

	if (close(devc->socket) < 0)
		return SR_ERR;

	devc->socket = -1;
printf("<<<< xmos_close\n");
	return SR_OK;
}

static int xmos_detect(struct dev_context *devc)
{
printf(">>>> xmos_detect()\n");
#if 0
// TODO
	char *resp = NULL;
	int ret;

	ret = xmos_tcp_get_string(devc, "version", &resp);
	if (ret == SR_OK && !g_ascii_strncasecmp(resp, "xmos", 11))
		ret = SR_OK;
	else
		ret = SR_ERR;

	g_free(resp);

	return ret;
#else
//	unsigned char cmd = XMOS_DISCONNECT;
//	xmos_send(devc, &cmd, 1);
	devc++;
	return SR_OK;
#endif
}

SR_PRIV const struct xmos_ops xmos_tcp_ops = {
	.open = xmos_open,
	.close = xmos_close,
	.detect = xmos_detect,

	.xmos_record_iter_init = xmos_record_iter_init,
	.xmos_record_iter_next = xmos_record_iter_next,
	.xmos_record_iter_undonext = xmos_record_iter_undonext,
	.xmos_record_iter_done = xmos_record_iter_done,
};

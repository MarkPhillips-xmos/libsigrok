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
#if 0
#include <assert.h>
#include <config.h>
#include <errno.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x0501
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <glib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#ifndef _WIN32

#include <netdb.h>

static int xmos_open2(struct dev_context *devc)
{
	struct addrinfo hints;
}

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif

#endif


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
#else
	#define MUTEX_TYPE(m) pthread_mutex_t (m) = PTHREAD_MUTEX_INITIALIZER
	#define MUTEX_INIT(m)
	#define MUTEX_LOCK(m) pthread_mutex_lock(&(m))
	#define MUTEX_UNLOCK(m) pthread_mutex_unlock(&(m))
#endif

#if 0
#ifdef _WIN32
	#include <windows.h>
	#define	THREAD_HND(h) HANDLE (h)
	#define THREAD_FUNC_DEC(f) long WINAPI f (void)
	#define CREATE_THREAD(func, hdl) hdl = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(func), 0, 0, 0)
	#define JOIN_THREAD(func, hdl) WaitForSingleObject(hdl, INFINITE)
	#define SEMAPHORE(s) HANDLE (s)
  #define SEM_INIT(s) s = CreateSemaphore(NULL, 0, LONG_MAX, NULL)
  #define SEM_WAIT(s) WaitForSingleObject(s, INFINITE)
	#define SEM_TIMEDWAIT(s, t, r) r = WaitForSingleObject(s, t)
	#define SEM_POST(s) ReleaseSemaphore(s, 1, NULL)
	#define MUTEX_TYPE(m) HANDLE (m)
	#define MUTEX_INIT(m) (m) = CreateMutex(NULL, FALSE, NULL)
	#define MUTEX_LOCK(m) WaitForSingleObject(m, INFINITE)
	#define MUTEX_UNLOCK(m) ReleaseMutex(m)
#else
	#include <pthread.h>
	#include <unistd.h>
	#include <semaphore.h>
  #include <arpa/inet.h>
	#define	THREAD_HND(h) pthread_t (h)
	#define THREAD_FUNC_DEC(f) void *f (void *arg)
	#define CREATE_THREAD(func, hdl) pthread_create(&hdl, NULL, func, NULL)
	#define JOIN_THREAD(func, hdl) pthread_join(hdl, NULL)
	#define SEMAPHORE(s) sem_t (s)
	#define SEM_INIT(s) sem_init(&(s), 1, 0)
	#define SEM_TIMEDWAIT(s, t, r) { struct timespec ts;  ts.tv_sec = 0; ts.tv_nsec = t*1000000; r = sem_timedwait(&(s), &ts); }
	#define SEM_WAIT(s) sem_wait(&(s))
	#define SEM_POST(s) sem_post(&(s))
	#define PIPE(p) int (p)[2]
  #define PIPE_INIT(p) { pipe(p); }
	#define PIPE_WAIT(p) { char sigval; fd_set fds; FD_ZERO(&fds); FD_SET(p[0],&fds); select(p[0]+1, &fds, NULL, NULL, NULL); read(p[0], &sigval, 1); }
	#define PIPE_POST(p) { char sig = '+'; write(p[1], &sig, 1); }
	#define PIPE_CLOSE(p) { close(p[0]); close(p[1]); }

	#define MUTEX_TYPE(m) pthread_mutex_t (m) = PTHREAD_MUTEX_INITIALIZER
	#define MUTEX_INIT(m)
	#define MUTEX_LOCK(m) pthread_mutex_lock(&(m))
	#define MUTEX_UNLOCK(m) pthread_mutex_unlock(&(m))
#endif
#endif



#ifdef _WIN32
  #include <windows.h>
  #define THREAD_HND(h) HANDLE (h)
  #define THREAD_FUNC_DEC(f, arg) long WINAPI f (_In_ LPVOID arg)
  #define CREATE_THREAD(func, hdl, arg) hdl = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(func), (__drv_aliasesMem LPVOID)arg, 0, 0)
  #define JOIN_THREAD(func, hdl) WaitForSingleObject(hdl, INFINITE)
#else
  #if defined(__LINUX__) || defined(__linux__)
//    #include <netinet/tcp.h>
  #endif


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

#if 0
#define XSCOPE_SOCKET_MSG_EVENT_STATS 3
#define XSCOPE_SOCKET_MSG_EVENT_TARGET_DATA 4
#define XSCOPE_SOCKET_MSG_EVENT_CONNECT 5
#define XSCOPE_SOCKET_MSG_EVENT_ALL_REGISTERED 6
#define XSCOPE_SOCKET_MSG_EVENT_CLOCK 7
#define XSCOPE_SOCKET_MSG_EVENT_PRINT 8
#endif

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

static xmos_record_t* buffer_pool[MAX_CHANNELS];//[NUM_BUF_ENTRIES];

static xmos_record_t* write_ptr[MAX_CHANNELS];
static xmos_record_t* read_ptr[MAX_CHANNELS];

//static unsigned long long data_available_in_us = 0;
//static unsigned           records_available    = 0;

static MUTEX_TYPE(buffer_mutex);

static int xmos_recv(struct dev_context *devc, void* buffer, size_t length)
{
//printf("xmos_recv(): devc %p, fd %d, length %d\n", devc, devc->socket, length);

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

// printf("xSCOPE record event (id [%d] timestamp [%lld] value [%lld] length [%d])\n",id, timestamp, dataval, length);

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

#if 0
{
	static unsigned r = 0;
	if (0 == r % 1000) {
		printf("xscope_record() : id %d, timestamp %lu, write_ptr[id] %p\n", id, timestamp, write_ptr[id]);
	}
	r++;
}
#endif
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
// printf("xscope_record() : FULL : id %d, timestamp %lu, write_ptr[id] %p\n", id, timestamp, write_ptr[id]);
        if (read_ptr[id] == &buffer_pool[id][NUM_BUF_ENTRIES-1] ) {
            // wrap back to start
            read_ptr[id] = buffer_pool[id];
        } else {
            read_ptr[id]++;
        }
    }

//    data_available_in_us = (timestamp - read_ptr[i]->timestamp)/1000000;

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

//printf("xmos_record_iter_init: buffer_pool %p, read_ptr[i] %p, write_ptr[i] %p\n", buffer_pool, read_ptr[i], write_ptr[i]);
	iter->data = read_ptr[channel];
}

static void xmos_record_iter_done(xmos_iter_t* iter, unsigned channel) {

//printf("xmos_record_iter_done: buffer_pool %p, read_ptr[i] %p, write_ptr[i] %p, iter->data %p\n", buffer_pool, read_ptr[i], write_ptr[i], iter->data);

	read_ptr[channel] = iter->data;

	MUTEX_UNLOCK(buffer_mutex);
}

static int xmos_record_iter_next(xmos_iter_t* iter, unsigned channel, xmos_record_t** record) {

    xmos_record_t* next_ptr = (xmos_record_t*)iter->data;
    iter->last_data = next_ptr;

//printf("xmos_record_iter_next: buffer_pool %p, read_ptr[i] %p, write_ptr[i] %p, iter->data %p\n", buffer_pool, read_ptr[i], write_ptr[i], iter->data);
#if 0
{
	static unsigned r = 0;
	if (0 == r % 1000) {
		printf("xmos_record_iter_next() : channel %d, write_ptr[channel] %p\n", channel, write_ptr[channel]);
	}
	r++;
}
#endif


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
//printf(">>>> xscope_ep_process()\n");


	uint8_t header = 0;

	if (xmos_recv(devc, (char *)&header, 1) < 1) {
		sr_err("XMOS: failed to read header\n");
		return SR_ERR;
	}

	switch (header) {
	default:
	  	sr_err("XMOS: unexpected header %d\n", (int)header);
    	return SR_ERR;

#if 0  	
    case XSCOPE_SOCKET_MSG_EVENT_STATS:
      {
      unsigned int data = 0;
      if (xmos_recv(devc, (char *)&length, 4) == 4) {
        return -1;
      }
      if (length == 0) {
        return -1;
      } else {
        payload = (char *)malloc(length);
        if (xmos_recv(devc, payload, length) == XSCOPE_EP_FAILURE)
          return -1;
      }
      memcpy((char*)&data, &payload[0], 4);
      stats_cb(0, data);
      break;
      }
#endif
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
#if 0
        if (xscope_ep_fixed_record_len) {
          // Fixed length record packets, no length, only unsigned int data
          char recvBytes[4];
          if (xmos_recv(devc, (char *)recvBytes, 4) == XSCOPE_EP_FAILURE)
            return -1;
          dataval = *(unsigned *)recvBytes;
        } else {
#endif
		if (xmos_recv(devc, (char *)&length, sizeof(length))<(int)sizeof(length)) {
			sr_err("XMOS: Failed read flagB\n");
			return SR_ERR;
		}
//printf("XSCOPE_SOCKET_MSG_EVENT_RECORD: id %d, flagA %d, flagB %d, length %d\n", (int)id, flagA, flagB, length);
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
//        }
		if (xmos_recv(devc, (char *)&timestamp, sizeof(timestamp))<(int)sizeof(timestamp)) {
			sr_err("XMOS: Failed read timestamp\n");
			return SR_ERR;
		}

		xscope_record(id, timestamp, length, dataval, databytes);

        g_free(databytes);
        break;
      }
#if 0      
    case XSCOPE_SOCKET_MSG_EVENT_TARGET_DATA:
      {
      unsigned int msg = 0;
      if (xmos_recv(devc, (char *)&msg, 4) == XSCOPE_EP_FAILURE)
        return -1;
      break;
      }
    case XSCOPE_SOCKET_MSG_EVENT_PRINT:
      {
      unsigned int length = 0;
      unsigned char *databytes = NULL;
      unsigned long long timestamp = 0;

      if (xmos_recv(devc, (char *)&timestamp, 8) == XSCOPE_EP_FAILURE)
        return -1;
      if (xmos_recv(devc, (char *)&length, 4) == XSCOPE_EP_FAILURE)
        return -1;
      if (length) {
        databytes = (unsigned char *)malloc(length);
        if (xscope_tcp_read_data(devc, (char *)databytes, length) == XSCOPE_EP_FAILURE)
          return -1;
      }

      if (print_cb != NULL) {
        print_cb(timestamp, length, databytes);
      }
      free(databytes);
      break;
    }
#endif    
  }

  return 0;
}

#if 0
static int xscope_ep_check_for_data() {
  int result = 0;
  if (xscope_ep_socket_ready(xscope_ep_socket_fd) == 1) {
    result = xscope_ep_process();
    return result;
  }
  return 0;
}

XSCOPE_EP_DLL_EXPORT int xscope_ep_set_register_cb(xscope_ep_register_fptr registration) {
  if (xscope_ep_socket_fd)
    return XSCOPE_EP_FAILURE;

  register_cb = registration;

  return XSCOPE_EP_SUCCESS;
}

XSCOPE_EP_DLL_EXPORT int xscope_ep_set_record_cb(xscope_ep_record_fptr record) {
  if (xscope_ep_socket_fd)
    return XSCOPE_EP_FAILURE;

  record_cb = record;

  return XSCOPE_EP_SUCCESS;
}

XSCOPE_EP_DLL_EXPORT int xscope_ep_set_stats_cb(xscope_ep_stats_fptr stats) {
  if (xscope_ep_socket_fd)
    return XSCOPE_EP_FAILURE;

  stats_cb = stats;

  return XSCOPE_EP_SUCCESS;
}

XSCOPE_EP_DLL_EXPORT int xscope_ep_set_print_cb(xscope_ep_print_fptr print) {
  if (xscope_ep_socket_fd)
    return XSCOPE_EP_FAILURE;

  print_cb = print;

  return XSCOPE_EP_SUCCESS;
}

XSCOPE_EP_DLL_EXPORT int xscope_ep_set_exit_cb(xscope_ep_exit_fptr exit) {
  if (xscope_ep_socket_fd)
    return XSCOPE_EP_FAILURE;

  exit_cb = exit;

  return XSCOPE_EP_SUCCESS;
}

#endif


#if 0
XSCOPE_EP_DLL_EXPORT int xscope_ep_connect(const char *ipaddr, const char *port) {
  if (xscope_ep_data_thread_run) {
    // xscope_ep_connect() has been called already
    return XSCOPE_EP_FAILURE;
  }

  int result = 0;

  ErrorHandler eh;

  // Turn off the exit on error state in the error handler
  eh.setExitOnError(false);
  eh.setTraceOnError(false);

  if (xscope_ep_socket) {
    delete xscope_ep_socket;
    xscope_ep_socket = NULL;
    xscope_ep_socket_fd = 0;
  }

  xscope_ep_socket = new Socket(&eh);
  result = xscope_ep_socket->socConnect(ipaddr, port);
  if (result == -1) {
    // fprintf(stderr, "Failed: xscope_ep_socket->socConnect(ipaddr %s, port %s)\n", ipaddr, port);
    return XSCOPE_EP_FAILURE;
  }

  xscope_ep_socket_fd = xscope_ep_socket->getSocketFd();

#if defined(__LINUX__) || defined(__linux__)
  // Set TCP_NODELAY to disable the NAGLE algorithm which "collates data" before sending instead of sending immediately
  int flag = 1;
  result = setsockopt(xscope_ep_socket_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));

  if (result == -1) {
    fprintf(stderr, "xscope_ep_connect() ERROR: couldn't setsockopt(TCP_NODELAY) errno=%d\n", errno);
    return XSCOPE_EP_FAILURE;
  }
#endif

  const unsigned MAX_TRYS = 10;
  // Do we need to loop?
  for (unsigned trys = 0; trys<MAX_TRYS; trys++) {
    // request is the "flags" byte:
    // #define XSCOPE_SERVER_REGISTRATION 1
    // #define XSCOPE_SERVER_RECORDS 2
    // #define XSCOPE_SERVER_STATS 4
    // #define XSCOPE_SERVER_PRINT 8

    // So 0xB == XSCOPE_SERVER_PRINT | XSCOPE_SERVER_RECORDS | XSCOPE_SERVER_REGISTRATION
    char request = 0xB;

    int bytesSent = xscope_ep_socket->socReply(xscope_ep_socket_fd, &request, 1);
    if (bytesSent == 1) {
      xscope_ep_fixed_record_len = 0;

      // Start xscope data thread
      xscope_ep_data_thread_run = 1;
      CREATE_THREAD(xscope_ep_data_thread, xscope_ep_data_thread_handle);

      return XSCOPE_EP_SUCCESS;
    } else {
      fprintf(stderr, "Failed: xscope_ep_socket->socReply() bytesSent=%d\n", bytesSent);
    }
  }

  return XSCOPE_EP_FAILURE;
}

#endif

#if 0
XSCOPE_EP_DLL_EXPORT int xscope_ep_disconnect(void) {
  if (xscope_ep_data_thread_run) {
    xscope_ep_data_thread_run = 0;
    JOIN_THREAD(xscope_ep_data_thread, xscope_ep_data_thread_handle);

    if (exit_cb != NULL) {
      exit_cb();
    }
  }
  if (xscope_ep_socket_fd != 0) {
    delete xscope_ep_socket;
    xscope_ep_socket = NULL;
    xscope_ep_socket_fd = 0;
  }

  return XSCOPE_EP_SUCCESS;
}
#endif

#if 0
XSCOPE_EP_DLL_EXPORT int xscope_ep_request_registered(void) {
  if (xscope_ep_data_thread_run) {
    return XSCOPE_EP_SUCCESS;
  } else {
    return XSCOPE_EP_FAILURE;
  }
}

XSCOPE_EP_DLL_EXPORT int xscope_ep_request_stats(void) {
  if (xscope_ep_data_thread_run) {
    char request = XSCOPE_SOCKET_MSG_EVENT_STATS;
    int bytesSent = xscope_ep_socket->socReply(xscope_ep_socket_fd, &request, 1);
    if (bytesSent == 1) {
      return XSCOPE_EP_SUCCESS;
    }
  }

  return XSCOPE_EP_SUCCESS;
}

XSCOPE_EP_DLL_EXPORT int xscope_ep_request_upload(unsigned int length, const unsigned char *data) {
  // Send request to xscope server
  // Max packet size of 256 bytes
  // Server responds with data remaining
  // 0 means can send more otherwise data is still to be recieved

  if (xscope_ep_data_thread_run && length <= 256) {
    char request = XSCOPE_SOCKET_MSG_EVENT_TARGET_DATA;
    char *requestBuffer = (char *)malloc(sizeof(char)+sizeof(int)+length);
    int requestBufIndex = 0;

    requestBuffer[requestBufIndex] = request;
    requestBufIndex+=1;
    *(unsigned int *)&requestBuffer[requestBufIndex] = length;
    requestBufIndex+=4;
    memcpy(&requestBuffer[requestBufIndex], data, length);
    requestBufIndex+=length;

    xscope_ep_socket->socReply(xscope_ep_socket_fd, requestBuffer, requestBufIndex);
    free(requestBuffer);

    return XSCOPE_EP_SUCCESS;
  }

  return XSCOPE_EP_FAILURE;
}
#endif

//
// ******* xscope_endpoint.cpp **************
//


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
#if 0
			xscope_ep_fixed_record_len = 0;
#endif
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

#if 0
SR_PRIV int xmos_tcp_drain(struct dev_context *devc)
{
	char *buf = g_malloc(1024);
	fd_set rset;
	int ret, len = 0;
	struct timeval tv;

	FD_ZERO(&rset);
	FD_SET(devc->socket, &rset);

	/* 25ms timeout */
	tv.tv_sec = 0;
	tv.tv_usec = 25 * 1000;

	do {
		ret = select(devc->socket + 1, &rset, NULL, NULL, &tv);
		if (ret > 0)
			len += xmos_recv(devc, buf, 1024);
	} while (ret > 0);

	sr_spew("Drained %d bytes of data.", len);

	g_free(buf);

	return SR_OK;
}
#endif

#if 0
static int xmos_tcp_get_string(struct dev_context *devc, const char *cmd,
				      char **tcp_resp)
{
	GString *response = g_string_sized_new(1024);
	int len;
	gint64 timeout;

	*tcp_resp = NULL;
	if (cmd) {
		if (xmos_tcp_send_cmd(devc, cmd) != SR_OK)
			return SR_ERR;
	}

	timeout = g_get_monotonic_time() + devc->read_timeout;
	len = xmos_recv(devc, response->str,
					response->allocated_len);

	if (len < 0) {
		g_string_free(response, TRUE);
		return SR_ERR;
	}

	if (len > 0)
		g_string_set_size(response, len);

	if (g_get_monotonic_time() > timeout) {
		sr_err("Timed out waiting for response.");
		g_string_free(response, TRUE);
		return SR_ERR_TIMEOUT;
	}

	/* Remove trailing newline if present */
	if (response->len >= 1 && response->str[response->len - 1] == '\n')
		g_string_truncate(response, response->len - 1);

	/* Remove trailing carriage return if present */
	if (response->len >= 1 && response->str[response->len - 1] == '\r')
		g_string_truncate(response, response->len - 1);

	sr_spew("Got response: '%.70s', length %" G_GSIZE_FORMAT ".",
		response->str, response->len);

	*tcp_resp = g_string_free(response, FALSE);

	return SR_OK;
}
#endif

#if 0
static int xmos_tcp_get_int(struct dev_context *devc,
				   const char *cmd, int *response)
{
	int ret;
	char *resp = NULL;

	ret = xmos_tcp_get_string(devc, cmd, &resp);
	if (!resp && ret != SR_OK)
		return ret;

	if (sr_atoi(resp, response) == SR_OK)
		ret = SR_OK;
	else
		ret = SR_ERR_DATA;

	g_free(resp);

	return ret;
}
#endif

static int xmos_detect(struct dev_context *devc)
{
printf(">>>> xmos_detect()\n");
#if 0
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

#if 0
static int xmos_get_buffersize(struct dev_context *devc)
{
	return xmos_tcp_get_int(devc, "memalloc",
		(int *)&devc->buffersize);
	return 0;
}
#endif

#if 0
static int xmos_set_buffersize(struct dev_context *devc)
{
	int ret;
	char *resp;

	xmos_tcp_send_cmd(devc, "memalloc %" PRIu32, devc->buffersize);
	ret = xmos_tcp_get_string(devc, NULL, &resp);
	if (ret == SR_OK && !g_ascii_strncasecmp(resp, "ok", 2))
		ret = SR_OK;
	else
		ret = SR_ERR;

	g_free(resp);

	return ret;
	return 0;
}
#endif

#if 0
static int xmos_get_samplerate(struct dev_context *devc)
{
	int arg, err;

	err = xmos_tcp_get_int(devc, "samplerate", &arg);
	if (err)
		return err;

	devc->cur_samplerate = arg;
	return SR_OK;
}
#endif

#if 0
static int xmos_set_samplerate(struct dev_context *devc)
{
	int ret;
	char *resp;

	xmos_tcp_send_cmd(devc, "samplerate %" PRIu32,
		(uint32_t)devc->cur_samplerate);
	ret = xmos_tcp_get_string(devc, NULL, &resp);
	if (ret == SR_OK && !g_ascii_strncasecmp(resp, "ok", 2))
		ret = SR_OK;
	else
		ret = SR_ERR;

	g_free(resp);

	return ret;
	return SR_OK;
}
#endif

#if 0
static int xmos_get_sampleunit(struct dev_context *devc)
{
	return xmos_tcp_get_int(devc, "sampleunit",
		(int *)&devc->sampleunit);
	return 0;
}
#endif

#if 0
static int xmos_set_sampleunit(struct dev_context *devc)
{
	int ret;
	char *resp;

	xmos_tcp_send_cmd(devc, "sampleunit %" PRIu32, devc->sampleunit);
	ret = xmos_tcp_get_string(devc, NULL, &resp);
	if (ret == SR_OK && !g_ascii_strncasecmp(resp, "ok", 2))
		ret = SR_OK;
	else
		ret = SR_ERR;

	g_free(resp);

	return ret;
	return SR_OK;
}
#endif

#if 0
static int xmos_get_triggerflags(struct dev_context *devc)
{
	return xmos_tcp_get_int(devc, "triggerflags",
		(int *)&devc->triggerflags);
	return 0;
}
#endif

#if 0
static int xmos_set_triggerflags(struct dev_context *devc)
{
	int ret;
	char *resp;

	xmos_tcp_send_cmd(devc, "triggerflags %" PRIu32, devc->triggerflags);
	ret = xmos_tcp_get_string(devc, NULL, &resp);
	if (ret == SR_OK && !g_ascii_strncasecmp(resp, "ok", 2))
		ret = SR_OK;
	else
		ret = SR_ERR;

	g_free(resp);

	return ret;
	return 0;
}
#endif

#if 0
static int xmos_get_lasterror(struct dev_context *devc)
{
	devc->last_error = 0;
	return SR_OK;
}
#endif

#if 0
static int xmos_start(struct dev_context *devc)
{
	xmos_tcp_drain(devc);

	return xmos_tcp_send_cmd(devc, "get");
}

static int xmos_stop(struct dev_context *devc)
{
	return xmos_tcp_send_cmd(devc, "close");
}
#endif

#if 0
static int xmos_get_bufunitsize(struct dev_context *devc)
{
	return xmos_tcp_get_int(devc, "bufunitsize",
		(int *)&devc->bufunitsize);
	return 0;
}
#endif

#if 0
static int xmos_set_bufunitsize(struct dev_context *devc)
{
	int ret;
	char *resp;

	xmos_tcp_send_cmd(devc, "bufunitsize %" PRIu32, devc->bufunitsize);
	ret = xmos_tcp_get_string(devc, NULL, &resp);
	if (ret == SR_OK && !g_ascii_strncasecmp(resp, "ok", 2))
		ret = SR_OK;
	else
		ret = SR_ERR;

	g_free(resp);

	return ret;
	return SR_OK;
}
#endif

#if 0
static int dummy(struct dev_context *devc)
{
	(void)devc;

	return SR_ERR_NA;
}
#endif

SR_PRIV const struct xmos_ops xmos_tcp_ops = {
	.open = xmos_open,
	.close = xmos_close,
	.detect = xmos_detect,

//	.send = xmos_send,
//	.recv = xmos_recv,

	.xmos_record_iter_init = xmos_record_iter_init,
//    .xmos_record_iter_reset = xmos_record_iter_reset,
	.xmos_record_iter_next = xmos_record_iter_next,
	.xmos_record_iter_undonext = xmos_record_iter_undonext,
	.xmos_record_iter_done = xmos_record_iter_done,

#if 0
	.get_buffersize = xmos_get_buffersize,
	.set_buffersize = xmos_set_buffersize,
	.get_samplerate = xmos_get_samplerate,
	.set_samplerate = xmos_set_samplerate,
	.get_sampleunit = xmos_get_sampleunit,
	.set_sampleunit = xmos_set_sampleunit,
	.get_triggerflags = xmos_get_triggerflags,
	.set_triggerflags = xmos_set_triggerflags,
	.start = xmos_start,
	.stop = xmos_stop,

	.get_lasterror = xmos_get_lasterror,
	.get_bufunitsize = xmos_get_bufunitsize,
	.set_bufunitsize = xmos_set_bufunitsize,
#endif
};

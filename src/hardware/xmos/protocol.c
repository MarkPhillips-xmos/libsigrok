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

static xmos_iter_t xmos_record_iter;

#define  FLOG
// TODO
uint64_t xmos_get_acquisition_start_timestamp();
static uint64_t latest_timestamp = 0;

// hack TODO
uint64_t last_timestamp = UINT64_MAX;

#define MAX_CHANNELS  	256
#define MAX_SAMPLES   16384
static float    data[MAX_CHANNELS][MAX_SAMPLES];
//static unsigned long samples_just_done = 0;

// To keep host time in sync with target time apply an adjustment

static int64_t packet_host_time = -1;

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

	int record_available;

	if (!ag->ch || !ag->ch->enabled)
		goto done;

	devc = sdi->priv;

	const unsigned long sammple_period_ps = UINT64_C(1000000000000)/devc->cur_samplerate;

	packet.type = SR_DF_ANALOG;
	packet.payload = &ag->packet;

	unsigned long samples_todo = MIN(MAX_SAMPLES, ag->samples_todo);

	unsigned channel;
	channel = ag->ch->index;

	if (UINT64_MAX == last_timestamp) {
		last_timestamp = xmos_get_acquisition_start_timestamp();
	}

	#ifdef FLOG
	{
	fprintf(flog, "channel %u, xmos_get_acquisition_start_timestamp %lu\n", channel, last_timestamp);
	}
	#endif

	sample_timestamp = last_timestamp;

	#ifdef FLOG
	{
	fprintf(flog, "channel %u, samples_todo %lu, sample_timestamp %lu\n", channel, samples_todo, sample_timestamp);
	}
	unsigned datarecs = 0;
	unsigned padding = 0;
	unsigned padding2 = 0;
	#endif

	devc->xmos_tcp_ops->xmos_record_iter_init(&xmos_record_iter, channel);

	record_available = devc->xmos_tcp_ops->xmos_record_iter_next(&xmos_record_iter, channel, &record);

	while (record_available || samples_done < samples_todo) {

		if (record_available) {


			#ifdef FLOG
			{
			int64_t host_ts_ns = ((packet_host_time - devc->first_packet_host_time) * 1000 ) + ((sammple_period_ps * samples_done)/1000);


			fprintf(flog, "channel %u, sammple_period_ps %lu, sample_timestamp %lu, host_ts_ns %ld, record->timestamp %lu, record->dataval %f\n", 
				channel, sammple_period_ps, sample_timestamp, host_ts_ns, ( record->timestamp - xmos_get_acquisition_start_timestamp() ) / 1000, (float)((signed long long)(record->dataval)) );
			}
			#endif
			while (sample_timestamp < record->timestamp &&
				   samples_done < samples_todo) {

				data[channel][samples_done] = last_sample_value[channel];

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
			int64_t host_ts_ns = ((packet_host_time - devc->first_packet_host_time) * 1000 ) + ((sammple_period_ps * samples_done)/1000);

			fprintf(flog, "EMIT: channel %u, padding %u, padding2 %u, sample_timestamp %lu, host_ts_ns %ld, record->timestamp %lu, record->dataval %f\n", 
				channel, padding, padding2, sample_timestamp, host_ts_ns, ( record->timestamp - xmos_get_acquisition_start_timestamp() ) / 1000, (float)((signed long long)(record->dataval)) );
			}
			#endif


			// emit record
			data[channel][samples_done] = (float)((signed long long)(record->dataval));
			last_sample_value[channel] = data[channel][samples_done];

			sample_timestamp += sammple_period_ps;
			samples_done++;

			#ifdef FLOG
			datarecs++;
			#endif

			if (samples_todo == samples_done) {
				// reached the buffer end - send the buffer

		    	ag->packet.data = data[channel];
		    	ag->packet.num_samples = samples_done;

				sr_session_send(sdi, &packet);
#ifdef FLOG
{
fprintf(flog, "data %ld, datarecs %u, val %lu\n", samples_done, datarecs, record->dataval);
}
#endif
				goto done;
			}
		} else {
			// No record available - add padding until one becomes available
			data[channel][samples_done] = last_sample_value[channel];

			sample_timestamp += sammple_period_ps;
			samples_done++;
			#ifdef FLOG
			padding2++;
			#endif
			if (samples_todo == samples_done) {
				// reached the buffer end - send the buffer

		    	ag->packet.data = data[channel];
		    	ag->packet.num_samples = samples_done;

				sr_session_send(sdi, &packet);

				#ifdef FLOG
				{
				fprintf(flog, "pad2 %ld, datarecs %u, sample_timestamp %lu\n", samples_done, datarecs, sample_timestamp);
				}
				#endif
				// Put the record back for next time
				goto done;
			}

		}
		record_available = devc->xmos_tcp_ops->xmos_record_iter_next(&xmos_record_iter, channel, &record);

	}

done:

	devc->xmos_tcp_ops->xmos_record_iter_done(&xmos_record_iter, channel);

	ag->samples_todo -= samples_done;

	if (last_timestamp != UINT64_MAX) {
		// Only do this update if we have received a packet
		latest_timestamp = MAX(latest_timestamp, sample_timestamp);
	}
#ifdef FLOG
{
fprintf(flog, "<<<: ag->samples_todo %ld, latest_timestamp %lu\n", ag->samples_todo, latest_timestamp);
}
#endif

}

/* Callback handling data */
SR_PRIV int xmos_prepare_data(int fd, int revents, void *cb_data)
{
	struct sr_dev_inst *sdi;
	struct dev_context *devc;
	struct analog_gen *ag;
	GHashTableIter iter;
	void *value;
	uint64_t samples_todo, analog_done; //, sending_now;	
	int64_t elapsed_us, limit_us, todo_us;
//	int64_t trigger_offset;
//	int pre_trigger_samples;

	(void)fd;
	(void)revents;


	sdi = cb_data;
	devc = sdi->priv;


    packet_host_time =  g_get_monotonic_time();
    if (-1 == devc->first_packet_host_time) {
    	devc->first_packet_host_time = packet_host_time;
    }

	/* Just in case. */
	if (devc->cur_samplerate <= 0 ||
		devc->num_analog_channels <= 0) {
		sr_dev_acquisition_stop(sdi);
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

	if (devc->limit_samples > 0) {
		if (devc->limit_samples < devc->sent_samples)
			samples_todo = 0;
		else if (devc->limit_samples - devc->sent_samples < samples_todo)
			samples_todo = devc->limit_samples - devc->sent_samples;
	}

	if (samples_todo == 0) {
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

			if (ag->samples_todo == 0) {
				channels_to_complete--;
			}
		}
		// assume all channels sent the same number of samples - use count from the last channel
		if (last_timestamp != UINT64_MAX) {
			last_timestamp = latest_timestamp;
		}
	}
	analog_done = samples_todo;

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
#if 0
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
#endif
		sr_dbg("Requested number of samples reached.");
		sr_dev_acquisition_stop(sdi);
	} else if (devc->limit_frames) {
#if 0
		if (devc->sent_frame_samples == 0)
			std_session_send_frame_begin(sdi);
#endif
	}

	return G_SOURCE_CONTINUE;
}

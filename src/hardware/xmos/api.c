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


#define DEFAULT_NUM_LOGIC_CHANNELS		8
#define DEFAULT_LOGIC_PATTERN			PATTERN_SIGROK

#define DEFAULT_NUM_ANALOG_CHANNELS		5

static struct sr_dev_driver xmos_driver_info;


/* Note: No spaces allowed because of sigrok-cli. */
static const char *logic_pattern_str[] = {
	"sigrok",
	"random",
	"incremental",
	"walking-one",
	"walking-zero",
	"all-low",
	"all-high",
	"squid",
	"graycode",
};


static const uint32_t scanopts[] = {
	SR_CONF_CONN,
};

static const uint32_t drvopts[] = {
	SR_CONF_OSCILLOSCOPE,
};

#if 0
static const uint32_t scanopts[] = {
	SR_CONF_NUM_LOGIC_CHANNELS,
	SR_CONF_NUM_ANALOG_CHANNELS,
	SR_CONF_LIMIT_FRAMES,
};

static const uint32_t drvopts[] = {
	SR_CONF_DEMO_DEV,
	SR_CONF_LOGIC_ANALYZER,
	SR_CONF_OSCILLOSCOPE,
};
#endif

static const uint32_t devopts[] = {
	SR_CONF_CONTINUOUS,
	SR_CONF_LIMIT_SAMPLES | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_LIMIT_MSEC | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_LIMIT_FRAMES | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_SAMPLERATE | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
	SR_CONF_AVERAGING | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_AVG_SAMPLES | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_TRIGGER_MATCH | SR_CONF_LIST,
	SR_CONF_CAPTURE_RATIO | SR_CONF_GET | SR_CONF_SET,
};

static const uint32_t devopts_cg_logic[] = {
	SR_CONF_PATTERN_MODE | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
};

static const uint32_t devopts_cg_analog_group[] = {
	SR_CONF_AMPLITUDE | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_OFFSET | SR_CONF_GET | SR_CONF_SET,
};

static const uint32_t devopts_cg_analog_channel[] = {
	SR_CONF_MEASURED_QUANTITY | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_PATTERN_MODE | SR_CONF_GET | SR_CONF_SET | SR_CONF_LIST,
	SR_CONF_AMPLITUDE | SR_CONF_GET | SR_CONF_SET,
	SR_CONF_OFFSET | SR_CONF_GET | SR_CONF_SET,
};

static const int32_t trigger_matches[] = {
	SR_TRIGGER_ZERO,
	SR_TRIGGER_ONE,
	SR_TRIGGER_RISING,
	SR_TRIGGER_FALLING,
	SR_TRIGGER_EDGE,
};

static const uint64_t samplerates[] = {
	SR_HZ(1),
	SR_GHZ(1),
	SR_HZ(1),
};


// hack
struct sr_channel_group *g_acg;
struct sr_dev_inst *g_sdi;
static int channels_available = 0;
static int channels_ready = 0;

typedef struct {
	unsigned    id;
	const char* name;	
} all_channels_t;

all_channels_t all_channels[256];

// TODO
void add_analogue_channel(unsigned ch_id, const char* name);
void done_adding_analogue_channel(void);

void add_analogue_channel(unsigned ch_id, const char* name)
{
#if 0	
// TODO
struct sr_channel_group *acg = g_acg;
struct sr_dev_inst *sdi = g_sdi;
struct dev_context *devc = sdi->priv;

//		sdi->channel_groups = g_slist_append(sdi->channel_groups, acg);
char channel_name[16];
struct sr_channel *ch;
struct sr_channel_group *cg;
struct analog_gen *ag;

	snprintf(channel_name, 16, "A%d", ch_id);
	ch = sr_channel_new(sdi, ch_id, SR_CHANNEL_ANALOG,
			TRUE, channel_name);
	acg->channels = g_slist_append(acg->channels, ch);

	/* Every analog channel gets its own channel group as well. */
	cg = g_malloc0(sizeof(struct sr_channel_group));
	cg->name = g_strdup(channel_name);
	cg->channels = g_slist_append(NULL, ch);
	sdi->channel_groups = g_slist_append(sdi->channel_groups, cg);

	/* Every channel gets a generator struct. */
	ag = g_malloc(sizeof(struct analog_gen));
	ag->ch = ch;
	ag->mq = SR_MQ_VOLTAGE;
	ag->mq_flags = SR_MQFLAG_DC;
	ag->unit = SR_UNIT_VOLT;
	ag->amplitude = DEFAULT_ANALOG_AMPLITUDE;
	ag->offset = DEFAULT_ANALOG_OFFSET;
	sr_analog_init(&ag->packet, &ag->encoding, &ag->meaning, &ag->spec, 2);
	ag->packet.meaning->channels = cg->channels;
	ag->packet.meaning->mq = ag->mq;
	ag->packet.meaning->mqflags = ag->mq_flags;
	ag->packet.meaning->unit = ag->unit;
	ag->packet.encoding->digits = DEFAULT_ANALOG_ENCODING_DIGITS;
	ag->packet.spec->spec_digits = DEFAULT_ANALOG_SPEC_DIGITS;
//	ag->packet.data = devc->analog_patterns[pattern];
//	ag->pattern = pattern;
	ag->avg_val = 0.0f;
	ag->num_avgs = 0;
	g_hash_table_insert(devc->ch_ag, ch, ag);
#endif
	all_channels[channels_available].id = ch_id;
	all_channels[channels_available].name = g_strdup(name);
	channels_available++;
}

void done_adding_analogue_channel(void) {
	channels_ready = 1;
}


static GSList *scan(struct sr_dev_driver *di, GSList *options)
{
	struct dev_context *devc;
	struct sr_dev_inst *sdi;
	struct sr_channel *ch;
	struct sr_channel_group *cg, *acg;
	struct sr_config *src;
	struct analog_gen *ag;
	GSList *l;
	int num_logic_channels, num_analog_channels, pattern, i;
	uint64_t limit_frames;
	char channel_name[16];
	const char* conn;
	gchar **params;

//	num_logic_channels = DEFAULT_NUM_LOGIC_CHANNELS;
//	num_analog_channels = DEFAULT_NUM_ANALOG_CHANNELS;

	num_logic_channels = 0;
    num_analog_channels = 0;

    conn = NULL;
	limit_frames = DEFAULT_LIMIT_FRAMES;
	for (l = options; l; l = l->next) {
		src = l->data;
		switch (src->key) {
		case SR_CONF_NUM_LOGIC_CHANNELS:
			num_logic_channels = g_variant_get_int32(src->data);
			break;
		case SR_CONF_NUM_ANALOG_CHANNELS:
			num_analog_channels = g_variant_get_int32(src->data);
			break;
		case SR_CONF_LIMIT_FRAMES:
			limit_frames = g_variant_get_uint64(src->data);
			break;
		case SR_CONF_CONN:
			conn = g_variant_get_string(src->data, NULL);
			break;
		}
	}

	params = g_strsplit(conn, "/", 0);
	if (!params || !params[1] || !params[2]) {
		sr_err("Invalid Parameters.");
		g_strfreev(params);
		return NULL;
	}
	if (g_ascii_strncasecmp(params[0], "tcp", 3)) {
		sr_err("Only TCP (tcp-raw) protocol is currently supported.");
		g_strfreev(params);
		return NULL;
	}

	sdi = g_malloc0(sizeof(struct sr_dev_inst));
	sdi->status = SR_ST_INACTIVE;
	sdi->model = g_strdup("XMOS xSCOPE");
	sdi->vendor = g_strdup("XMOS");
    sdi->version = g_strdup("15.0.6");
//	sdi->serial_num = g_strdup(hw_info->serial_number);
//	sdi->driver = &lecroy_xstream_driver_info;
	sdi->driver = &xmos_driver_info;

	sdi->inst_type = SR_INST_USER;
	sdi->conn = sr_dev_inst_user_new(sdi->vendor, sdi->model, sdi->version);

	devc = g_malloc0(sizeof(struct dev_context));
	devc->cur_samplerate = SR_KHZ(200);
	devc->num_logic_channels = num_logic_channels;
	devc->logic_unitsize = (devc->num_logic_channels + 7) / 8;
	devc->all_logic_channels_mask = 1UL << 0;
	devc->all_logic_channels_mask <<= devc->num_logic_channels;
	devc->all_logic_channels_mask--;
	devc->logic_pattern = DEFAULT_LOGIC_PATTERN;
	devc->num_analog_channels = num_analog_channels;
	devc->limit_frames = limit_frames;
	devc->capture_ratio = 20;
	devc->stl = NULL;

	devc->read_timeout = 1000 * 1000;
	devc->address = g_strdup(params[1]);
	devc->port = g_strdup(params[2]);
	devc->xmos_tcp_ops = &xmos_tcp_ops;

	devc->socket = -1;

	if (devc->xmos_tcp_ops->open(devc) != SR_OK)
		goto err_free;
	if (devc->xmos_tcp_ops->detect(devc) != SR_OK)
		goto err_free;
//	if (devc->xmos_tcp_ops->close(devc) != SR_OK)
//		goto err_free;
	sr_info("XMOS device found at %s : %s",
		devc->address, devc->port);

while (0 == channels_ready) {
		g_usleep(10000);
}
printf("channels_ready: %d, channels_available %d\n", channels_ready, channels_available);
devc->num_analog_channels = num_analog_channels = channels_available;

	g_strfreev(params);

	if (num_logic_channels > 0) {
		/* Logic channels, all in one channel group. */
		cg = g_malloc0(sizeof(struct sr_channel_group));
		cg->name = g_strdup("Logic");
		for (i = 0; i < num_logic_channels; i++) {
			sprintf(channel_name, "D%d", i);
			ch = sr_channel_new(sdi, i, SR_CHANNEL_LOGIC, TRUE, channel_name);
			cg->channels = g_slist_append(cg->channels, ch);
		}
		sdi->channel_groups = g_slist_append(NULL, cg);
	}

	/* Analog channels, channel groups and pattern generators. */
	devc->ch_ag = g_hash_table_new(g_direct_hash, g_direct_equal);
	if (num_analog_channels > 0) {
		/*
		 * Have the waveform for analog patterns pre-generated. It's
		 * supposed to be periodic, so the generator just needs to
		 * access the prepared sample data (DDS style).
		 */
//		xmos_generate_analog_pattern(devc);

		pattern = 0;
		/* An "Analog" channel group with all analog channels in it. */
		acg = g_malloc0(sizeof(struct sr_channel_group));
#if 0
// TODO
g_acg = acg;		
g_sdi = sdi;
#endif
		acg->name = g_strdup("Analog");
		sdi->channel_groups = g_slist_append(sdi->channel_groups, acg);

		for (i = 0; i < num_analog_channels; i++) {
//			snprintf(channel_name, 16, "A%d", i);
strcpy(channel_name, all_channels[i].name);
			ch = sr_channel_new(sdi, i + num_logic_channels, SR_CHANNEL_ANALOG,
					TRUE, channel_name);
			acg->channels = g_slist_append(acg->channels, ch);

			/* Every analog channel gets its own channel group as well. */
			cg = g_malloc0(sizeof(struct sr_channel_group));
// TODO			cg->name = g_strdup(channel_name);
cg->name = (char*) all_channels[i].name;
			cg->channels = g_slist_append(NULL, ch);
			sdi->channel_groups = g_slist_append(sdi->channel_groups, cg);

			/* Every channel gets a generator struct. */
			ag = g_malloc(sizeof(struct analog_gen));
			ag->ch = ch;
			ag->mq = SR_MQ_VOLTAGE;
			ag->mq_flags = SR_MQFLAG_DC;
			ag->unit = SR_UNIT_VOLT;
			ag->amplitude = DEFAULT_ANALOG_AMPLITUDE;
			ag->offset = DEFAULT_ANALOG_OFFSET;
			sr_analog_init(&ag->packet, &ag->encoding, &ag->meaning, &ag->spec, 2);
			ag->packet.meaning->channels = cg->channels;
			ag->packet.meaning->mq = ag->mq;
			ag->packet.meaning->mqflags = ag->mq_flags;
			ag->packet.meaning->unit = ag->unit;
			ag->packet.encoding->digits = DEFAULT_ANALOG_ENCODING_DIGITS;
			ag->packet.spec->spec_digits = DEFAULT_ANALOG_SPEC_DIGITS;
			ag->packet.data = devc->analog_patterns[pattern];
			ag->pattern = pattern;
			ag->avg_val = 0.0f;
			ag->num_avgs = 0;
			ag->samples_todo = 0;
			g_hash_table_insert(devc->ch_ag, ch, ag);

			if (++pattern == ARRAY_SIZE(analog_pattern_str))
				pattern = 0;
		}
	}

	sdi->priv = devc;
	return std_scan_complete(di, g_slist_append(NULL, sdi));

err_free:
	return NULL;
}

static void clear_helper(struct dev_context *devc)
{
	GHashTableIter iter;
	void *value;

//	xmos_free_analog_pattern(devc);

	/* Analog generators. */
	g_hash_table_iter_init(&iter, devc->ch_ag);
	while (g_hash_table_iter_next(&iter, NULL, &value))
		g_free(value);
	g_hash_table_unref(devc->ch_ag);
}

static int dev_clear(const struct sr_dev_driver *di)
{
	return std_dev_clear_with_callback(di, (std_dev_clear_callback)clear_helper);
}

static int config_get(uint32_t key, GVariant **data,
	const struct sr_dev_inst *sdi, const struct sr_channel_group *cg)
{
	struct dev_context *devc;
	struct sr_channel *ch;
	struct analog_gen *ag;
	GVariant *mq_arr[2];
	int pattern;

	if (!sdi)
		return SR_ERR_ARG;

	devc = sdi->priv;
	switch (key) {
	case SR_CONF_SAMPLERATE:
		*data = g_variant_new_uint64(devc->cur_samplerate);
		break;
	case SR_CONF_LIMIT_SAMPLES:
		*data = g_variant_new_uint64(devc->limit_samples);
		break;
	case SR_CONF_LIMIT_MSEC:
		*data = g_variant_new_uint64(devc->limit_msec);
		break;
	case SR_CONF_LIMIT_FRAMES:
		*data = g_variant_new_uint64(devc->limit_frames);
		break;
	case SR_CONF_AVERAGING:
		*data = g_variant_new_boolean(devc->avg);
		break;
	case SR_CONF_AVG_SAMPLES:
		*data = g_variant_new_uint64(devc->avg_samples);
		break;
	case SR_CONF_MEASURED_QUANTITY:
		if (!cg)
			return SR_ERR_CHANNEL_GROUP;
		/* Any channel in the group will do. */
		ch = cg->channels->data;
		if (ch->type != SR_CHANNEL_ANALOG)
			return SR_ERR_ARG;
		ag = g_hash_table_lookup(devc->ch_ag, ch);
		mq_arr[0] = g_variant_new_uint32(ag->mq);
		mq_arr[1] = g_variant_new_uint64(ag->mq_flags);
		*data = g_variant_new_tuple(mq_arr, 2);
		break;
	case SR_CONF_PATTERN_MODE:
		if (!cg)
			return SR_ERR_CHANNEL_GROUP;
		/* Any channel in the group will do. */
		ch = cg->channels->data;
		if (ch->type == SR_CHANNEL_LOGIC) {
			pattern = devc->logic_pattern;
			*data = g_variant_new_string(logic_pattern_str[pattern]);
		} else if (ch->type == SR_CHANNEL_ANALOG) {
			ag = g_hash_table_lookup(devc->ch_ag, ch);
			pattern = ag->pattern;
			*data = g_variant_new_string(analog_pattern_str[pattern]);
		} else
			return SR_ERR_BUG;
		break;
	case SR_CONF_AMPLITUDE:
		if (!cg)
			return SR_ERR_CHANNEL_GROUP;
		/* Any channel in the group will do. */
		ch = cg->channels->data;
		if (ch->type != SR_CHANNEL_ANALOG)
			return SR_ERR_ARG;
		ag = g_hash_table_lookup(devc->ch_ag, ch);
		*data = g_variant_new_double(ag->amplitude);
		break;
	case SR_CONF_OFFSET:
		if (!cg)
			return SR_ERR_CHANNEL_GROUP;
		/* Any channel in the group will do. */
		ch = cg->channels->data;
		if (ch->type != SR_CHANNEL_ANALOG)
			return SR_ERR_ARG;
		ag = g_hash_table_lookup(devc->ch_ag, ch);
		*data = g_variant_new_double(ag->offset);
		break;
	case SR_CONF_CAPTURE_RATIO:
		*data = g_variant_new_uint64(devc->capture_ratio);
		break;
	default:
		return SR_ERR_NA;
	}

	return SR_OK;
}

static int config_set(uint32_t key, GVariant *data,
	const struct sr_dev_inst *sdi, const struct sr_channel_group *cg)
{
	struct dev_context *devc;
	struct analog_gen *ag;
	struct sr_channel *ch;
	GVariant *mq_tuple_child;
	GSList *l;
	int logic_pattern, analog_pattern;

	devc = sdi->priv;

	switch (key) {
	case SR_CONF_SAMPLERATE:
		devc->cur_samplerate = g_variant_get_uint64(data);
		break;
	case SR_CONF_LIMIT_SAMPLES:
		devc->limit_msec = 0;
		devc->limit_samples = g_variant_get_uint64(data);
		break;
	case SR_CONF_LIMIT_MSEC:
		devc->limit_msec = g_variant_get_uint64(data);
		devc->limit_samples = 0;
		break;
	case SR_CONF_LIMIT_FRAMES:
		devc->limit_frames = g_variant_get_uint64(data);
		break;
	case SR_CONF_AVERAGING:
		devc->avg = g_variant_get_boolean(data);
		sr_dbg("%s averaging", devc->avg ? "Enabling" : "Disabling");
		break;
	case SR_CONF_AVG_SAMPLES:
		devc->avg_samples = g_variant_get_uint64(data);
		sr_dbg("Setting averaging rate to %" PRIu64, devc->avg_samples);
		break;
	case SR_CONF_MEASURED_QUANTITY:
		if (!cg)
			return SR_ERR_CHANNEL_GROUP;
		for (l = cg->channels; l; l = l->next) {
			ch = l->data;
			if (ch->type != SR_CHANNEL_ANALOG)
				return SR_ERR_ARG;
			ag = g_hash_table_lookup(devc->ch_ag, ch);
			mq_tuple_child = g_variant_get_child_value(data, 0);
			ag->mq = g_variant_get_uint32(mq_tuple_child);
			mq_tuple_child = g_variant_get_child_value(data, 1);
			ag->mq_flags = g_variant_get_uint64(mq_tuple_child);
			g_variant_unref(mq_tuple_child);
		}
		break;
	case SR_CONF_PATTERN_MODE:
		if (!cg)
			return SR_ERR_CHANNEL_GROUP;
		logic_pattern = std_str_idx(data, ARRAY_AND_SIZE(logic_pattern_str));
		analog_pattern = std_str_idx(data, ARRAY_AND_SIZE(analog_pattern_str));
		if (logic_pattern < 0 && analog_pattern < 0)
			return SR_ERR_ARG;
		for (l = cg->channels; l; l = l->next) {
			ch = l->data;
			if (ch->type == SR_CHANNEL_LOGIC) {
				if (logic_pattern == -1)
					return SR_ERR_ARG;
				sr_dbg("Setting logic pattern to %s",
						logic_pattern_str[logic_pattern]);
				devc->logic_pattern = logic_pattern;
				/* Might as well do this now, these are static. */
				if (logic_pattern == PATTERN_ALL_LOW)
					memset(devc->logic_data, 0x00, LOGIC_BUFSIZE);
				else if (logic_pattern == PATTERN_ALL_HIGH)
					memset(devc->logic_data, 0xff, LOGIC_BUFSIZE);
			} else if (ch->type == SR_CHANNEL_ANALOG) {
				if (analog_pattern == -1)
					return SR_ERR_ARG;
				sr_dbg("Setting analog pattern for channel %s to %s",
						ch->name, analog_pattern_str[analog_pattern]);
				ag = g_hash_table_lookup(devc->ch_ag, ch);
				ag->pattern = analog_pattern;
			} else
				return SR_ERR_BUG;
		}
		break;
	case SR_CONF_AMPLITUDE:
		if (!cg)
			return SR_ERR_CHANNEL_GROUP;
		for (l = cg->channels; l; l = l->next) {
			ch = l->data;
			if (ch->type != SR_CHANNEL_ANALOG)
				return SR_ERR_ARG;
			ag = g_hash_table_lookup(devc->ch_ag, ch);
			ag->amplitude = g_variant_get_double(data);
		}
		break;
	case SR_CONF_OFFSET:
		if (!cg)
			return SR_ERR_CHANNEL_GROUP;
		for (l = cg->channels; l; l = l->next) {
			ch = l->data;
			if (ch->type != SR_CHANNEL_ANALOG)
				return SR_ERR_ARG;
			ag = g_hash_table_lookup(devc->ch_ag, ch);
			ag->offset = g_variant_get_double(data);
		}
		break;
	case SR_CONF_CAPTURE_RATIO:
		devc->capture_ratio = g_variant_get_uint64(data);
		break;
	default:
		return SR_ERR_NA;
	}

	return SR_OK;
}

static int config_list(uint32_t key, GVariant **data,
	const struct sr_dev_inst *sdi, const struct sr_channel_group *cg)
{
	struct sr_channel *ch;

#if 0
// from lecroy-xtream
	case SR_CONF_SCAN_OPTIONS:
		return STD_CONFIG_LIST(key, data, sdi, cg, scanopts, NO_OPTS, NO_OPTS);
	case SR_CONF_DEVICE_OPTIONS:
		if (!cg)
			return STD_CONFIG_LIST(key, data, sdi, cg, NO_OPTS, drvopts, devopts);
		*data = std_gvar_array_u32(ARRAY_AND_SIZE(devopts_cg_analog));
		break;
#endif

	if (!cg) {
		switch (key) {
		case SR_CONF_SCAN_OPTIONS:
			return STD_CONFIG_LIST(key, data, sdi, cg, scanopts, NO_OPTS, NO_OPTS);
		case SR_CONF_DEVICE_OPTIONS:
			return STD_CONFIG_LIST(key, data, sdi, cg, scanopts, drvopts, devopts);
		case SR_CONF_SAMPLERATE:
			*data = std_gvar_samplerates_steps(ARRAY_AND_SIZE(samplerates));
			break;
		case SR_CONF_TRIGGER_MATCH:
			*data = std_gvar_array_i32(ARRAY_AND_SIZE(trigger_matches));
			break;
		default:
			return SR_ERR_NA;
		}
	} else {
		ch = cg->channels->data;
		switch (key) {
		case SR_CONF_DEVICE_OPTIONS:
			if (ch->type == SR_CHANNEL_LOGIC)
				*data = std_gvar_array_u32(ARRAY_AND_SIZE(devopts_cg_logic));
			else if (ch->type == SR_CHANNEL_ANALOG) {
				if (strcmp(cg->name, "Analog") == 0)
					*data = std_gvar_array_u32(ARRAY_AND_SIZE(devopts_cg_analog_group));
				else
					*data = std_gvar_array_u32(ARRAY_AND_SIZE(devopts_cg_analog_channel));
			}
			else
				return SR_ERR_BUG;
			break;
		case SR_CONF_PATTERN_MODE:
			/* The analog group (with all 4 channels) shall not have a pattern property. */
			if (strcmp(cg->name, "Analog") == 0)
				return SR_ERR_NA;

			if (ch->type == SR_CHANNEL_LOGIC)
				*data = g_variant_new_strv(ARRAY_AND_SIZE(logic_pattern_str));
			else if (ch->type == SR_CHANNEL_ANALOG)
				*data = g_variant_new_strv(ARRAY_AND_SIZE(analog_pattern_str));
			else
				return SR_ERR_BUG;
			break;
		default:
			return SR_ERR_NA;
		}
	}

	return SR_OK;
}

void collect_data(void);
static int dev_open(struct sr_dev_inst *sdi)
{
fprintf(stderr, ">>>> xmos: dev_open()\n");
#if 0
	if (sr_scpi_open(sdi->conn) != SR_OK)
		return SR_ERR;

	if (lecroy_xstream_state_get(sdi) != SR_OK)
		return SR_ERR;
#endif
//	struct dev_context *devc = sdi->priv;
//	return devc->xmos_tcp_ops->open(devc);
	collect_data();
	sdi++;
	return SR_OK;
}

static int dev_close(struct sr_dev_inst *sdi)
{
fprintf(stderr, ">>>> xmos: dev_close()\n");
#if 0
	return sr_scpi_close(sdi->conn);
	struct dev_context *devc = sdi->priv;
	return devc->xmos_tcp_ops->close(devc);
#endif
	sdi++;
	return SR_OK;

}

// HACK
void xmos_acquisition_start();
extern uint64_t last_timestamp;

static int dev_acquisition_start(const struct sr_dev_inst *sdi)
{
fprintf(stderr, ">>>> xmos: dev_acquisition_start()\n");

	struct dev_context *devc;
	GSList *l;
	struct sr_channel *ch;
	int bitpos;
	uint8_t mask;
	struct sr_trigger *trigger;

	devc = sdi->priv;
	devc->sent_samples = 0;
	devc->sent_frame_samples = 0;

	/* Setup triggers */
	if ((trigger = sr_session_trigger_get(sdi->session))) {
		int pre_trigger_samples = 0;
		if (devc->limit_samples > 0)
			pre_trigger_samples = (devc->capture_ratio * devc->limit_samples) / 100;
		devc->stl = soft_trigger_logic_new(sdi, trigger, pre_trigger_samples);
		if (!devc->stl)
			return SR_ERR_MALLOC;

		/* Disable all analog channels since using them when there are logic
		 * triggers set up would require having pre-trigger sample buffers
		 * for analog sample data.
		 */
		for (l = sdi->channels; l; l = l->next) {
			ch = l->data;
			if (ch->type == SR_CHANNEL_ANALOG)
				ch->enabled = FALSE;
		}
	}
	devc->trigger_fired = FALSE;

	/*
	 * Determine the numbers of logic and analog channels that are
	 * involved in the acquisition. Determine an offset and a mask to
	 * remove excess logic data content before datafeed submission.
	 */
	devc->enabled_logic_channels = 0;
	devc->enabled_analog_channels = 0;
	for (l = sdi->channels; l; l = l->next) {
		ch = l->data;
		if (!ch->enabled)
			continue;
		if (ch->type == SR_CHANNEL_ANALOG) {
			devc->enabled_analog_channels++;
			continue;
		}
		if (ch->type != SR_CHANNEL_LOGIC)
			continue;
		/*
		 * TODO: Need we create a channel map here, such that the
		 * session datafeed packets will have a dense representation
		 * of the enabled channels' data? For example store channels
		 * D3 and D5 in bit positions 0 and 1 respectively, when all
		 * other channels are disabled? The current implementation
		 * generates a sparse layout, might provide data for logic
		 * channels that are disabled while it might suppress data
		 * from enabled channels at the same time.
		 */
		devc->enabled_logic_channels++;
	}
	devc->first_partial_logic_index = devc->enabled_logic_channels / 8;
	bitpos = devc->enabled_logic_channels % 8;
	mask = (1 << bitpos) - 1;
	devc->first_partial_logic_mask = mask;
	sr_dbg("num logic %zu, partial off %zu, mask 0x%02x.",
		devc->enabled_logic_channels,
		devc->first_partial_logic_index,
		devc->first_partial_logic_mask);

	devc->first_packet_host_time = -1;
	// HACK
	last_timestamp = UINT64_MAX;

	sr_session_source_add(sdi->session, -1, 0, 100,
			xmos_prepare_data, (struct sr_dev_inst *)sdi);

	std_session_send_df_header(sdi);
#if 0
	if (devc->limit_frames > 0)
		std_session_send_frame_begin(sdi);
#endif	
	/* We use this timestamp to decide how many more samples to send. */
	devc->start_us = g_get_monotonic_time();
	devc->spent_us = 0;
	devc->step = 0;



    // Add pseudo channels
    add_analogue_channel(254, "Time delta");
    add_analogue_channel(255, "Missing Data");
#if 0
unsigned char cmd = XMOS_CONNECT;
devc->xmos_tcp_ops->send(devc, &cmd, 1);
#endif

// HACK
xmos_acquisition_start();

	const unsigned long sammple_period_ps = UINT64_C(1000000000000)/ devc->cur_samplerate;

fprintf(stderr, "sammple_period_ps: %lu\n", sammple_period_ps);


fprintf(stderr, "<<<< xmos: dev_acquisition_start()\n");

	return SR_OK;
}

static int dev_acquisition_stop(struct sr_dev_inst *sdi)
{
	struct dev_context *devc;

fprintf(stderr, ">>>> xmos: dev_acquisition_stop()\n");

	sr_session_source_remove(sdi->session, -1);

	devc = sdi->priv;
#if 0
	if (devc->limit_frames > 0)
		std_session_send_frame_end(sdi);
#endif
	std_session_send_df_end(sdi);

	if (devc->stl) {
		soft_trigger_logic_free(devc->stl);
		devc->stl = NULL;
	}

//unsigned char cmd = XMOS_TERMINATE;
//devc->xmos_tcp_ops->send(devc, &cmd, 1);

fprintf(stderr, "<<<< xmos: dev_acquisition_stop()\n");
	return SR_OK;
}

static struct sr_dev_driver xmos_driver_info = {
	.name = "XMOS_xSCOPE",
	.longname = "XMOS xSCOPE virtual oscilloscope",
	.api_version = 1,
	.init = std_init,
	.cleanup = std_cleanup,
	.scan = scan,
	.dev_list = std_dev_list,
	.dev_clear = dev_clear,
	.config_get = config_get,
	.config_set = config_set,
	.config_list = config_list,
	.dev_open = dev_open,
	.dev_close = dev_close,
	.dev_acquisition_start = dev_acquisition_start,
	.dev_acquisition_stop = dev_acquisition_stop,
	.context = NULL,
};
SR_REGISTER_DEV_DRIVER(xmos_driver_info);

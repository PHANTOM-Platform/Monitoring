/*
* Copyright (C) 2018 University of Stuttgart
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iio.h>
#include "mf_Board_power_connector.h"

#define SUCCESS 0
#define FAILURE 1
#define MAX_DEVICES 8
#define MAX_CHANNELS 5
#define SAMPLES_PER_READ 16

/*******************************************************************************
* Variable Declarations
******************************************************************************/
struct my_channel {
	char label[128];
	char unit[128];
	double scale;
	double value;		/* average value for one buffer */
	const struct iio_channel *iio;
};

static struct iio_context *ctx;
struct iio_device **devices;
struct iio_buffer **buffer;
static long long sampling_freq[MAX_DEVICES];
static unsigned int nb_devices;
static unsigned int nb_channels;
static long long nb_samples[MAX_DEVICES];
static struct my_channel my_chn[MAX_DEVICES][MAX_CHANNELS];

/*******************************************************************************
* Forward Declarations
******************************************************************************/
int mf_acme_is_enabled(char *acme_name);
static void init_ina2xx_channels(struct iio_device *dev, int device_idx);
int create_EventSets(Plugin_metrics *data, char **events, size_t num_events);
static ssize_t print_sample(const struct iio_channel *chn, void *buf, size_t len, void *d);
static inline int chan_device(const struct iio_channel *chn);
static inline int chan_index(const struct iio_channel *chn, int device_idx);
void filter(Plugin_metrics *data);
void reset_my_channels_value();

/*******************************************************************************
* Functions implementation
******************************************************************************/
/** Initializes the Board_power plugin
*  
*  Checks if the ACME component is available through hostname "baylibre-acme.local",
*  if ACME component is available, initialize all channels for all 
*  devices; assign channel labels to data->events.
*  @return 1 on success; 0 otherwise.
*/
int mf_Board_power_init(Plugin_metrics *data, char **events, size_t num_events, char *acme_name)
{
	if(!mf_acme_is_enabled(acme_name))
		return FAILURE;
	if(!create_EventSets(data, events, num_events))
		return FAILURE;
	return SUCCESS;
}

/** @brief Samples all possible events and stores data into the Plugin_metrics
*
*  @return 1 on success; 0 otherwise.
*/
int mf_Board_power_sample(Plugin_metrics *data){
	int device_idx;
	/* for each device refill the buffer and reads all samples */
	for(device_idx = 0; device_idx < nb_devices; device_idx++) {
		int ret = iio_buffer_refill(buffer[device_idx]);
		if (ret < 0) {
			fprintf(stderr, "Unable to refill buffer: %s\n", strerror(-ret));
			return FAILURE;
		}
		iio_buffer_foreach_sample(buffer[device_idx], print_sample, NULL);
	}
	/* filters the user specified data */
	filter(data);
	/* my_chn values of all channels are reset to zeros */
	reset_my_channels_value();
	return SUCCESS;
}


/** @brief Formats the sampling data into a json string
*
*  json string contains: plugin name, timestamps, metrics_name and metrics_value
*
*/
void mf_Board_power_to_json(Plugin_metrics *data, char **events, size_t num_events, char *json)
{
	struct timespec timestamp;
	char tmp[128] = {'\0'};
	int i, ii;
	/*
	* prepares the json string, including current timestamp, and name of the plugin
	*/
	sprintf(json, "\"type\":\"Board_power\"");
	clock_gettime(CLOCK_REALTIME, &timestamp);
	double ts = timestamp.tv_sec * 1.0e3 + (double)(timestamp.tv_nsec / 1.0e6); // in millisecond
	sprintf(tmp, ",\"local_timestamp\":\"%.1f\"", ts);
	strcat(json, tmp);
	/*
	* filters the sampled data with respect to metrics given
	*/
	for (i = 0; i < num_events; i++) {
		for(ii = 0; ii < data->num_events; ii++) {
			/* if metrics' name matches, append the metrics to the json string */
			if(strcmp(events[i], data->events[ii]) == 0) {
				sprintf(tmp, ",\"%s\":%.3f", data->events[ii], data->values[ii]);
				strcat(json, tmp);
			}
		}
	}
}

/** @brief Stops the plugin
*
*  This methods shuts down gracefully for sampling.
*/
void mf_Board_power_shutdown()
{
	int i;
	for (i=0; i<nb_devices; i++) {
		if(buffer[i])
			iio_buffer_destroy(buffer[i]);
	}
	free(buffer);
	free(devices);
	if(ctx)
		iio_context_destroy(ctx);
}

/* Checks if the ACME component is available through hostname "baylibre-acme.local";
initializes variable "my_chn" for all possible devices and channels */
int mf_acme_is_enabled(char *acme_name)
{
	int c, device_idx;
	char temp[1024];
	unsigned int buffer_size = SAMPLES_PER_READ;
	/* create network link with the acme board with given acme board hostname */
	ctx = iio_create_network_context(acme_name);
	if (!ctx) {
		fprintf(stderr, "Unable to create IIO context (network link with the acme board)\n");
		return FAILURE;
	}

	/* count the number of devices <= 8 */
	nb_devices = iio_context_get_devices_count(ctx);
	if(nb_devices > MAX_DEVICES) {
		fprintf(stderr, "Too many devices than maximum setting\n");
		return FAILURE;	
	}

	/* allocate devices */
	devices = (struct iio_device **)malloc(nb_devices * sizeof(struct iio_device *));

	/* allocate different buffers for different devices */
	buffer = (struct iio_buffer **)malloc(nb_devices * sizeof(struct iio_buffer *));

	/* for each device, prepare sampling of all channels */
	for(device_idx = 0; device_idx < nb_devices; device_idx++) {
		devices[device_idx] = iio_context_get_device(ctx, device_idx);
		if(!devices[device_idx]) {
			iio_context_destroy(ctx);
			fprintf(stderr, "Device %d is not enabled.\n", device_idx);
			return FAILURE;
		}

		if(iio_device_attr_write(devices[device_idx], "in_oversampling_ratio", "4") <= 0) {
			fprintf(stderr, "write ratio 4 failed.\n");
			return FAILURE;
		}

		memset(temp, '0', 1024);
		c = iio_device_attr_read(devices[device_idx], "in_sampling_frequency", temp, 1024);
		if (c) {
			sampling_freq[device_idx] = atoi(temp);
		} else
			return FAILURE;
		
		/* initialize my_chn variables for sampling */
		init_ina2xx_channels(devices[device_idx], device_idx);

		buffer[device_idx] = iio_device_create_buffer(devices[device_idx], buffer_size, false);

		if (!buffer[device_idx]) {
			fprintf(stderr, "Unable to allocate buffer\n");
			iio_context_destroy(ctx);
			return FAILURE;
		}
	}
	return SUCCESS;
}

/* Initialze the my_chn variable for each device, reads for each channel the scale, units, label,
and all channels found are enabled. */
static void init_ina2xx_channels(struct iio_device *dev, int device_idx)
{
	int i;
	char buf[1024];
	struct iio_channel *ch;

	if (strcmp(iio_device_get_name(dev), "ina226")) {
		fprintf(stderr, "Unknown device %s\n", iio_device_get_name(dev));
		exit(-1);
	}
	nb_samples[device_idx] = 0;
	nb_channels = iio_device_get_channels_count(dev);

	/* FIXME: dyn alloc */
	if(nb_channels > MAX_CHANNELS){
		fprintf(stderr, "Too many channels.\n");
		exit(-1);
	}
	for (i = 0; i < nb_channels; i++) {
		const char *id;
		ch = iio_device_get_channel(dev, i);
		my_chn[device_idx][i].value = 0.0;
		my_chn[device_idx][i].iio = ch;
		id = iio_channel_get_id(ch);
		if (iio_channel_attr_read(ch, "scale", buf, sizeof(buf)) >= 0)
			my_chn[device_idx][i].scale = atof(buf);
		else
			my_chn[device_idx][i].scale = 1.0;
		if (!strncmp(id, "power", 5)) {
			sprintf(my_chn[device_idx][i].label,"device%d:power",device_idx);
			strcpy(my_chn[device_idx][i].unit, "mW");
		} else if (!strncmp(id, "current", 6)) {
			sprintf(my_chn[device_idx][i].label,"device%d:current",device_idx);
			strcpy(my_chn[device_idx][i].unit, "mA");
		} else if (!strncmp(id, "voltage1", 8)) {
			sprintf(my_chn[device_idx][i].label,"device%d:vbus",device_idx);
			strcpy(my_chn[device_idx][i].unit, "mV");
		} else if (!strncmp(id, "voltage0", 8)) {
			sprintf(my_chn[device_idx][i].label,"device%d:vshunt",device_idx);
			strcpy(my_chn[device_idx][i].unit, "mV");
		}
		iio_channel_enable(ch);
	}
}

/* According to the user's configuration and channels available, create a EventSet */
int create_EventSets(Plugin_metrics *data, char **events, size_t num_events)
{
	int i, device_idx, channel_idx;
	int idx = 0;
	for (i = 0; i < num_events; i++) {
		for(device_idx = 0; device_idx < nb_devices; device_idx++) {
			for (channel_idx = 0; channel_idx < nb_channels; channel_idx++) {
				if(!strcmp(my_chn[device_idx][channel_idx].label, events[i])) {
					/*input event is found*/
					data->events[idx] = malloc(MAX_EVENTS_LEN * sizeof(char));
					strcpy(data->events[idx], events[i]);
					idx++;
					break;
				}
			}
			if(channel_idx < nb_channels)
				break;
		}
		if(device_idx >= nb_devices) {
			fprintf(stderr, "Event %s is not found.\n", events[i]);
			return FAILURE;
		}
	}
	
	data->num_events = idx;
	return SUCCESS;
}

/* callback function for each sample inside a buffer, 
* values of the same channel are aggregated, and averaged overall one buffer */
static ssize_t print_sample(const struct iio_channel *chn, void *buf, size_t len, void *d)
{
	int val = (int)*(short *)buf;
	int vabs = 0;
	int i = chan_device(chn);	//i is device id
	int j = chan_index(chn, i);	//j is channel id
	/* increment the sample count on the first channel */
	if(!j)
		nb_samples[i]++;
	vabs =abs(val);
	my_chn[i][j].value += ((double)(vabs) - my_chn[i][j].value) / nb_samples[i];
	
	return len;
}

/* get the device index for a channel */
static inline int chan_device(const struct iio_channel *chn)
{
	int i;
	const struct iio_device *dev;
	char tmp[64];
	dev = iio_channel_get_device(chn);
	const char * device_id = iio_device_get_id(dev);
	for (i = 0; i < nb_devices; i++) {
		memset(tmp, '0', 64);
		sprintf(tmp, "iio:device%d", i);
		if(!strcmp(device_id, tmp)) {
			return i;
		}
	}
	return -1;
}

/* get the channel index for a channel */
static inline int chan_index(const struct iio_channel *chn, int device_idx)
{
	int i;

	for (i = 0; i < nb_channels; i++)
		if (my_chn[device_idx][i].iio == chn)
			return i;
	return -1;
}

/* filter out the user specifed data, store the values in Plugin_metrics data */
void filter(Plugin_metrics *data)
{
	int idx, device_idx, channel_idx;
	for (idx = 0; idx < data->num_events; idx++) {
			for (device_idx = 0; device_idx < nb_devices; device_idx++) {
				for (channel_idx = 0; channel_idx < nb_channels; channel_idx++) {
					if(!strcmp(data->events[idx], my_chn[device_idx][channel_idx].label)) {
						data->values[idx] = 
							my_chn[device_idx][channel_idx].value * my_chn[device_idx][channel_idx].scale;
						break;
					}
				}
				if(channel_idx < nb_channels)
					break;
			}
			if(device_idx >= nb_devices) {
				fprintf(stderr, "event %s is not sampled\n", data->events[idx]);
			}	
		}
}

/* my_chn values are reset to zero, this function is called after all buffers are refilled, and samples are filtered */
void reset_my_channels_value()
{
	int device_idx, channel_idx;
	for (device_idx = 0; device_idx < nb_devices; device_idx++) {
		nb_samples[device_idx] = 0;
		for (channel_idx = 0; channel_idx < nb_channels; channel_idx++) {
			my_chn[device_idx][channel_idx].value = 0.0;
		}
	}
}

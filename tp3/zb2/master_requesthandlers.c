#include "master_requesthandlers.h"
#include "zb_transport.h"
#include "zb_packets.h"
#include "diagnostics.h"
#include <pthread.h>
#include <time.h>

/* this implementation of the REQUEST functions is not thread-safe. only one thread should be calling them. */
struct sensor_result {
	int device_id;
	sensor_data_t data;
	time_t time;
	char is_valid;
	pthread_mutex_t lock;
};

struct sensor_config {
	int device_id;
	sensor_data_t offset;
	time_t calibrated;
};

static struct sensor_config sensor_configs[SENSOR_COUNT];
static struct sensor_result sensor_results[SENSOR_COUNT];

enum comms_state {STATE_IDLE, STATE_PENDING_MEASURE, STATE_PENDING_CALIBRATE};


static enum comms_state state;

int busy() {
	/* TODO implement time-out. */
	return !(state == STATE_IDLE);
}

void sensors_init() {
	int i;
	
	state = STATE_IDLE;

	for (i = 0; i < SENSOR_COUNT; i++) {
		pthread_mutex_init(&sensor_results[i].lock, NULL);
		pthread_mutex_lock(&sensor_results[i].lock);
		sensor_results[i].device_id = i;
		sensor_results[i].data = 0;
		sensor_results[i].time = 0;
		sensor_results[i].is_valid = 0;
		pthread_mutex_unlock(&sensor_results[i].lock);
		
		sensor_configs[i].device_id = i;
		sensor_configs[i].offset = 0; /* TODO get from config file or something? */
		sensor_configs[i].calibrated = 0;
	}
}


void REQUEST_measure(char *buf) {
	if (!busy()) {
		DIAGNOSTICS("MEASURE: sending broadcast message to get measurements\n");
		state = STATE_PENDING_MEASURE;
		zb_send_packet(OP_MEASURE_REQUEST, NULL, 0);
	} else {
		DIAGNOSTICS("MEASURE:  request not honoured as the system is currently busy.\n");
	}
}

void REQUEST_calibrate(char *buf) {
	if (!busy()) {
		DIAGNOSTICS("CALIBRATE: sending broadcast message to get raw measurements\n");
		state = STATE_PENDING_CALIBRATE;
		zb_send_packet(OP_MEASURE_REQUEST, NULL, 0);
	} else {
		DIAGNOSTICS("CALIBRATE: request not honoured as the system is currently busy.\n");
	}
}

void REQUEST_ping(char *buf) {
	if (!busy()) {
		DIAGNOSTICS("PING sent.\n");
		state = STATE_PENDING_CALIBRATE;
		zb_send_packet(OP_PING, NULL, 0);
	} else {
		DIAGNOSTICS("PING request not honoured as the system is currently busy.\n");
	}
}

void REQUEST_data(char *buf) {
	int i;
	DIAGNOSTICS("DATA: Returning current sensor data.\n");

	for (i = 0; i < SENSOR_COUNT; i++) {
		pthread_mutex_lock(&sensor_results[i].lock);
		DIAGNOSTICS("DATA: Sensor %d: Raw %d, Offset %d, Corrected %d. Last read at %d\n",
				i,
				sensor_results[i].data,
				sensor_configs[i].offset,
				sensor_results[i].data - sensor_configs[i].offset,
				sensor_results[i].time);
		pthread_mutex_unlock(&sensor_results[i].lock);
	}
}

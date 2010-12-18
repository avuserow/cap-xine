#include <stdio.h>
#include <unistd.h>

#include <xine.h>

#define SLEEP_TIME 250
#define DEBUG 0

int main (int argc, char* argv[]) {
	// basic xine initialization
	xine_t *engine;
	engine = xine_new();
	xine_init(engine);

	// some audio output
	xine_audio_port_t *ap;
	ap = xine_open_audio_driver(engine, NULL, NULL);

	// null video output
	xine_video_port_t *vp;
	vp = xine_open_video_driver(engine, NULL, XINE_VISUAL_TYPE_NONE, NULL);

	xine_stream_t *stream;
	stream = xine_stream_new(engine, ap, vp);

	xine_open(stream, argv[1]);
	xine_play(stream, 0, 0);

	int pos_stream, pos_time, length_time;
	xine_get_pos_length(stream, &pos_stream, &pos_time, &length_time);
	printf("length: %d\n", length_time);
	int status = xine_get_status(stream);

	// the stopped status is the primary thing to check here, since some files
	// report being longer than they are
	while (pos_time <= length_time && status != XINE_STATUS_STOP) {
		status = xine_get_status(stream);

		xine_get_pos_length(stream, &pos_stream, &pos_time, &length_time);
		if (DEBUG) printf("length (%d), pos(%d) ", length_time, pos_time);
		if (SLEEP_TIME < length_time - pos_time) {
			if (DEBUG) printf("... normal\n");
			usleep(SLEEP_TIME);
		} else {
			if (DEBUG) printf("... short\n");
			usleep(length_time - pos_time);
		}
	}

	// Stop playing and close down the stream 
	xine_close(stream);

	// Now shut down cleanly
	xine_close_audio_driver(engine, ap);
	xine_close_video_driver(engine, vp);
	xine_exit(engine);
	return 0;
}

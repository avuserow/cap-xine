#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>

#include <xine.h>

#define SLEEP_TIME 250
#define DEBUG 0

#define NB_ENABLE 1
#define NB_DISABLE 0

// thanks to http://cc.byexamples.com/2007/04/08/non-blocking-user-input-in-loop-without-ncurses/
int kbhit() {
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
	select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &fds);
}

void nonblock(int state) {
	struct termios ttystate;

	//get the terminal state
	tcgetattr(STDIN_FILENO, &ttystate);

	if (state==NB_ENABLE) {
		//turn off canonical mode
		ttystate.c_lflag &= ~ICANON;
		//minimum of number input read.
		ttystate.c_cc[VMIN] = 1;
	} else if (state==NB_DISABLE) {
		//turn on canonical mode
		ttystate.c_lflag |= ICANON;
	}
	//set the terminal attributes.
	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

char* fetchline() {
	unsigned int size = 1;
	unsigned int used = 0;
	char* line = malloc(size);

	char c = '?';
	do {
		c = getchar();
		if (c != '\n') line[used++] = c;
		if (used == size) {
			size *= 2;
			line = realloc(line, size);
		}
	} while (c != '\n');
	line[used++] = '\0';

	return line;
}

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

	//nonblock(NB_ENABLE);

	unsigned int volume = xine_get_param(stream, XINE_PARAM_AUDIO_AMP_LEVEL);
	printf("volume is %d\n", volume);

	// the stopped status is the primary thing to check here, since some files
	// report being longer than they are
	while (pos_time <= length_time && status != XINE_STATUS_STOP) {
		if (kbhit()) {
			char* line = fetchline();
			printf("got a %s\n", line);

			if (strncmp("vol", line, 3) == 0) {
				int vol;
				sscanf(line, "vol %d\n", &vol);
				if (vol > 200) vol = 200;
				if (vol < 0) vol = 0;
				printf("changing volume to %d\n", vol);
				xine_set_param(stream, XINE_PARAM_AUDIO_AMP_LEVEL, vol);
				volume = xine_get_param(stream, XINE_PARAM_AUDIO_AMP_LEVEL);
			} else if (strncmp("skip", line, 4) == 0) {
				char* newsong = line + 5;
				printf("switching to song '%s'\n", newsong);
				xine_close(stream);
				xine_open(stream, newsong);
				xine_play(stream, 0, 0);
			} else if (strncmp("quit", line, 4) == 0) {
				printf("exiting...\n");
				free(line);
				break;
			} else {
				printf("unknown command: '%s'\n", line);
			}

			free(line);
		}

		xine_get_pos_length(stream, &pos_stream, &pos_time, &length_time);
		if (DEBUG) printf("length (%d), pos(%d) ", length_time, pos_time);
		if (SLEEP_TIME < length_time - pos_time) {
			if (DEBUG) printf("... normal\n");
			usleep(SLEEP_TIME);
		} else {
			if (DEBUG) printf("... short\n");
			usleep(length_time - pos_time);
		}
		status = xine_get_status(stream);
	}

	nonblock(NB_DISABLE);

	// Stop playing and close down the stream 
	xine_close(stream);

	// Now shut down cleanly
	xine_close_audio_driver(engine, ap);
	xine_close_video_driver(engine, vp);
	xine_exit(engine);
	return 0;
}

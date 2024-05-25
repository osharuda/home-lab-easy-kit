#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include "hlekio_ioctl.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

struct test_options {
	int non_block;
	unsigned long bin_mode;
	int monitor;
	unsigned long monitor_delay;
	unsigned long debounce;
	int reset;
};

int check_params(int argc, const char* argv[], struct test_options* opts) {
	const int min_args = 2;
	const int last_arg = argc - 1;
	const char* s_non_block = "--monitor=";
	const int s_non_block_len = strlen(s_non_block);
	const char* s_debounce = "--debounce=";
	const int s_debounce_len = strlen(s_debounce);

	char* endptr;
	if (argc < min_args) {
		return -1;
	}

	for (int i=1; i<argc-1; i++) {
		if (strcmp(argv[i], "--non-block")==0) {
			opts->non_block = 1;
		} else if (strcmp(argv[i], "--bin-mode")==0) {
			opts->bin_mode = 1;
		} else if (strcmp(argv[i], "--reset")==0) {
			opts->reset = 1;
		} else if (strncmp(argv[i], s_non_block, s_non_block_len)==0) {
			opts->monitor = 1;
			const char* valptr = argv[i]+s_non_block_len;
			opts->monitor_delay = strtoul(valptr, &endptr, 10);
			if (*valptr==0 || *endptr!=0) {
				printf("*** Invalid monitor value.");
				return -1;
			}
		} else if (strncmp(argv[i], s_debounce, s_debounce_len)==0) {
			const char* valptr = argv[i]+s_debounce_len;
			opts->debounce = strtoul(valptr, &endptr, 10);
			if (*valptr==0 || *endptr!=0) {
				printf("*** Invalid debounce value.");
				return -1;
			}
		}
	}

	return 0;
}

void help() {
	printf("Usage:\n\
test.c [opts] <device>\n\
Options:\n\
--non-block    - Instructs use non-blocking IO (by default blocking IO is used).\n\
--bin-mode     - Instructs to set up binary mode.\n\
--monitor=val  - Runs in cycle until killed by Ctrl-C.\n\
                 If non-blocking IO is used, a delay of 1 second is inserted\n\
                 between loop cycles.\n\
                 val - number of milliseconds for pause.\n\
--debounce=val - Set debounce value in number of system timer tickets (__jiffees).\n\
--reset        - Reset device.\n\
");
}

int main(int argc, const char* argv[]) {
	struct test_options opts = {
		.non_block = 0,
		.bin_mode  = 0,
		.monitor   = 0,
		.debounce  = 0,
		.reset     = 0
	};

	char* buffer[1024];
	static_assert(sizeof(buffer) >= sizeof(struct hlekio_input_info));
	int fd = -1;
	int flags = O_CLOEXEC;
	unsigned long bin_mode = 0;

	int res = -1;
	if (check_params(argc, argv, &opts)) {
		help();
		goto done;
	}

	// Open file
	if (opts.non_block) flags |= O_NONBLOCK;
	fd = open(argv[argc-1], flags);
	if (fd <= 0) {
		res = errno;
		printf("*** open failed, res = %i\n", res);
		goto done;
	}

	res = ioctl(fd, HLEKIO_BINARY_MODE, opts.bin_mode);
	if (res < 0) {
		printf("*** ioctl(binary mode) failed, res = %i\n", res);
		goto done;
	}

	res = ioctl(fd, HLEKIO_DEBOUNCE, opts.debounce);
	if (res < 0) {
		printf("*** ioctl(debounce) failed, res = %i\n", res);
		goto done;
	}

	if (opts.reset) {
		res = ioctl(fd, HLEKIO_RESET);
	}

	if (res < 0) {
		printf("*** ioctl(reset) failed, res = %i\n", res);
		goto done;
	}

	do {
		res = read(fd, buffer, sizeof(buffer));

		if (res <= 0) {
			printf("*** read result is %i\n", res);
			goto done;
		}

		if (opts.bin_mode) {
			assert(res==sizeof(struct hlekio_input_info));
			struct hlekio_input_info * in_info = (struct hlekio_input_info *)buffer;
			printf("[BIN] %llu,%llu,%lu,%lu,%d\n",
            	in_info->last_isr_jiffers,
            	in_info->reset_jiffers,
            	in_info->isr_count,
            	in_info->isr_debounce,
            	in_info->level);
		} else {
			printf("[TXT] %s\n", buffer);
		}

		if (opts.monitor) {
			// seek
			res = lseek(fd, SEEK_SET, 0);
			if (res != 0) {
				printf("*** lseek result is %i\n", res);
				goto done;
			}

			if (opts.monitor_delay) {
				usleep(opts.monitor_delay*1000);
			}
		}
	} while ( opts.monitor );

done:
	if (fd) {
		close(fd);
	}

	return res;
}
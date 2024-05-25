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
	int set;
	unsigned long set_value;
	int bin_mode;
	int reset;
};

int check_params(int argc, const char* argv[], struct test_options* opts) {
	const int min_args = 2;
	const int last_arg = argc - 1;
	const char* s_set = "--set=";
	const int s_set_len = strlen(s_set);

	char* endptr;
	if (argc < min_args) {
		return -1;
	}

	for (int i=1; i<argc-1; i++) {
		if (strcmp(argv[i], "--bin-mode")==0) {
			opts->bin_mode = 1;
		} else if (strcmp(argv[i], "--reset")==0) {
			opts->reset = 1;
		} else if (strncmp(argv[i], s_set, s_set_len)==0) {
			const char* valptr = argv[i]+s_set_len;
			opts->set = 1;
			opts->set_value = strtoul(valptr, &endptr, 10);
			if (*valptr==0 || *endptr!=0 || opts->set_value>1) {
				printf("*** Invalid set value.");
				return -1;
			}
		}
	}

	return 0;
}

void help() {
	printf("Usage:\n\
testout [opts] <device>\n\
Options:\n\
--bin-mode - Instructs to set up binary mode.\n\
--reset    - Reset device.\n\
--set=val  - Set pin value: 0 - low, 1 - high. \n\
             If this option is not used, current level is printed.\n\
");
}

int main(int argc, const char* argv[]) {
	struct test_options opts = {
		.bin_mode  = 0,
		.reset     = 0,
		.set       = 0,
		.set_value = 0
	};

	char* buffer[1024];
	static_assert(sizeof(buffer) >= sizeof(struct hlekio_input_info));
	int fd = -1;
	int flags = O_CLOEXEC | O_RDWR;
	unsigned long bin_mode = 0;
	uint8_t level;

	int res = -1;
	if (check_params(argc, argv, &opts)) {
		help();
		goto done;
	}

	// Open file
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

	if (opts.reset) {
		res = ioctl(fd, HLEKIO_RESET);
	}

	if (res < 0) {
		printf("*** ioctl(reset) failed, res = %i\n", res);
		goto done;
	}

	if (opts.set) {
		level = !!(opts.set_value);
		if (!opts.bin_mode) {
			level = level ? '1' : '0';
		}

		res = write(fd, &level, 1);
		if (res < 1) {
			printf("*** write failed, errno=%i\n", errno);
			goto done;
		}
	} else {
		res = read(fd, &level, 1);
		if (res < 1) {
			printf("*** write read, res is %i\n", res);
			goto done;
		}

		if (!opts.bin_mode) {
			printf("[TXT] %c\n", level);
		} else {
			printf("[BIN] %d\n", (unsigned int)level);
		}
	}

done:
	if (fd) {
		close(fd);
	}

	return res;
}
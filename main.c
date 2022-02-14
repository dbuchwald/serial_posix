#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"

static int parse_command_line_params(int argc, char * args[], serial_parameters* params);

int main(int argc, char * args[]) {
    int fd;
    serial_parameters params;
    params.clear_flags = 0;
    params.no_ctty = 1;
    params.no_delay = 1;
    params.bitsmode = BITS_8;
    params.paritymode = PARITY_EVEN;
    params.stopmode = STOP_TWO;

    if (!parse_command_line_params(argc, args, &params)) {
        fprintf(stderr, "Unable to parse input parameters, aborting...\n");
        return 1;
    }

    if (!serial_open(args[1], params, &fd)) {
        fprintf(stderr, "Serial port open failed, aborting...\n");
        return 1;
    }

    serial_close(fd);
    return 0;
}

static int parse_command_line_params(int argc, char * args[], serial_parameters* params) {

    if (argc < 3) {
        fprintf(stderr, "%s: Usage %s PORT_NAME BAUD_RATE <CLEAR_FLAGS> <CTTY> <DELAY>\n", args[0], args[0]);
        return 0;
    }

    params->baud_rate = atol(args[2]);

    if (params->baud_rate == 0) {
        fprintf(stderr, "Unable to parse [%s] as baud rate\n", args[2]);
        return 0;
    }

    for (int i=3; i<argc; i++) {
        if (!strcasecmp(args[i], "CLEAR_FLAGS")) {
            params->clear_flags = 1;
        }
        if (!strcasecmp(args[i], "CTTY")) {
            params->no_ctty = 0;
        }
        if (!strcasecmp(args[i], "DELAY")) {
            params->no_delay = 0;
        }
    }
    return 1;
}
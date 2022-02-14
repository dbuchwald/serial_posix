#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>

#include "serial.h"

static speed_t convert_baud_rate(long baud) {
    switch (baud) {
        case 300:
            return B300;
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        default:
            return B0;
    }
}

void log_termios(struct termios options);

int serial_open(char * port, serial_parameters params, int * fd) {

    struct termios options;
    int rc;

    fprintf(stdout, "Opening port [%s] with the following parameters:", port);
    fprintf(stdout, " O_RDWR [enabled]");
    fprintf(stdout, " O_NOCTTY [%s]", params.no_ctty ? "enabled" : "disabled");
    fprintf(stdout, " O_NONBLOCK [%s]", params.no_delay ? "enabled" : "disabled");
    fprintf(stdout, "\n");
    
    *fd = open(port, O_RDWR | 
                     params.no_ctty  ? O_NOCTTY   : 0 | 
                     params.no_delay ? O_NONBLOCK : 0);

    if (*fd == -1) {
        fprintf(stderr, "Unable to open port [%s]\n", port);
        return 0;
    } else {
        fcntl(*fd, F_SETFL, 0);
    }

    speed_t baud_rate = convert_baud_rate(params.baud_rate);

    if (baud_rate == B0) {
        fprintf(stderr, "Baud rate [%ld] not supported\n", params.baud_rate);
        return 0;
    }

    rc = tcgetattr(*fd, &options);
    if (rc < 0) {
        fprintf(stderr, "Unable to load attributes, error code: [%d]\n", rc);
        return 0;
    }

    fprintf(stdout, "Initial state of termios structure:\n");
    log_termios(options);

    if (params.clear_flags) {
        options.c_iflag = 0;
        options.c_oflag = 0;
        options.c_lflag = 0;
        options.c_cflag = 0;
        fprintf(stdout, "Termios structure after clear:\n");
        log_termios(options);
    }

    options.c_iflag &= ~(INLCR | IGNCR | ICRNL | IGNBRK);

#ifdef IUCLC
    options.c_iflag &= ~IUCLC;
#endif /* IUCLC */

#ifdef PARMRK
    options.c_iflag &= ~PARMRK;
#endif /* IUCLC */

#ifdef IXANY
    options.c_iflag &= ~IXANY;
#endif /* IXANY */
    options.c_iflag &= ~(IXON | IXOFF);

#ifdef CRTSCTS
    options.c_cflag &= ~(CRTSCTS);
#endif /* CRTSCTS */

#ifdef CNEW_RTSCTS
    options.c_cflag &= ~(CNEW_RTSCTS);
#endif /* CNEW_RTSCTS */


    options.c_cflag |= CREAD | CLOCAL;

    switch(params.paritymode) {
        case PARITY_NONE:
            options.c_cflag &= ~(PARENB | PARODD);
            break;
        case PARITY_SPACE:
        case PARITY_EVEN:
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            break;
        case PARITY_MARK:
        case PARITY_ODD:
            options.c_cflag |= (PARENB | PARODD);
            break;
    }

    switch(params.stopmode) {
        case STOP_ONE:
            options.c_cflag &= ~CSTOPB;
            break;
        case STOP_ONEANDHALF:
        case STOP_TWO:
            options.c_cflag |= CSTOPB;
            break;
    }

    options.c_cflag &= ~CSIZE;
    switch(params.bitsmode) {
        case BITS_5:
            options.c_cflag |= CS5;
            break;
        case BITS_6:
            options.c_cflag |= CS6;
            break;
        case BITS_7:
            options.c_cflag |= CS7;
            break;
        case BITS_8:
            options.c_cflag |= CS8;
            break;
    }

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG | IEXTEN);

#ifdef ECHOCTL
    options.c_lflag &= ~ECHOCTL;
#endif /* ECHOCTL */

#ifdef ECHOKE
    options.c_lflag &= ~ECHOKE;
#endif /* ECHOKE */

    options.c_oflag &= ~(OPOST | ONLCR | OCRNL);

    // probably ignored in the NDELAY mode (?)
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 0;

    cfsetispeed(&options, baud_rate);
    cfsetospeed(&options, baud_rate);

    fprintf(stdout, "Final state of termios structure:\n");
    log_termios(options);

    rc = tcsetattr(*fd, TCSANOW, &options);
    if (rc < 0) {
        fprintf(stderr, "Unable to set attributes, error code: [%d]\n", rc);
        return 0;
    }

/*
    rc = fcntl(*fd, F_GETFL);
    fprintf(stdout, "Received [%d] from F_GETFL\n", rc);
    if (rc != -1)
        fcntl(*fd, F_SETFL, rc & ~O_NONBLOCK);
*/
    tcflush(*fd, TCIFLUSH);

    return 1;
}

void serial_close(int fd) {
    close(fd);
}

#define log_mask_state(VARIABLE, FIELD, MASK) \
    fprintf(stdout, "    " #FIELD ".[%-10s] = [%-9s]\n", #MASK, VARIABLE.FIELD & MASK ? "enabled" : "disabled") 

void log_termios(struct termios options) {
    fprintf(stdout, "Termios structure: {\n");
    fprintf(stdout, "  c_iflag: [%lu] [0x%04lX],\n", (unsigned long)options.c_iflag, (unsigned long)options.c_iflag);

    log_mask_state(options, c_iflag, IGNBRK);
    log_mask_state(options, c_iflag, BRKINT);
    log_mask_state(options, c_iflag, IGNPAR);
    log_mask_state(options, c_iflag, PARMRK);
    log_mask_state(options, c_iflag, INPCK);
    log_mask_state(options, c_iflag, ISTRIP);
    log_mask_state(options, c_iflag, INLCR);
    log_mask_state(options, c_iflag, IGNCR);
    log_mask_state(options, c_iflag, ICRNL);
    log_mask_state(options, c_iflag, IXON);
    log_mask_state(options, c_iflag, IXOFF);
    log_mask_state(options, c_iflag, IXANY);
    log_mask_state(options, c_iflag, IMAXBEL);
    log_mask_state(options, c_iflag, IUTF8);

    fprintf(stdout, "  c_oflag: [%lu] [0x%04lX],\n", (unsigned long)options.c_oflag, (unsigned long)options.c_oflag);

    log_mask_state(options, c_oflag, OPOST);
    log_mask_state(options, c_oflag, ONLCR);
#ifdef OXTABS
    log_mask_state(options, c_oflag, OXTABS);
#endif /* OXTABS */
#ifdef ONOEOT
    log_mask_state(options, c_oflag, ONOEOT);
#endif /* ONOEOT */
    log_mask_state(options, c_oflag, OCRNL);
    log_mask_state(options, c_oflag, ONOCR);
    log_mask_state(options, c_oflag, ONLRET);
    log_mask_state(options, c_oflag, OFILL);
    log_mask_state(options, c_oflag, NLDLY);
    log_mask_state(options, c_oflag, TABDLY);
    log_mask_state(options, c_oflag, CRDLY);
    log_mask_state(options, c_oflag, FFDLY);
    log_mask_state(options, c_oflag, BSDLY);
    log_mask_state(options, c_oflag, VTDLY);
    log_mask_state(options, c_oflag, OFDEL);

    fprintf(stdout, "  c_lflag: [%lu] [0x%04lX],\n", (unsigned long)options.c_lflag, (unsigned long)options.c_lflag);

    log_mask_state(options, c_lflag, ECHOKE);
    log_mask_state(options, c_lflag, ECHOE);
    log_mask_state(options, c_lflag, ECHOK);
    log_mask_state(options, c_lflag, ECHO);
    log_mask_state(options, c_lflag, ECHONL);
    log_mask_state(options, c_lflag, ECHOPRT);
    log_mask_state(options, c_lflag, ECHOCTL);
    log_mask_state(options, c_lflag, ISIG);
    log_mask_state(options, c_lflag, ICANON);
#ifdef ALTWERASE
    log_mask_state(options, c_lflag, ALTWERASE);
#endif /* ALTWERASE */
    log_mask_state(options, c_lflag, IEXTEN);
    log_mask_state(options, c_lflag, EXTPROC);
    log_mask_state(options, c_lflag, TOSTOP);
    log_mask_state(options, c_lflag, FLUSHO);
#ifdef NOKERNINFO
    log_mask_state(options, c_lflag, NOKERNINFO);
#endif /* NOKERNINFO */
    log_mask_state(options, c_lflag, PENDIN);
    log_mask_state(options, c_lflag, NOFLSH);

    fprintf(stdout, "  c_cflag: [%lu] [0x%04lX],\n", (unsigned long)options.c_cflag, (unsigned long)options.c_cflag);

#ifdef CIGNORE
    log_mask_state(options, c_cflag, CIGNORE);
#endif /* CIGNORE */
    log_mask_state(options, c_cflag, CSIZE);
    log_mask_state(options, c_cflag, CS5);
    log_mask_state(options, c_cflag, CS6);
    log_mask_state(options, c_cflag, CS7);
    log_mask_state(options, c_cflag, CS8);
    log_mask_state(options, c_cflag, CSTOPB);
    log_mask_state(options, c_cflag, CREAD);
    log_mask_state(options, c_cflag, PARENB);
    log_mask_state(options, c_cflag, PARODD);
    log_mask_state(options, c_cflag, HUPCL);
    log_mask_state(options, c_cflag, CLOCAL);
#ifdef CCTS_OFLOW
    log_mask_state(options, c_cflag, CCTS_OFLOW);
#endif /* CCTS_OFLOW */
    log_mask_state(options, c_cflag, CRTSCTS);
#ifdef CRTS_IFLOW
    log_mask_state(options, c_cflag, CRTS_IFLOW);
#endif /* CRTS_IFLOW */
#ifdef CDTR_IFLOW
    log_mask_state(options, c_cflag, CDTR_IFLOW);
#endif /* CDTR_IFLOW */
#ifdef CDSR_OFLOW
    log_mask_state(options, c_cflag, CDSR_OFLOW);
#endif /* CDSR_OFLOW */
#ifdef CCAR_OFLOW
    log_mask_state(options, c_cflag, CCAR_OFLOW);
#endif /* CCAR_OFLOW */
#ifdef MDMBUF
    log_mask_state(options, c_cflag, MDMBUF);
#endif /* MDMBUF */

    fprintf(stdout, "  c_cc: {\n");
    for (int i=0; i<NCCS; i++) {
        fprintf(stdout, "    [%02d] = [0x%02X]", i, (unsigned char)options.c_cc[i]);
        if (i==NCCS-1) {
            fprintf(stdout, "}\n");
        } else {
            fprintf(stdout, ",\n");
        }
    }
    fprintf(stdout, "  c_ispeed: [%lu],\n", (unsigned long)options.c_ispeed);
    fprintf(stdout, "  c_ospeed: [%lu] }\n", (unsigned long)options.c_ospeed);

}
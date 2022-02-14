#ifndef __SERIAL_H_INCLUDED
#define __SERIAL_H_INCLUDED

typedef enum {
  PARITY_NONE,
  PARITY_EVEN,
  PARITY_ODD,
  PARITY_MARK,
  PARITY_SPACE
} parity_mode;

typedef enum {
  STOP_ONE,
  STOP_ONEANDHALF,
  STOP_TWO
} stop_mode;

typedef enum {
  BITS_5,
  BITS_6,
  BITS_7,
  BITS_8
} bits_mode;

typedef struct {
  long baud_rate;
  int  no_ctty;
  int  no_delay;
  int  clear_flags;
  parity_mode paritymode;
  stop_mode stopmode;
  bits_mode bitsmode;
} serial_parameters;

int serial_open(char * port, serial_parameters params, int * fd);
void serial_close(int fd);

#endif /* __SERIAL_H_INCLUDED */
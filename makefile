HEADERS= \
	serial.h

OBJECTS= \
	main.o \
	serial.o

TARGET=serialtest

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ 

%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@  

clean:
	$(RM) $(OBJECTS) $(TARGET)

CC=gcc
RM=rm -vrf
CFLAGS= -O2 -I ./ -I include/ -I drivers/ -DNCV_UNIT_TEST 

TARGET=ncv7718.out

OBJS=drivers/gpiodev/gpiodev.o \
	drivers/spibus/spibus.o \
	ncv7718.o

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) -lpthread -lm

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean

clean:
	$(RM) $(OBJS)
	$(RM) $(TARGET)
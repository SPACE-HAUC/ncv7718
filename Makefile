CC=gcc
RM=rm -vrf
CFLAGS= -O2 -I ./ -I include/ -I drivers/ -DNCV7718_UNIT_TEST -DNCV7718_DEBUG

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
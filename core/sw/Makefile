CFLAGS  = -std=gnu11 -Wall ## -Werror
CFLAGS += -I/opt/redpitaya/include
LDFLAGS = -L/opt/redpitaya/lib
LDLIBS = -static -lrp -lrp-hw-calib -lrp-hw-profiles

INCLUDE += -I/opt/redpitaya/include/api250-12
LDLIBS += -lrp-gpio -lrp-i2c
LDLIBS += -lrp-hw -lm -lstdc++ -lpthread -li2c #-lsocketcan


PRGS = digital_led_bar rf_loopback


all: $(PRGS)
	mkdir -p build
	mv $(PRGS) build

$(PRGS): %: %.c
	$(CC) $< $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -rf build

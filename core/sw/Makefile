CFLAGS  = -std=gnu11 -Wall ## -Werror
CFLAGS += -I/opt/redpitaya/include -Icontrol/inc -I/opt/redpitaya/include/api250-12
LDFLAGS = -L/opt/redpitaya/lib
LDLIBS = -static -lrp -lrp-hw-calib -lrp-hw-profiles -lrp-gpio -lrp-i2c -lrp-hw -lm -lstdc++ -lpthread -li2c #-lsocketc


PRGS = rf_write_test #digital_led_bar rf_loopback 

SRC_DIRS = control/src
OBJDIR = build/objects

SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJS = $(SRCS:%=$(OBJDIR)/%)
OBJS := $(OBJS:.c=.o)


all: $(PRGS)
	mkdir -p build
	mv $(PRGS) build



$(PRGS): %: %.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@


$(OBJDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@



$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o s@


clean:
	rm -rf build

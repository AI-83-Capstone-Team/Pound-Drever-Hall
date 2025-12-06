CFLAGS  := -std=gnu11 -Wall -Werror
CFLAGS += -I/opt/redpitaya/include -Icontrol/inc -Icontrol/hw_access/inc -I/opt/redpitaya/include/api250-12 -DDEBUG
LDFLAGS := -L/opt/redpitaya/lib
LDLIBS := -static -lrp -lrp-hw-calib -lrp-hw-profiles -lrp-gpio -lrp-i2c -lrp-hw -lm -lstdc++ -lpthread -li2c #-lsocketc

SRC_DIRS := control control/hw_access

OBJDIR := build/objects
ASMDIR := build/asm
BUILDDIR := build

SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
TESTS := scope_test lock_in_test rf_write_test rf_loopback digital_led_bar

OBJS := $(SRCS:%.c=$(OBJDIR)/%.o)


all:
	@echo "To make server:"
	@echo "  make server"
	@echo "To generate server assembly:"
	@echo "  make build/asm/server.s"
	@echo "Available test executables:"
	@echo "  make scope_test"
	@echo "  make lock_in_test"
	@echo "  make rf_write_test"
	@echo "  make rf_loopback"
	@echo "  make digital_led_bar"


server: server.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(BUILDDIR)/$@


$(TESTS): %: tests/%.c $(OBJS)
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $< $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(BUILDDIR)/$@


$(OBJDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


$(ASMDIR)/%.s: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -S $< -o $@



clean:
	rm -rf $(BUILDDIR)

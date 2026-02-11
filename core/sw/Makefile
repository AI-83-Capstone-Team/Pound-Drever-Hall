CFLAGS  := -std=gnu11 -Wall -Werror
CFLAGS += -Icontrol/inc -DDEBUG
LDLIBS := -static -lm -lstdc++ -lpthread #-lsocketc

SRC_DIRS := control 
OBJDIR := build/objects
ASMDIR := build/asm
BUILDDIR := build

SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

OBJS := $(SRCS:%.c=$(OBJDIR)/%.o)


all:
	@echo "To make server:"
	@echo "  make server"
	@echo "To generate server assembly:"
	@echo "  make build/asm/server.s"



server: server.c $(OBJS)
	$(CC) $(CFLAGS) $< $(OBJS) $(LDLIBS) -o $(BUILDDIR)/$@




$(OBJDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


$(ASMDIR)/%.s: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -S $< -o $@



clean:
	rm -rf $(BUILDDIR)
	rm -rf .cache

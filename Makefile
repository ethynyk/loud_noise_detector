CC 			= gcc
CFLAGS 		= -g -Wall -O0

INCLUDES	= -I.

LIBS		=-pthread

TARGET 		= lndetect.bin

SRCS		= $(wildcard *.c)
OBJS		= $(SRCS:.c=.o)
DEPS		= $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

-include $(DEPS)

%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET) $(DEPS)

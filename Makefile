OBJS = main.o draw.o

CFLAGS = -I/usr/include/cairo/ -lcairo -Idraw
LNFLAGS = -lm -lcairo 

CC = gcc
LD = gcc

TARGET = mct

all: $(TARGET) 

.c.o: $<
	@$(CC) -c $< $(CFLAGS)

$(TARGET): $(OBJS)
	@$(LD) -o $(TARGET) $(OBJS) $(LNFLAGS)

clean:
	rm mct \
	*~ \
	*.png \
	*.o

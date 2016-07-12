OBJS = main.o draw.o

CFLAGS = -I/usr/include/cairo/ -lcairo -Idraw
LNFLAGS = -lm -lcairo 

CC = gcc
LD = gcc

TARGET = prog

# cairo:
# 	gcc main.c -o prog $(CFLAGS) -lm 

all: $(TARGET) 

.c.o: $<
	@$(CC) -c $< $(CFLAGS)

$(TARGET): $(OBJS)
	@$(LD) -o $(TARGET) $(OBJS) $(LNFLAGS)

clean:
	rm prog \
	*~ \
	*.png \
	*.o

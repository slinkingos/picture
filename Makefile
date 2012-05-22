TARGET=main
CC=gcc
OBJS=main.o v4l.o

all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

main.o:main.c v4l.h
	$(CC) -c -o main.o main.c

v4lcto.o:v4l.c v4l.h
	$(CC) -c -o v4l.o v4l.c

clean:
	rm -rf *.o *.bak *~ $(TARGET)

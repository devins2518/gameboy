CC ?= gcc
CFLAGS = `sdl2-config --cflags` -Wall -Werror -std=c89
LDFLAGS = `sdl2-config --libs`

TARGET = gameboy

SRCS = $(wildcard src/*.c)

OBJS = $(wildcard build/object/*.o)

all: $(TARGET)

$(TARGET): build
	@$(CC) $(LDFLAGS) -o ./build/$(TARGET) $(OBJS)

build:
	@mkdir -p ./build/object
	@$(CC) $(CFLAGS) -c $(SRCS)
	@mv *.o ./build/object

run: $(TARGET)
	@./build/gameboy

clean:
	@$(RM) -rf ./build

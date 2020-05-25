EXEC=simulation

SRC=main.c\
	types.c\
	functions.c\
	draw.c\
	pid.c\
	simulator.o

OBJ=main.o\
	types.o\
	functions.o\
	draw.o\
	pid.o\
	simulator.o


WARNINGS = -Wall -pedantic
OPTIMIZE = -O3

CC=gcc
CFLAGS=-std=c99 `pkg-config --cflags glfw3 glew`
LDFLAGS=`pkg-config --static --libs glfw3 glew`

OPTIONS=-lm -lpthread  # to use math.h and pthread.h


# execute $(EXEC) target if no argument is provided to the make command
all: $(EXEC)

install:
	echo "Installing is not supported"

run:
	./$(EXEC) $(ARGS)

# generate the binary file from ".o" files
$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(WARNINGS) $(OPTIMIZE) $(OPTIONS)

# applied for each ".o" file needed in the previous target
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(LDFLAGS) $(WARNINGS) $(OPTIMIZE) $(OPTIONS)

# prevent make from mistaking clean and run targets with namesake files
.PHONY: clean run

clean:
	rm -f $(OBJ) $(EXEC)


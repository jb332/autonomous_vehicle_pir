EXEC=simulation

SRC=simulation.c\
    simulation_point.c\
    simulation_vehicle.c\
    simulation_circuit.c\
    simulation_window.c\
    simulation_functions.c

OBJ=simulation.o\
    simulation_point.o\
    simulation_vehicle.o\
    simulation_circuit.o\
    simulation_window.o\
    simulation_functions.o

WARNINGS = -Wall -pedantic  # add "-ansi"
#DEBUG = -ggdb -fno-omit-frame-pointer
OPTIMIZE = -O3

CC=gcc
CFLAGS=-std=c99 `pkg-config --cflags glib-2.0 gio-2.0 gobject-2.0 gtk+-3.0`  #replace c99 with c89
LDFLAGS=`pkg-config --libs glib-2.0 gio-2.0 gobject-2.0 gtk+-3.0`

OPTIONS=-lm  # to use the math library


# execute $(EXEC) target if no argument is provided to the make command
all: $(EXEC)

install:
	echo "Installing is not supported"

run:
	./simulation $(ARGS)

# generate the binary file from ".o" files
$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(WARNINGS) $(OPTIMIZE) $(OPTIONS) #$(DEBUG)

# applied for each ".o" file needed in the previous target
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(WARNINGS) $(OPTIMIZE) $(OPTIONS) #$(DEBUG)

# prevent make from mistaking clean and run targets with namesake files
.PHONY: clean run

clean:
	rm -f $(OBJ) $(EXEC)


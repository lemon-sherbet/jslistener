#GENERATED AUTOMATICALLY BY lcmk -> https://github.com/lemon-sherbet/lcmk

CC=gcc
CFLAGS=
LINK=-lallegro
PROG=jslistener
OBJ=main.o 

all: $(PROG)
$(PROG): $(OBJ)
	$(CC) $(CFLAGS) $(LINK) $(OBJ) -o $(PROG)
main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c
clean:
	rm -f $(OBJ) $(PROG)

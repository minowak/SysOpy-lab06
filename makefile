NAME=zad1
CFLAGS=-Wall -O2

CC=gcc
VPATH=./src

SRC=zad1.c
OBJ=$(SRC:.c=.o)

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $^

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $^

clean:
	rm -rf *.o

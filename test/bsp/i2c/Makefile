
CC = gcc
CFLAGS = -Wall -g
SRC = a2b-commandlist-app.c
OBJ = $(SRC:.c=.o)
TARGET = a2b-commandlist-app

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: clean


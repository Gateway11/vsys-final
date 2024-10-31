CC = gcc

CFLAGS = -Ia2b-commandlist -Wall -g

TARGET = adi_a2b_busconfig

SRC = adi_a2b_busconfig.c
OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

#CROSSBIN = ../../aarch64--glibc--stable-2022.03-1/bin/aarch64-buildroot-linux-gnu-
CC = $(CROSSBIN)gcc

CFLAGS = $(addprefix -I, $(shell find a2bstack -name "inc")) \
         -Ia2bstack/a2bstack/src \
         -Ia2bstack/a2bplugin-master/src \
         -Isrc/a2bstack-pal/platform \
         -Isrc/a2bstack-pal \
         -Isrc \
         -Iexamples/demo/app-plugin/inc \
         -Wno-unused-variable \
         -Wno-unused-function \
         -Wno-unknown-pragmas \
         -Wno-unused-but-set-variable \
         -Wall -fvisibility=hidden -Os #-flto# -g

LDFLAGS = -Wl,-s -lpthread

TARGET = a2bapp-linux

SRC = $(shell find a2bstack src examples/demo/app-plugin -name "*.c")
OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

NV_TOPDIR=/home/$(USER)/thor/new/sdk_7_0_0_1/ciu98
#CROSSBIN = $(NV_TOPDIR)/aarch64--glibc--stable-2022.03-1/bin/aarch64-buildroot-linux-gnu-
#CROSSBIN = ../../aarch64--glibc--stable-2022.03-1/bin/aarch64-buildroot-linux-gnu-
CC = $(CROSSBIN)gcc

CFLAGS = $(addprefix -I, $(shell find a2bstack -name "inc")) \
         -Ia2bstack/a2bstack/src \
         -Ia2bstack/a2bplugin-master/src \
         -Ia2bstack/a2bplugin-slave/src \
         -Isrc/a2bstack-pal/platform \
         -Isrc/a2bstack-pal \
         -Isrc \
         -Iexamples/demo/app-plugin/inc \
         -Wno-unused-variable \
         -Wno-unused-function \
         -Wall -fvisibility=hidden -Os #-flto# -g

LDFLAGS = -Wl,-s

TARGET = a2bapp-linux

SRC = $(shell find a2bstack -name "*.c") $(shell find src -name "*.c") examples/demo/app-plugin/src/a2bapp.c #adi_a2b_busconfig.c
OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

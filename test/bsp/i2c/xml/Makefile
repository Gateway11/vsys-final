NV_TOPDIR=/home/$(USER)/thor/new/sdk_7_0_0_1/ciu98
#CROSSBIN = $(NV_TOPDIR)/aarch64--glibc--stable-2022.03-1//bin/aarch64-buildroot-linux-gnu-
#CROSSBIN = ../aarch64--glibc--stable-2022.03-1//bin/aarch64-buildroot-linux-gnu-
CC = $(CROSSBIN)gcc

CFLAGS = -Wall -ffunction-sections -fdata-sections -fvisibility=hidden -Os -flto# -g
LDFLAGS = -Wl,-s

TARGET = adi_a2b_busconfig

SRC = adi_a2b_busconfig.c a2b-pal-interface.c
OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	#tar -zcvf - $(TARGET) adi_a2b_commandlist.xml | base64 > output.txt
	tar -cvf - $(TARGET) adi_a2b_commandlist.xml | xz -9 --extreme | base64 > output.txt
	#base64 -d output.txt | tar -xvf -

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) output.txt

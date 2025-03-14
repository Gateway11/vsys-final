NV_TOPDIR=/home/$(USER)/thor/new/sdk_7_0_0_1/ciu98
#CROSSBIN = $(NV_TOPDIR)/aarch64--glibc--stable-2022.03-1/bin/aarch64-buildroot-linux-gnu-
#CROSSBIN = ../aarch64--glibc--stable-2022.03-1/bin/aarch64-buildroot-linux-gnu-
CC = $(CROSSBIN)gcc

CFLAGS = -Ia2b-commandlist -Wall -fvisibility=hidden -Os -flto# -g
LDFLAGS = -Wl,-s

TARGET = adi_a2b_busconfig

SRC = adi_a2b_busconfig.c a2b-pal-interface.c
OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	tar -cvf - $(TARGET) busconfig.sh run.sh adi_a2b_commandlist.xml | xz -9 --extreme | base64 > output.txt
	-@xclip -sel clip < output.txt
	-@#base64 -d output.txt | tar -xJf -
	-@#cat part_* > output.tar.gz

copy:
	@tar -cvf - $(FILE) | xz -9 --extreme | base64 > output.txt
	@split -b $(shell expr 800 \* 1024) output.txt part_ && echo "Total parts created: $$(ls part_* | wc -l | tr -d ' ')"
	@for part in part_*; do \
		cat $$part | xclip -sel clip; read -n1 -r -p "Press any key to continue..."; \
	done
	@rm -f part_* output.txt && echo "All parts processed and deleted."

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) output.txt

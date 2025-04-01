#CROSSBIN = ../aarch64--glibc--stable-2022.03-1/bin/aarch64-buildroot-linux-gnu-
CC = $(CROSSBIN)gcc
SHELL = /bin/bash

CFLAGS = -Ia2b-commandlist -Wall -fvisibility=hidden -Os -flto# -g
LDFLAGS = -Wl,-s

TARGET = adi_a2b_busconfig

SRC = adi_a2b_busconfig.c a2b-pal-interface.c
OBJ = $(SRC:.c=.o)

$(TARGET): $(OBJ)
	@$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@tar -cf - $(TARGET) busconfig.sh run.sh adi_a2b_commandlist.xml | xz -9 --extreme | base64 > output.txt
	-@xclip -sel clip < output.txt
	-@#scp -J xuwy@10.106.224.114,zhangcy52@192.168.5.20 nvidia@192.168.1.109:/home/nvidia/record.wav .
	-@#base64 -d output.txt | tar -xJf - && truncate -s 0 output.txt
	-@#cat part_* > output.tar.gz

copy:
	@tar -cf - $(FILE) | xz -9 --extreme | base64 | split -b $(shell expr 1000 \* 1024) - part_
	@for part in part_*; do \
		cat $$part | xclip -sel clip || cat $$part | nc -q 0 10.106.224.114 5555; \
		read -n1 -r -p "($$((++count)) / $$(ls part_* | wc -l | tr -d ' ')) Press any key to continue..."; \
	done
	@rm -f part_* output.txt && echo "All parts processed and deleted."

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) output.txt

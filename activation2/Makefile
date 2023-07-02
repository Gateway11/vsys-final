CC := g++
LD := g++
NDK_BUILD := ndk-build
PROJECT_ROOT := $(shell pwd)
PLATFORM_SDK_VERSION ?= 23

rkvacti.out-dir-objs := $(PROJECT_ROOT)/out/obj
rkvacti.out-dir-libs := $(PROJECT_ROOT)/out/libs

rkvacti.cpp.src-dirs := $(shell find src -maxdepth 3 -type d)
rkvacti.cpp.src-files := $(foreach dir, $(rkvacti.cpp.src-dirs), $(wildcard $(dir)/*.cpp))
rkvacti.cpp.src-objs = $(patsubst %.cpp, %.o, $(rkvacti.cpp.src-files))
rkvacti.cpp.src-flags := -std=c++11 -fPIC #-Wno-format
rkvacti.cpp.src-includes := -Iinclude -Isrc -Isrc/legacy -Ithirdparty/include
rkvacti.cpp.src-deps := -Lthirdparty/libs/dnn/linux-x86_64 -lr2vt5 -lr2ssp -lztvad -lmkl_intel_lp64 -lmkl_sequential -lmkl_core
rkvacti.cpp.out-file := librkvacti.so
ifeq ($(shell uname), Linux)
ifeq ($(shell uname -m), x86_64)
	rkvacti.cpp.out-class = -shared
else
	$(error Dynamic libraries not supported on this platform.)
endif
else
ifeq ($(shell uname), Darwin)
	$(error Dynamic libraries not supported on this platform.)
endif
endif

rkvacti.ndk.app-script := $(PROJECT_ROOT)/ndk-app.mk
rkvacti.ndk.build-script := $(PROJECT_ROOT)/ndk.mk

rkvacti.java.src-dirs := $(shell find android/java -maxdepth 3 -type d)
rkvacti.java.src-files := $(foreach dir, $(rkvacti.java.src-dirs), $(wildcard $(dir)/*.java))
rkvacti.java.sourcepath := android/java
rkvacti.java.classpath := $(ANDROID_SDK_HOME)/platforms/android-$(PLATFORM_SDK_VERSION)/android.jar
rkvacti.java.classes-outdir := $(PROJECT_ROOT)/out/classes
rkvacti.java.out-file := $(rkvacti.out-dir-libs)/rkvacti.jar

.PHONY: all android clean test

$(rkvacti.cpp.out-file): $(rkvacti.cpp.src-objs)
	@mkdir -p $(rkvacti.out-dir-libs)
	$(LD) $(rkvacti.cpp.out-class) $^ $(rkvacti.cpp.src-flags) $(rkvacti.cpp.src-deps) -o $(rkvacti.out-dir-libs)/$@

%.o:%.cpp
	$(CC) -c $< $(rkvacti.cpp.src-flags) $(rkvacti.cpp.src-includes) -o $@

all: $(rkvacti.cpp.out-file) test

android: $(rkvacti.java.out-file)
	@mkdir -p $(rkvacti.out-dir-objs)
	@mkdir -p $(rkvacti.out-dir-libs)
	$(NDK_BUILD) \
		NDK_PROJECT_PATH=$(PROJECT_ROOT) \
		NDK_APPLICATION_MK=$(rkvacti.ndk.app-script) \
		APP_BUILD_SCRIPT=$(rkvacti.ndk.build-script) \
		NDK_OUT=$(rkvacti.out-dir-objs) \
		NDK_LIBS_OUT=$(rkvacti.out-dir-libs) \
		PLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION)

$(rkvacti.java.out-file): $(rkvacti.java.src-files)
ifeq ($(ANDROID_SDK_HOME),)
	$(error ANDROID_SDK_HOME missed, exec shell command 'export ANDROID_SDK_HOME=...')
endif
	mkdir -p $(rkvacti.java.classes-outdir)
	mkdir -p $(rkvacti.out-dir-libs)
	javac -sourcepath $(rkvacti.java.sourcepath) -classpath $(rkvacti.java.classpath) -d $(rkvacti.java.classes-outdir) $(rkvacti.java.src-files)
	jar cf $@ -C $(rkvacti.java.classes-outdir) .

test: main.o
	$(CC) $^ $(rkvacti.cpp.src-flags) $(rkvacti.cpp.src-deps) -L$(rkvacti.out-dir-libs) -lrkvacti -o test_activation

clean:
	-rm -f *.o
	-rm -f src/*.o
	-rm -f src/legacy/*.o
	-rm -f $(target)
	-rm -rf out


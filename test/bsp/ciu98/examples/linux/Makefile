
HED_SDK = ../..
RPI = YES
PORT_DIR =  $(HED_SDK)/port/linux
LIBDIR = $(HED_SDK)/src/util
LIBDIR += $(HED_SDK)/src/api
LIBDIR += $(HED_SDK)/src/cmd
LIBDIR += $(HED_SDK)/src/ial
LIBDIR += $(HED_SDK)/src/proto
LIBDIR += $(HED_SDK)/extern
 
OTHDIR = ./demo/addins
OTHDIR +=./demo/all_api_test
 
BINDIR = ../../bin
#ENGDIR = ./demo/HED_SDK_engine
LIB_INSTALL_DIR = /usr/lib/arm-linux-gnueabihf
ENGINE_INSTALL_DIR = $(LIB_INSTALL_DIR)/engines-1.1

APPDIR += ./demo/cmd_api_test
ifdef ENGDIR
	APPDIR += ./demo/HED_SDK_engine_test
endif

INCDIR = $(HED_SDK)/extern
INCDIR += $(HED_SDK)/port/linux
INCDIR += $(HED_SDK)/port/linux/raspberrypi_cnf
INCDIR += $(HED_SDK)/src/api
INCDIR += $(HED_SDK)/src/cmd
INCDIR += $(HED_SDK)/src/ial
INCDIR += $(HED_SDK)/src/proto
INCDIR += $(HED_SDK)/src/util
INCDIR += $(HED_SDK)/examples/linux/demo/addins
INCDIR += $(HED_SDK)/examples/linux/demo/all_api_test
ifdef ENGDIR
	INCDIR += $(HED_SDK)/examples/linux/demo/HED_SDK_engine
endif
ifdef INCDIR
	INCSRC := $(shell find $(INCDIR) -name '*.h')
	INCDIR := $(addprefix -I ,$(INCDIR))
endif

ifdef LIBDIR
	ifdef PORT_DIR
	        LIBSRC =  $(PORT_DIR)/port_i2c.c
	        LIBSRC += $(PORT_DIR)/port_spi.c
	        LIBSRC += $(PORT_DIR)/port_util.c
	        ifeq ($(RPI), YES)
	                LIBSRC += $(PORT_DIR)/raspberrypi_cnf/port_config.c
        	endif

	endif

        LIBSRC += $(shell find $(LIBDIR) -name '*.c')
        LIBOBJ := $(patsubst %.c,%.o,$(LIBSRC))
	LIB = libHed_SDK.so
endif

ifdef OTHDIR
	OTHSRC := $(shell find $(OTHDIR) -name '*.c')
	OTHOBJ := $(patsubst %.c,%.o,$(OTHSRC))
endif

ifdef APPDIR
	APPSRC := $(shell find $(APPDIR) -name '*.c')

	ifndef ENGDIR
		ENGTEST := ./demo/cmd_api_test/server_test.c ./demo/cmd_api_test/client_test.c ./demo/cmd_api_test/engine_test.c ./demo/cmd_api_test/gen_client_rsa2048_key.c ./demo/cmd_api_test/gen_client_nist256_key.c
		APPSRC := $(filter-out $(ENGTEST),$(APPSRC))
	endif
	APPOBJ := $(patsubst %.c,%.o,$(APPSRC))
	APPS := $(patsubst %.c,%,$(APPSRC))
endif

ifdef OTHDIR
	OTHSRC := $(shell find $(OTHDIR) -name '*.c')
	OTHOBJ := $(patsubst %.c,%.o,$(OTHSRC))
endif

ifdef ENGDIR
	ENGSRC := $(shell find $(ENGDIR) -name '*.c')
	ENGOBJ := $(patsubst %.c,%.o,$(ENGSRC))
	ENG = hed_engine.so
endif

CC = gcc
DEBUG = -g

CFLAGS += -c  
CFLAGS += $(DEBUG)
CFLAGS += $(INCDIR) 
#CFLAGS += -Wall 
CFLAGS += -DENGINE_DYNAMIC_SUPPORT
#CFLAGS += -DMODULE_ENABLE_DTLS_MUTUAL_AUTH

LDFLAGS += -lrt   
LDFLAGS += -lpthread
ifdef ENGDIR
LDFLAGS += -lssl
LDFLAGS += -lcrypto
endif

LDFLAGS_1 = -L$(BINDIR) -Wl,-R$(BINDIR)
LDFLAGS_1 += -lHed_SDK

.Phony : install uninstall all clean install_lib install_engine

all : $(BINDIR)/$(LIB) $(APPS) $(BINDIR)/$(ENG)


install:
ifdef ENGDIR
	@ln -s $(realpath $(BINDIR)/$(ENG)) $(ENGINE_INSTALL_DIR)/$(ENG)
endif
	@ln -s $(realpath $(BINDIR)/$(LIB)) $(LIB_INSTALL_DIR)/$(LIB)
	
uninstall: clean
ifdef ENGDIR
	@-rm $(ENGINE_INSTALL_DIR)/$(ENG)
endif
	@-rm $(LIB_INSTALL_DIR)/$(LIB)

clean :
	@rm -rf $(LIBOBJ)
	@rm -rf $(OTHOBJ)
	@rm -rf $(APPOBJ)
	@rm -rf $(ENGOBJ)
	@rm -rf $(APPS)
	@rm -rf ../../bin/*
  
ifdef ENGDIR	
$(BINDIR)/$(ENG): %: $(ENGOBJ) $(INCSRC) $(BINDIR)/$(LIB)
	@echo "################ linking $@ #################"
	@mkdir -p ../../bin
	@$(CC) $(LDFLAGS) $(LDFLAGS_1) $(ENGOBJ) -shared -o $@
endif

$(APPS): %: $(OTHOBJ) $(INCSRC) %.o
	@echo "################ linking $@ #################"
	@mkdir -p ../../bin
	@$(CC) $(LDFLAGS) $@.o $(OTHOBJ) -o $@ $(LDFLAGS_1)
	@cp $@ ../../bin/.

$(BINDIR)/$(LIB): %: $(LIBOBJ) $(INCSRC)
	@echo "################ linking $@ #################"
	@mkdir -p ../../bin
	@$(CC) $(LDFLAGS) $(LIBOBJ) -shared -o $@

$(LIBOBJ): %.o: %.c $(INCSRC)
	@echo "**************** build lib $@ ***************"
	@$(CC) $(CFLAGS) $< -o $@

%.o: %.c $(INCSRC)
	@echo "**************** build demo $@ ***************"
	@$(CC) $(CFLAGS) $< -o $@
	

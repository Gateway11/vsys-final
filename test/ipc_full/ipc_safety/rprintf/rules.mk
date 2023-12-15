LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
        $(LOCAL_DIR)/rprintf-socket.c \
        $(LOCAL_DIR)/rprintf-ipcc.c \
        $(LOCAL_DIR)/rprintf-test.c

#MODULE_DEPS += \
	3rd/rpmsg-lite \
	framework/communication

MODULE_COMPILEFLAGS += -Wno-format -Wno-unused-variable -Wno-sign-compare

include make/module.mk

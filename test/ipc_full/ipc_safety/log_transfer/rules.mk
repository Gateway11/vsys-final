LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/log_transfer.c
#GLOBAL_INCLUDES += $(LOCAL_DIR)/include

#MODULE_COMPILEFLAGS += -Wno-format -Wno-unused-variable -Wno-sign-compare

include $(MKROOT)/module.mk

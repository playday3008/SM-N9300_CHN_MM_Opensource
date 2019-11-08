LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=\
        libbridge_devif.c \
        libbridge_if.c \
        libbridge_init.c \
        libbridge_misc.c

LOCAL_CFLAGS:= -Wall -O2 -g
LOCAL_CFLAGS += -DFEATURE_ANDROID

LOCAL_SYSTEM_SHARED_LIBRARIES := libc libcutils
LOCAL_MODULE:= libbridge
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)

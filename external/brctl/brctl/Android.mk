LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=\
        brctl_cmd.c \
        brctl_disp.c \
        brctl.c

LOCAL_CFLAGS := -O2 -g -Wno-ignored-qualifiers
LOCAL_CFLAGS += -Wno-sign-compare -Wno-missing-field-initializers
LOCAL_CFLAGS += -DHAVE_CONFIG_H -D_U_="__attribute__((unused))"
LOCAL_CFLAGS += -DFEATURE_ANDROID

LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/../libbridge/

LOCAL_SYSTEM_SHARED_LIBRARIES := libc libcutils

LOCAL_MODULE_TAGS := debug

LOCAL_STATIC_LIBRARIES += libbridge

LOCAL_MODULE := brctl

include $(BUILD_EXECUTABLE)

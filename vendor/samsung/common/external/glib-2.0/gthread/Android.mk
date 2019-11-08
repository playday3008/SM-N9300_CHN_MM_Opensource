LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=			\
	gthread-impl.c

LOCAL_MODULE := libgthread-2.0

LOCAL_C_INCLUDES :=			\
	$(LOCAL_PATH)			\
	$(GLIB_TOP)/android-internal	\
	$(GLIB_C_INCLUDES)

LOCAL_CFLAGS :=				\
	-DHAVE_CONFIG_H			\
    -DG_LOG_DOMAIN=\"GThread\"		\
    -D_POSIX4_DRAFT_SOURCE		\
    -D_POSIX4A_DRAFT10_SOURCE		\
    -U_OSF_SOURCE			\
    -DG_DISABLE_DEPRECATED 
LOCAL_CFLAGS += -Wno-type-limits

LOCAL_MODULE_TAGS := eng optional

ifeq ($(TARGET_ARCH), arm64)
LOCAL_MULTILIB := 32
endif

ifeq ($(GLIB_BUILD_STATIC),true)
include $(BUILD_STATIC_LIBRARY)
else
LOCAL_SHARED_LIBRARIES := libglib-2.0

include $(BUILD_SHARED_LIBRARY)
endif

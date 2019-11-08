LOCAL_PATH:= $(call my-dir)

ifeq ($(TARGET_ARCH), arm64)
include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto_static_dmverity
LOCAL_MODULE_TAGS := optional
LOCAL_PREBUILT_LIBS := /arm64/libcrypto_static_dmverity.a
include $(BUILD_MULTI_PREBUILT)
else
include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto_static_dmverity
LOCAL_MODULE_TAGS := optional
LOCAL_PREBUILT_LIBS := libcrypto_static_dmverity.a
include $(BUILD_MULTI_PREBUILT)
endif

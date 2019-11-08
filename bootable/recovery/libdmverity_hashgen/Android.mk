# Copyright 2005 The Android Open Source Project
#
# Android.mk for adb
#

LOCAL_PATH:= $(call my-dir)
ifneq (sc,$(findstring sc,$(TARGET_BOARD_PLATFORM)))
include $(CLEAR_VARS)

LOCAL_SRC_FILES := libdmverity_hashgen.c

LOCAL_CFLAGS := -Wall -DPARALLEL_HASH

ifeq ($(TARGET_ARCH), arm64)
LOCAL_CFLAGS += -D_USE_LFS
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/.. \
		$(TOP)/external/boringssl/include
#LOCAL_C_INCLUDES += system/extras/ext4_utils

LOCAL_MODULE := libdmverity_hashgen
LOCAL_STATIC_LIBRARIES := libc libcrypto_static

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := dm_verity_hash
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -D__NO_UI_PRINT
LOCAL_CFLAGS += -D__USE_DM_VERITY -g

LOCAL_SRC_FILES := dm_verity_hash.c 
LOCAL_C_INCLUDES += system/extras/ext4_utils \
	$(TOP)/external/boringssl/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/..

ifeq (exynos,$(findstring exynos,$(TARGET_SOC)))
ifeq ($(TARGET_SOC),exynos5433)
LOCAL_CFLAGS += -DEXYNOS_5433
endif
ifeq ($(TARGET_SOC),exynos8890)
LOCAL_CFLAGS += -DEXYNOS_8890
LOCAL_CFLAGS += -DUSE_SHA256
endif
ifeq ($(TARGET_SOC),exynos7420)
LOCAL_CFLAGS += -DEXYNOS_7420
LOCAL_CFLAGS += -DUSE_SHA1
endif
ifeq ($(TARGET_SOC),exynos7870)
LOCAL_CFLAGS += -DEXYNOS_7870
LOCAL_CFLAGS += -DUSE_SHA256
endif
ifeq ($(TARGET_PROJECT), TRE)
LOCAL_CFLAGS += -DTRE_PROJECT
endif
ifeq ($(TARGET_SOC),exynos7580)
LOCAL_CFLAGS += -DEXYNOS_7580
endif
ifeq ($(TARGET_SOC),exynos3475)
LOCAL_CFLAGS += -DEXYNOS_3475
endif
ifeq ($(TARGET_SOC),exynos5430)
LOCAL_CFLAGS += -DEXYNOS_5430
endif

else
ifeq ($(TARGET_BOARD_PLATFORM),apq8084)
LOCAL_CFLAGS += -DAPQ_8084
endif
ifeq ($(TARGET_BOARD_PLATFORM),msm8996)
LOCAL_CFLAGS += -DMSM_8996
LOCAL_CFLAGS += -DUSE_SHA256
endif
ifeq ($(TARGET_BOARD_PLATFORM),msm8916)
LOCAL_CFLAGS += -DMSM_8916
endif
ifeq ($(TARGET_BOARD_PLATFORM),msm8953)
LOCAL_CFLAGS += -DUSE_SHA256
endif
endif

LOCAL_STATIC_LIBRARIES := \
	libc \
	libstdc++ \
	libext4_utils_static \
	libmtdutils \
	libdmverity_hashgen \
	libmincrypt \
	libfs_mgr \
	libcrypto_static

LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/sbin
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_SRC_FILES := libdmverity_hashgen.c

LOCAL_CFLAGS := -Wall

LOCAL_C_INCLUDES += $(LOCAL_PATH)/.. \
		$(TOP)/external/boringssl/include
#LOCAL_C_INCLUDES += system/extras/ext4_utils

LOCAL_MODULE := libdmverity_hashgen_host

include $(BUILD_HOST_STATIC_LIBRARY)


# include $(CLEAR_VARS)

# LOCAL_SRC_FILES := libdmverity_hashgen.c img_dm_verity.c
# LOCAL_MODULE := img_dm_verity
# LOCAL_STATIC_LIBRARIES := libmincrypt libsparse_host libz

# include $(BUILD_HOST_EXECUTABLE)
endif
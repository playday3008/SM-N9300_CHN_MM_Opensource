# device/samsung/smdkc210/alsa-lib/Android.mk
#
# Copyright 2011 Samsung Electronics S.LSI Co. LTD
# Copyright 2008 Wind River Systems
#

ifeq ($(TARGET_BOARD_PLATFORM),mrvl)

# do not build alsa-lib in Marvell platform.

else

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libasound

LOCAL_MODULE_TAGS := eng optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

# libasound must be compiled with -fno-short-enums, as it makes extensive
# use of enums which are often type casted to unsigned ints.
LOCAL_CFLAGS := \
	-fPIC -DPIC -D_POSIX_SOURCE \
	-DALSA_CONFIG_DIR=\"/system/usr/share/alsa\" \
	-DALSA_PLUGIN_DIR=\"/system/usr/lib/alsa-lib\" \
	-DALSA_DEVICE_DIRECTORY=\"/dev/snd/\"
LOCAL_CFLAGS += -Wno-pointer-arith -Wno-strict-aliasing

LOCAL_LDFLAGS := -Wl,--no-fatal-warnings

LOCAL_SRC_FILES := $(sort $(call all-c-files-under, src))

# It is easier to exclude the ones we don't want...
#
LOCAL_SRC_FILES := $(filter-out src/alisp/alisp_snd.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/compat/hsearch_r.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/control/control_shm.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/control/ctl_symbols_list.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/pcm/pcm_d%.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/pcm/pcm_ladspa.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/pcm/pcm_shm.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/pcm/pcm_symbols_list.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/pcm/scopes/level.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/ucm/main.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/ucm/parser.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/ucm/utils.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/shmarea.c, $(LOCAL_SRC_FILES))

LOCAL_SHARED_LIBRARIES := \
    libdl

ifeq ($(TARGET_CPU_ABI), arm64-v8a)
LOCAL_MULTILIB := 32
endif

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libasound

LOCAL_MODULE_TAGS := eng optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

# libasound must be compiled with -fno-short-enums, as it makes extensive
# use of enums which are often type casted to unsigned ints.
LOCAL_CFLAGS := \
	-fPIC -DPIC -D_POSIX_SOURCE \
	-DALSA_CONFIG_DIR=\"/system/usr/share/alsa\" \
	-DALSA_PLUGIN_DIR=\"/system/usr/lib/alsa-lib\" \
	-DALSA_DEVICE_DIRECTORY=\"/dev/snd/\"
LOCAL_CFLAGS += -Wno-pointer-arith -Wno-strict-aliasing

LOCAL_LDFLAGS := -Wl,--no-fatal-warnings

LOCAL_SRC_FILES := $(sort $(call all-c-files-under, src))

# It is easier to exclude the ones we don't want...
#
LOCAL_SRC_FILES := $(filter-out src/alisp/alisp_snd.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/compat/hsearch_r.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/control/control_shm.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/control/ctl_symbols_list.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/pcm/pcm_d%.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/pcm/pcm_ladspa.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/pcm/pcm_shm.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/pcm/pcm_symbols_list.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/pcm/scopes/level.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/ucm/main.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/ucm/parser.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/ucm/utils.c, $(LOCAL_SRC_FILES))
LOCAL_SRC_FILES := $(filter-out src/shmarea.c, $(LOCAL_SRC_FILES))

ifeq ($(TARGET_ARCH), arm64)
LOCAL_MULTILIB := 32
endif

include $(BUILD_STATIC_LIBRARY)

endif

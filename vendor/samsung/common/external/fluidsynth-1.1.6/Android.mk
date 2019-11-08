#
# fluidsynth-1.1.6
#

BUILD_ROOT_PATH := $(call my-dir)

FLUID_ROOT := $(call my-dir)

LOCAL_PATH := $(call my-dir)


SUPPORT_ANDROID_REALTIME_SCHED_FLUID := true

include $(CLEAR_VARS)
#ALSA_INCLUDES := \
#    vendor/samsung/variant/alsa-lib/include \
#    $(JACK_ROOT)/linux/alsa
#ALSA_INCLUDES := \
#    device/samsung/smdk_common/alsa-lib/include
#    $(JACK_ROOT)/linux/alsa
#include $(BUILD_ROOT_PATH)/src/bindings/Android.mk
#include $(BUILD_ROOT_PATH)/src/drivers/Android.mk for the future
#include $(BUILD_ROOT_PATH)/src/midi/Android.mk
#include $(BUILD_ROOT_PATH)/src/synth/Android.mk
#include $(BUILD_ROOT_PATH)/src/rvoice/Android.mk
#include $(BUILD_ROOT_PATH)/src/sfloader/Android.mk
#include $(BUILD_ROOT_PATH)/src/utils/Android.mk

ifeq ($(SUPPORT_ANDROID_REALTIME_SCHED), true)
sched_c_include := bionic/libc/bionic \
	bionic/libc \
	frameworks/av/services/audioflinger
endif

fluid_include := \
	$(FLUID_ROOT)/src \
	$(FLUID_ROOT)/src/utils \
	$(FLUID_ROOT)/src/sfloader \
	$(FLUID_ROOT)/src/rvoice \
	$(FLUID_ROOT)/src/synth \
	$(FLUID_ROOT)/src/midi \
	$(FLUID_ROOT)/src/drivers \
	$(FLUID_ROOT)/src/bindings \
	$(FLUID_ROOT)/src/android \
	$(FLUID_ROOT)/bindings/fluidmax/fluidsynth \
	$(FLUID_ROOT)/include \
	$(FLUID_ROOT)/include/fluidsynth \
	$(FLUID_ROOT)/include/extra \
	$(FLUID_ROOT)/include/glib \
	bionic/libc/include \
	bionic/libm/include \
	bionic/libm/include \

fluid_libsources := \
	src/utils/fluid_conv.c \
	src/utils/fluid_hash.c \
	src/utils/fluid_list.c \
	src/utils/fluid_ringbuffer.c \
	src/utils/fluid_settings.c \
	src/utils/fluid_sys.c \
	src/utils/fluid_mathfunc.c \
	src/sfloader/fluid_defsfont.c \
	src/sfloader/fluid_ramsfont.c \
	src/rvoice/fluid_adsr_env.c \
	src/rvoice/fluid_chorus.c \
	src/rvoice/fluid_iir_filter.c \
	src/rvoice/fluid_lfo.c \
	src/rvoice/fluid_rvoice.c \
	src/rvoice/fluid_rvoice_dsp.c \
	src/rvoice/fluid_rvoice_event.c \
	src/rvoice/fluid_rvoice_mixer.c \
	src/rvoice/fluid_rev.c \
	src/synth/fluid_chan.c \
	src/synth/fluid_event.c \
	src/synth/fluid_gen.c \
	src/synth/fluid_mod.c \
	src/synth/fluid_synth.c \
	src/synth/fluid_tuning.c \
	src/synth/fluid_voice.c \
	src/midi/fluid_midi.c \
	src/midi/fluid_midi_router.c \
	src/midi/fluid_seqbind.c \
	src/midi/fluid_seq.c \
	src/drivers/fluid_adriver.c \
	src/drivers/fluid_mdriver.c \
	src/bindings/fluid_filerenderer.c \
	src/android/JAMSynth.c \
	src/android/SapaSynthClient.cpp
#	src/drivers/fluid_aufile.c \
#	src/bindings/fluid_cmd.c \
#	src/bindings/fluid_ladspa.c \

ifeq ($(SUPPORT_ANDROID_REALTIME_SCHED_FLUID), true)
sched_c_include := bionic/libc/bionic \
	bionic/libc \
	frameworks/av/services/audioflinger \
	frameworks/av/media/utils/include

sched_c_src := src/android/fluid_realtime.cpp
endif

# ========================================================
# libfluidsynth.so
# ========================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(fluid_libsources)

#to avoid __cxa_XXXX errors
LOCAL_C_INCLUDES := $(fluid_include)
ifeq ((TARGET_ARCH_VARIANT), armv7-a-neon)
LOCAL_CFLAGS := -DHAVE_CONFIG_H -march=armv7-a -O3 -mfloat-abi=softfp -mfpu=neon -DHAVE_NEON=1
else
LOCAL_CFLAGS := -DHAVE_CONFIG_H
endif
LOCAL_LDFLAGS := $(common_ldflags)
LOCAL_SHARED_LIBRARIES := libc libcutils libutils libdl libbinder libglib-2.0 libgthread-2.0 libjack
LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := libfluidsynth
ifeq ($(SUPPORT_ANDROID_REALTIME_SCHED_FLUID), true)
LOCAL_CFLAGS += -DFLUID_ANDROID_REALTIME_SCHED
LOCAL_C_INCLUDES += $(sched_c_include)
LOCAL_SRC_FILES += $(sched_c_src)
LOCAL_SHARED_LIBRARIES += libmediautils
endif

ifeq ($(TARGET_ARCH), arm64)
LOCAL_MULTILIB := 32
endif

include $(BUILD_SHARED_LIBRARY)


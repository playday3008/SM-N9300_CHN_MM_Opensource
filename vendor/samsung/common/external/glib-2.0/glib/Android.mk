LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ./libcharset/localcharset.c \
    gstrfuncs.c     \
    gstring.c       \
    gutf8.c	\
    gatomic.c       \
    gmem.c          \
    gprintf.c	\
    gthread.c       \
    gconvert.c      \
    gdataset.c      \
    gslice.c        \
    gslist.c        \
    ghash.c         \
    gerror.c        \
    gtestutils.c    \
    gutils.c	\
    garray.c        \
    gtimer.c        \
    gtimezone.c     \
    glist.c         \
    grand.c         \
    gmain.c         \
    gbitlock.c      \
    gpattern.c      \
    gfileutils.c    \
    gpoll.c         \
    gqsort.c        \
    gqueue.c        \
    gbuffer.c       \
    gstdio.c        \
    gmappedfile.c   \
    gmessages.c
#    gasyncqueue.c   \
#    gbacktrace.c    \
#    gbase64.c       \
#    gcache.c        \
#    gchecksum.c     \
#    gcompletion.c   \
#    gdate.c         \
#    gdatetime.c     \
#    gdir.c          \
#    ghook.c         \
#    ghostutils.c    \
#    gkeyfile.c      \
#    gmarkup.c       \
#    gnode.c         \
#    goption.c       \
#    gprimes.c       \
#    grel.c          \
#    gregex.c        \
#    gsequence.c     \
#    gthreadpool.c   \
#    gvariant.c      \
#    gvariant-core.c \
#    gvariant-parser.c \
#    gvariant-serialiser.c \
#    gvarianttypeinfo.c \
#    gvarianttype.c  \
#    giounix.c	\
#    giochannel.c    \
#    gtree.c         \
#    gscanner.c      \
#    gspawn.c	\
#    gbookmarkfile.c \
#    gshell.c

LOCAL_MODULE := libglib-2.0

LOCAL_C_INCLUDES := 			\
	$(GLIB_TOP)			\
	$(LOCAL_PATH)../../../bionic/libc/include \
	$(GLIB_TOP)/android		\
	$(GLIB_TOP)/android-internal	\
	$(LOCAL_PATH)/android-internal	\
	$(LOCAL_PATH)/libcharset       	\
	$(LOCAL_PATH)/gnulib           	\
	$(LOCAL_PATH)../gobject
#	$(LOCAL_PATH)/pcre

# ./glib private macros, copy from Makefile.am
LOCAL_CFLAGS := \
	-DLIBDIR=\"$(libdir)\"		\
	-DHAVE_CONFIG_H			\
	-DG_LOG_DOMAIN=\"GLib\"		\
	-DPCRE_STATIC			\
	-DG_DISABLE_DEPRECATED		\
	-DGLIB_COMPILATION
LOCAL_CFLAGS += -Wno-missing-field-initializers -Wno-sign-compare \
		-Wno-type-limits -Wno-switch

LOCAL_MODULE_TAGS := eng optional

ifeq ($(TARGET_ARCH), arm64)
LOCAL_MULTILIB := 32
endif

ifeq ($(GLIB_BUILD_STATIC),true)
include $(BUILD_STATIC_LIBRARY)
else
#LOCAL_STATIC_LIBRARIES := libpcre
#LOCAL_LDLIBS :=				\
#	-llog

include $(BUILD_SHARED_LIBRARY)
endif

#include $(LOCAL_PATH)/pcre/Android.mk

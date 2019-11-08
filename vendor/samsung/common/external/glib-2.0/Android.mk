# This file is the top android makefile for all sub-modules.

ifeq ($(TARGET_BOARD_PLATFORM),clovertrail)

# do not build glib-2.0 in Intel platform.

else

LOCAL_PATH := $(call my-dir)

GLIB_TOP := $(LOCAL_PATH)

GLIB_BUILD_STATIC := $(BUILD_STATIC)

GLIB_C_INCLUDES :=			\
	$(GLIB_TOP)			\
	$(GLIB_TOP)/glib		\
	$(GLIB_TOP)/android

GTHREAD_C_INCLUDES :=			\
	$(GLIB_C_INCLUDES)		\
	$(GLIB_TOP)/gthread

GOBJECT_C_INCLUDES :=			\
	$(GLIB_C_INCLUDES)		\
	$(GLIB_TOP)/gobject		\
	$(GLIB_TOP)/gobject/android

GMODULE_C_INCLUDES :=			\
	$(GLIB_C_INCLUDES)		\
	$(GLIB_TOP)/gmodule

GIO_C_INCLUDES :=			\
	$(GLIB_C_INCLUDES)		\
	$(GLIB_TOP)/gio			\
	$(GLIB_TOP)/gio/android

GLIB_SHARED_LIBRARIES :=		\
	libgmodule-2.0			\
	libgobject-2.0			\
	libgthread-2.0			\
	libglib-2.0

GLIB_STATIC_LIBRARIES :=		\
	$(GLIB_SHARED_LIBRARIES)	\
	libpcre

include $(CLEAR_VARS)


include $(GLIB_TOP)/glib/Android.mk
include $(GLIB_TOP)/gthread/Android.mk
#include $(GLIB_TOP)/gio/Android.mk
#include $(GLIB_TOP)/tests/Android.mk

endif

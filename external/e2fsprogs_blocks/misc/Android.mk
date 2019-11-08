LOCAL_PATH := $(call my-dir)


#########################################################################
# Build filefrag
#
include $(CLEAR_VARS)

filefrag_src_files := \
    filefrag.c

filefrag_c_includes := \
    external/e2fsprogs_blocks/lib

lsattr_cflags := -O2 -g -W -Wall \
    -DHAVE_UNISTD_H \
    -DHAVE_ERRNO_H \
    -DHAVE_NETINET_IN_H \
    -DHAVE_SYS_IOCTL_H \
    -DHAVE_SYS_MMAN_H \
    -DHAVE_SYS_MOUNT_H \
    -DHAVE_SYS_PRCTL_H \
    -DHAVE_SYS_RESOURCE_H \
    -DHAVE_SYS_SELECT_H \
    -DHAVE_SYS_STAT_H \
    -DHAVE_SYS_TYPES_H \
    -DHAVE_STDLIB_H \
    -DHAVE_STRCASECMP \
    -DHAVE_STRDUP \
    -DHAVE_MMAP \
    -DHAVE_UTIME_H \
    -DHAVE_GETPAGESIZE \
    -DHAVE_LSEEK64 \
    -DHAVE_LSEEK64_PROTOTYPE \
    -DHAVE_EXT2_IOCTLS \
    -DHAVE_LINUX_FD_H \
    -DHAVE_TYPE_SSIZE_T \
    -DHAVE_GETOPT_H \
    -DHAVE_SYS_TIME_H \
    -DHAVE_SYS_PARAM_H \
    -DHAVE_SYSCONF

filefrag_system_shared_libraries := libc

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(filefrag_src_files)
LOCAL_C_INCLUDES := $(filefrag_c_includes)
LOCAL_CFLAGS := $(filefrag_cflags)
LOCAL_STATIC_LIBRARIES := $(filefrag_system_shared_libraries)
LOCAL_MODULE := filefrag_blocks
LOCAL_MODULE_TAGS := optional
LOCAL_FORCE_STATIC_EXECUTABLE := true

# Static link into ramdisk
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)

include $(BUILD_EXECUTABLE)

#########################################################################
# Build e2image
#
include $(CLEAR_VARS)

e2image_src_files := \
    e2image.c

e2image_c_includes := \
    external/e2fsprogs_blocks/lib

lsattr_cflags := -O2 -g -W -Wall \
    -DHAVE_UNISTD_H \
    -DHAVE_ERRNO_H \
    -DHAVE_NETINET_IN_H \
    -DHAVE_SYS_IOCTL_H \
    -DHAVE_SYS_MMAN_H \
    -DHAVE_SYS_MOUNT_H \
    -DHAVE_SYS_PRCTL_H \
    -DHAVE_SYS_RESOURCE_H \
    -DHAVE_SYS_SELECT_H \
    -DHAVE_SYS_STAT_H \
    -DHAVE_SYS_TYPES_H \
    -DHAVE_STDLIB_H \
    -DHAVE_STRCASECMP \
    -DHAVE_STRDUP \
    -DHAVE_MMAP \
    -DHAVE_UTIME_H \
    -DHAVE_GETPAGESIZE \
    -DHAVE_LSEEK64 \
    -DHAVE_LSEEK64_PROTOTYPE \
    -DHAVE_EXT2_IOCTLS \
    -DHAVE_LINUX_FD_H \
    -DHAVE_TYPE_SSIZE_T \
    -DHAVE_GETOPT_H \
    -DHAVE_SYS_TIME_H \
    -DHAVE_SYS_PARAM_H \
    -DHAVE_SYSCONF

e2image_shared_libraries := \
    libext2fs_blocks \
    libext2_com_err_blocks \

e2image_system_shared_libraries := libc

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(e2image_src_files)
LOCAL_C_INCLUDES := $(e2image_c_includes)
LOCAL_CFLAGS := $(e2image_cflags)
LOCAL_STATIC_LIBRARIES := $(e2image_shared_libraries) $(e2image_system_shared_libraries)
LOCAL_MODULE := e2image_blocks
LOCAL_MODULE_TAGS := optional
LOCAL_FORCE_STATIC_EXECUTABLE := true

# Static link into ramdisk
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)


include $(BUILD_EXECUTABLE)

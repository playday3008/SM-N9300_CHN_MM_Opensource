LOCAL_PATH:= $(call my-dir)

#########################

include $(CLEAR_VARS)
LOCAL_SRC_FILES :=  bpf.c cache.c dbus.c dhcp.c dnsmasq.c \
                    forward.c helper.c lease.c log.c \
                    netlink.c network.c option.c rfc1035.c \
		    rfc2131.c tftp.c util.c conntrack.c \
		    dhcp6.c rfc3315.c dhcp-common.c outpacket.c \
		    route.c radv.c slaac.c ipv6_forward.c

LOCAL_MODULE := dnsmasq

LOCAL_C_INCLUDES := external/dnsmasq/src \
		system/core/include/netutils

LOCAL_CFLAGS := -O2 -g -W -Wall -D__ANDROID__ -DNO_TFTP -DNO_SCRIPT
LOCAL_CFLAGS += -Wno-unused -Wno-pointer-arith -Wno-type-limits
LOCAL_SYSTEM_SHARED_LIBRARIES := libc libcutils libnetutils

ifeq ($(BUILD_RIL_DISABLE_TETHERINGIPV6),true)
LOCAL_CFLAGS += -DNO_IPV6
endif

#LOCAL_LDLIBS := lpthread

include $(BUILD_EXECUTABLE)


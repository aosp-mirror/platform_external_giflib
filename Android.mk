LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	dgif_lib.c \
	gifalloc.c \
	gif_err.c  \
	openbsd-reallocarray.c

LOCAL_CFLAGS += -Wno-format -DHAVE_CONFIG_H
LOCAL_SDK_VERSION := 8
LOCAL_MODULE:= libgif

include $(BUILD_STATIC_LIBRARY)

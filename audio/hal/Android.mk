LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
	audio_hw.c \
        dsp/biquad.c \
        dsp/crossover.c \
        dsp/crossover2.c \
        dsp/drc.c \
        dsp/drc_kernel.c \
        dsp/drc_math.c \
        dsp/dsp_util.c \
        dsp/eq2.c \
        dsp/eq.c

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libcutils \
	libaudioutils \
	libtinyalsa \
	libtinycompress \
	libaudioroute \
	libdl


LOCAL_C_INCLUDES += \
	external/tinyalsa/include \
	external/tinycompress/include \
	$(call include-path-for, audio-utils) \
	$(call include-path-for, audio-route) \
	$(call include-path-for, audio-effects)


LOCAL_MODULE := audio.primary.dragon

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

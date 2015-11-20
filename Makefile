# use pkg-config for getting CFLAGS and LDLIBS
FFMPEG_LIBS=    libavdevice                        \
                libavformat                        \
                libavfilter                        \
                libavcodec                         \
                libswresample                      \
                libswscale                         \
                libavutil                          \

H264_LIBS=      libh264bitstream                   \

CFLAGS += -Wall -g
CFLAGS := $(shell pkg-config --cflags $(FFMPEG_LIBS)) $(CFLAGS)
LDLIBS := $(shell pkg-config --libs $(FFMPEG_LIBS) $(H264_LIBS)) $(LDLIBS)
LDLIBS += -lm

EXAMPLES=       open                       \
		streamVid                  \
		test		           \
		frame_add_data	           \

OBJS=$(addsuffix .o,$(EXAMPLES))

.phony: all clean-test clean

all: $(OBJS) $(EXAMPLES)

clean: 
	$(RM) $(EXAMPLES) $(OBJS)

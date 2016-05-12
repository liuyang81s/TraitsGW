LIB = 

INCLUDE_DIRS = $(ROOT)/ringbuffer \
				$(ROOT)/crc \
                $(ROOT)/timerlist \
                $(ROOT)/devs

BUILD_DIR = $(BUILDS)/gw
LINK_LIBS = ringbuffer devs crc tmlist \
            json-c curl event pthread


EXE = gw/gw

include $(ROOT)/make.rule

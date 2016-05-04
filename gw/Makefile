LIB = 

INCLUDE_DIRS = $(ROOT)/ringbuffer \
				$(ROOT)/crc \
                $(ROOT)/timerlist \
                $(ROOT)/devs

BUILD_DIR = $(BUILDS)/gw
LINK_LIBS = ringbuffer crc tmlist devs \
            json-c curl event pthread


EXE = gw/gw

include $(ROOT)/make.rule

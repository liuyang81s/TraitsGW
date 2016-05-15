LIB = 

INCLUDE_DIRS = $(ROOT)/ringbuffer \
				$(ROOT)/crc \
                $(ROOT)/timerlist \
                $(ROOT)/devs \
				$(ROOT)/http

BUILD_DIR = $(BUILDS)/gw
LINK_LIBS = ringbuffer devs crc tmlist http \
            json-c curl event pthread


EXE = traitsgw

include $(ROOT)/make.rule

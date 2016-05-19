LIB = 

INCLUDE_DIRS = $(ROOT)/ringbuffer \
				$(ROOT)/crc \
                $(ROOT)/timerlist \
                $(ROOT)/devs \
				$(ROOT)/http \
				$(ROOT)/packetfilemgr \
				$(ROOT)/easylogger


BUILD_DIR = $(BUILDS)/gw
LINK_LIBS = ringbuffer devs crc tmlist http pfmgr elog \
            json-c curl event pthread


EXE = traitsgw

include $(ROOT)/make.rule

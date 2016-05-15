.PHONY: all clean touch

ROOT = $(realpath .)
BUILDS = $(ROOT)/build

DIRS = $(ROOT)/crc \
	$(ROOT)/ringbuffer \
	$(ROOT)/devs \
	$(ROOT)/timerlist \
	$(ROOT)/http \
	$(ROOT)/packetfilemgr \
	$(ROOT)/gw 
#\
	$(ROOT)/rexmit

RM = rm
RMFLAGS = -fr

all:
	@set -e; \
	for dir in $(DIRS); \
	do \
		cd $$dir && $(MAKE) -r ROOT=$(ROOT) BUILDS=$(BUILDS) $@; \
	done
	@echo ""
	@echo ":-) Completed"
	@echo ""

clean:
	$(RM) $(RMFLAGS) $(BUILDS)
	@echo ""
	@echo ":-) cleaned"
	@echo ""
	
touch:
	@echo "Processing ..."
	@find $(ROOT) -exec touch {} \;
	@echo ""
	@echo ":-) Completed"
	@echo ""


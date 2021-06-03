include ../base.mk

TARGETS = dstcli${exe} dstsrv${exe}

all: ${TARGETS}

install:

include ../conf.mk

clean:
	@echo ">> Clean all <<"; \
	for i in *.o ${TARGETS}; do [ -f $$i ] && { \
		echo "    removing $$i"; rm $$i; \
	}; done; \
	for i in *.profile; do [ -d $$i ] && { \
		echo "    removing $$i"; rm -rf $$i; \
	}; done; true

%.o: %.c
	@$(call compile,$<)

dstcli${exe}: dstcli.o
	@$(call build_exe,,-lpthread)

dstsrv${exe}: dstsrv.o cmd.o
	@$(call build_exe,,cmd.o -lpthread)

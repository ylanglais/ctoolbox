include ../base.mk

TARGETS = flen${exe}

all: ${TARGETS}

clean:
	@echo ">> Clean all <<"; \
	for i in *.o ${TARGETS}; do [ -f $$i ] && { \
		echo "    removing $$i"; rm $$i; \
	}; done; \
	for i in *.profile; do [ -d $$i ] && { \
		echo "    removing $$i"; rm -rf $$i; \
	}; done; true

include ../conf.mk

.c.o:
%.o: %.c 
	@$(call compile,$<)

flen${exe}: flen.o
	@$(call build_exe)


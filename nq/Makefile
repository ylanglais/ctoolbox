include ../base.mk

TARGETS = nqs${exe} nqget${exe} nqput${exe} nq_print${exe}

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

NQS_OBJ = qe.o nq.o msg.o nqc.o
nqs${exe}: nqs.o ${NQS_OBJ} 
	@$(call build_exe,,${NQS_OBJ} -ltbx)

NQPRN_OBJ =  qe.o nq.o 
nq_print${exe}: nq_print.o ${NQPUT_OBJ} 
	@$(call build_exe,,${NQPRN_OBJ} -ltbx)

NQPUT_OBJ = nqc.o msg.o
nqput${exe}: nqput.o ${NQPUT_OBJ} 

	@$(call build_exe,,${NQS_OBJ} -ltbx)

nqget${exe}: nqput${exe}
	@[ -L % ] || ln -s nqput${exe} nqget${exe}


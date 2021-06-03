include ../base.mk

TARGETS = watch${exe} chrono${exe} countdown${exe} crc32${exe} grep${exe} egrep${exe} fibonacci${exe} getaddrinfo${exe} gethostbyname${exe} gethostbyaddr${exe} myelf32${exe} myelf64${exe}

all: ${TARGETS}

install:

test:

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

watch${exe}: watch.o
	@$(call build_exe)

chrono${exe}: chrono.o
	@$(call build_exe)

countdown${exe}: countdown.o
	@$(call build_exe)

crc32${exe}: crc32.o
	@$(call build_exe)

fibonacci${exe}: fibonacci.o
	@$(call build_exe)

grep${exe}: grep.o
	@$(call build_exe)

egrep${exe}: grep${exe}
	@echo -n ">>> build ${bold}$@${norm}..."; \
	ln -s grep egrep; \
	$(call status)

getaddrinfo${exe}: getaddrinfo.o
	@$(call build_exe)

gethostbyname${exe}: gethostbyname.o
	@$(call build_exe)

gethostbyaddr${exe}: gethostbyaddr.o
	@$(call build_exe)

myelf32${exe}: myelf32.o
	@$(call build_exe,,-lelf)

myelf64${exe}: myelf64.o
	@$(call build_exe,,-lelf)

DIRS = tbx db gdb geom xml dlspy mem smpp cli

all:
	@$(call sub,all)

%:  
	@$(call sub,$@)

sub = for i in ${DIRS} ; \
	do \
		( cd $$i && { \
			echo "*** Entering directory ${bold}${blue}$$i${norm} ***"; \
			make ${1}; \
		} );  \
	done


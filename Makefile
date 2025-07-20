SHELL=/bin/bash
FLTK_CONFIG=/usr/local/src/fltk-1.5.x.git/build/fltk-config

punchtape: punchtape.cxx
	$(FLTK_CONFIG) --compile punchtape.cxx

clean:
	if [[ -e punchtape ]]; then /bin/rm punchtape; fi

# make sure all subdirs get a makefile....
dirs = doubleenter movingaverage envelope quantize gap delay averagefft \
 amplifyfree distort stringenter rate freqmult amptolabel labeltype fadein \
 formant newsignal time about sonagram mix saveblock savelabel playback \
 memory pitch filter fadeout 

install clean distclean all:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	echo linking Makefiles
	$(foreach dir,$(dirs),$(shell ln -sf ../template/Makefile $(dir)))
	list='$(dirs)'; for subdir in $$list; do \
	target=`echo $@ | sed s/-recursive//`; \
	echo "Making $$target in $$subdir"; \
	(cd $$subdir && $(MAKE) $$target) \
	|| case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
    



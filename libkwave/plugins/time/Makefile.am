# make sure all subdirs get a makefile....
dirs = envelope noise zero flip center reverse distort amplify amplifyfree \
amplifytomax fadein fadeout quantize average filter delay

clean distclean all install:
	echo linking Makefiles
	$(foreach dir,$(dirs),$(shell ln -sf ../template/Makefile $(dir)))
	list='$(dirs)'; for subdir in $$list; do \
	target=`echo $@ | sed s/-recursive//`; \
	echo "Making $$target in $$subdir"; \
	(cd $$subdir && $(MAKE) $$target) \
	|| case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
    



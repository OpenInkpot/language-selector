SUBDIRS = src

bin_SCRIPTS = setlocale

xsession_dir=/etc/X11/Xsession.d

install-data-hook:
	$(mkdir_p) $(DESTDIR)$(xsession_dir)
	$(INSTALL) 10language-selector $(DESTDIR)$(xsession_dir)/10language-selector

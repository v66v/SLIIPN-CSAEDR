CC = gcc
PKGCONFIG = pkg-config
CFLAGS = -O3 $(shell $(PKGCONFIG) --cflags gtk+-3.0) -Wall -Wextra
LIBS = $(shell $(PKGCONFIG) --libs gtk+-3.0)
GLIB_COMPILE_RESOURCES = $(shell $(PKGCONFIG) --variable=glib_compile_resources \
											  gio-2.0)
INC_DIRS = ../
INCS := $(addprefix -I,$(INC_DIRS))

SRC = gtk.c gtkwin.c gtk-main.c shm.c
BUILT_SRC = resources.c
DESTDIR ?= $(CURDIR)

OBJS = $(BUILT_SRC:.c=.o) $(SRC:.c=.o)
BIN = $(addprefix $(DESTDIR)/bin/, gtk)
all: $(BIN)

resources.c: gtk.gresource.xml window.ui
	@$(GLIB_COMPILE_RESOURCES) gtk.gresource.xml --target=$@ \
							   --sourcedir=. --generate-source

shm.o: ../shm/shm.c makefile
	@$(CC) -c -o shm.o $(CFLAGS) $(INCS) $<

%.o: %.c makefile
	@$(CC) -c -o $(@F) $(CFLAGS) $(INCS) $<

$(DESTDIR)/bin/gtk: $(OBJS) $(DESTDIR)/bin/ makefile
	$(info [CC] Linking object files: $@)
	@$(CC) -o $(DESTDIR)/bin/gtk $(OBJS) $(LIBS)

.PRECIOUS: $(DESTDIR)/. $(DESTDIR)/%/.
$(DESTDIR)/:
	$(info [INFO] Creating directory $@)
	@mkdir -p $@

$(DESTDIR)/%/:
	$(info [INFO] Creating directory $@)
	@mkdir -p $@


.PHONY: clean
clean:
	@rm -f $(BUILT_SRC)
	@rm -f $(OBJS)
	@rm -f gtk
	@rm -f  $(DESTDIR)/bin/gtk

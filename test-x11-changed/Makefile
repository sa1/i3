# Default value so one can compile test-x11-changed standalone
TOPDIR=..

include $(TOPDIR)/common.mk

CPPFLAGS += -I$(TOPDIR)/include

CFLAGS += $(call cflags_for_lib, imlib2)
LIBS += $(call ldflags_for_lib, imlib2,imlib2)

# Depend on the object files of all source-files in src/*.c and on all header files
FILES=$(patsubst %.c,%.o,$(wildcard *.c))
HEADERS=$(wildcard *.h)

# Depend on the specific file (.c for each .o) and on all headers
%.o: %.c ${HEADERS}
	echo "[test-x11-changed] CC $<"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

all: test-x11-changed

test-x11-changed: $(TOPDIR)/libi3/libi3.a ${FILES}
	echo "[test-x11-changed] LINK test-x11-changed"
	$(CC) $(LDFLAGS) -o $@ $(filter-out libi3/libi3.a,$^) $(LIBS)

$(TOPDIR)/libi3/%.a: $(TOPDIR)/libi3/*.c
	$(MAKE) -C $(TOPDIR)/libi3

install: all
	echo "[test-x11-changed] INSTALL"
	$(INSTALL) -d -m 0755 $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 test-x11-changed $(DESTDIR)$(PREFIX)/bin/

clean:
	rm -f *.o

distclean: clean
	rm -f test-x11-changed

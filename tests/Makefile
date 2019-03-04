TESTS=utf log

all: $(TESTS)

clean:
	rm -f $(LIBCOMMON_OBJS) $(LIBCOMMON) $(TESTS)

LIBCOMMON_ROOT=../
include ../Makefile.inc

CFLAGS+=-Wall
CFLAGS+=$(LIBCOMMON_CFLAGS)

utf: utf.c $(LIBCOMMON)
	$(CC) $(CFLAGS) -o $@ utf.c $(LIBCOMMON)

log: log.c $(LIBCOMMON)
	$(CC) $(CFLAGS) -o $@ log.c $(LIBCOMMON)

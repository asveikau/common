TESTS=utf log fsinfo

all: $(TESTS)

clean:
	rm -f $(LIBCOMMON_OBJS) $(LIBCOMMON) $(TESTS)

LIBCOMMON_ROOT=../
include ../Makefile.inc

CFLAGS+=-Wall
CFLAGS+=$(LIBCOMMON_CFLAGS)

fsinfo: fsinfo.c $(LIBCOMMON)
	$(CC) $(CFLAGS) -o $@ fsinfo.c $(LIBCOMMON)

utf: utf.c $(LIBCOMMON)
	$(CC) $(CFLAGS) -o $@ utf.c $(LIBCOMMON)

log: log.c $(LIBCOMMON)
	$(CC) $(CFLAGS) -o $@ log.c $(LIBCOMMON)

.PHONY: all-phony
all-phony: all

LIBCOMMON_ROOT=../
include ../Makefile.inc

TESTS=append-path$(EXESUFFIX) utf$(EXESUFFIX) log$(EXESUFFIX)

ifndef WINDOWS
TESTS+=fsinfo$(EXESUFFIX)
endif

all: $(TESTS)

clean:
	rm -f $(LIBCOMMON_OBJS) $(LIBCOMMON) $(TESTS)

CFLAGS+=-Wall
CFLAGS+=$(LIBCOMMON_CFLAGS)

append-path$(EXESUFFIX): append-path.c $(LIBCOMMON)
	$(CC) $(CFLAGS) -o $@ append-path.c $(LIBCOMMON)

fsinfo$(EXESUFFIX): fsinfo.c $(LIBCOMMON)
	$(CC) $(CFLAGS) -o $@ fsinfo.c $(LIBCOMMON)

utf$(EXESUFFIX): utf.c $(LIBCOMMON)
	$(CC) $(CFLAGS) -o $@ utf.c $(LIBCOMMON)

log$(EXESUFFIX): log.c $(LIBCOMMON)
	$(CC) $(CFLAGS) -o $@ log.c $(LIBCOMMON)

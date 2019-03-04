.PHONY: all all-phony clean depend
all: all-phony

WINDOWS_SUBSYSTEM=console
include Makefile.inc

# XXX Windows has problems with this doing parallel builds.
# With cl, `cc -o foo.exe foo.c` will generate an implicit foo.obj
# which does not play nice with make -j and multiple targets needing foo.c.
#
ifdef UNDER_CI
DISABLE_TESTS:=y
endif

ifndef DISABLE_TESTS
TESTS=md5$(EXESUFFIX) sha1$(EXESUFFIX) sha256$(EXESUFFIX)
endif

all-phony: $(LIBCOMMON) $(TESTS)

clean:
	rm -f $(LIBCOMMON) $(LIBCOMMON_OBJS) $(TESTS)


# Test binaries
#
md5$(EXESUFFIX): src/crypto/test/hash.c $(LIBCOMMON) $(XP_SUPPORT_OBJS)
	${CC} -Iinclude -DHASH_ALGORITHM=md5 -o $@ $< -L. -lcommon $(LDFLAGS)
sha1$(EXESUFFIX): src/crypto/test/hash.c $(LIBCOMMON) $(XP_SUPPORT_OBJS)
	${CC} -Iinclude -DHASH_ALGORITHM=sha1 -o $@ $< -L. -lcommon $(LDFLAGS)
sha256$(EXESUFFIX): src/crypto/test/hash.c $(LIBCOMMON) $(XP_SUPPORT_OBSJ)
	${CC} -Iinclude -DHASH_ALGORITHM=sha256 -o $@ $< -L. -lcommon $(LDFLAGS)

export
depend:
	env PROJECT=LIBCOMMON $(DEPEND) \
	src/*.c src/*.cc src/*.m src/crypto/*.c \
		> depend.mk

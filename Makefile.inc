ifndef LIBCOMMON_INC
LIBCOMMON_INC:=y

MAKEFILES_ROOT?=$(LIBCOMMON_ROOT)submodules/makefiles/
-include $(MAKEFILES_ROOT)Makefile.inc

LIBCOMMON_SRC:=\
   $(LIBCOMMON_ROOT)src/appdata.c \
   $(LIBCOMMON_ROOT)src/backtrace.c \
   $(LIBCOMMON_ROOT)src/buffer.c \
   $(LIBCOMMON_ROOT)src/bundle.c \
   $(LIBCOMMON_ROOT)src/crashlog.c \
   $(LIBCOMMON_ROOT)src/closefrom.c \
   $(LIBCOMMON_ROOT)src/copy.c \
   $(LIBCOMMON_ROOT)src/error.c \
   $(LIBCOMMON_ROOT)src/error-libc.c \
   $(LIBCOMMON_ROOT)src/lazy.c \
   $(LIBCOMMON_ROOT)src/logcallback.c \
   $(LIBCOMMON_ROOT)src/logger.c \
   $(LIBCOMMON_ROOT)src/mutex.c \
   $(LIBCOMMON_ROOT)src/monotonic.c \
   $(LIBCOMMON_ROOT)src/path.c \
   $(LIBCOMMON_ROOT)src/progname.c \
   $(LIBCOMMON_ROOT)src/recmutex.c \
   $(LIBCOMMON_ROOT)src/refcnt.c \
   $(LIBCOMMON_ROOT)src/remotepath.c \
   $(LIBCOMMON_ROOT)src/rwlock.c \
   $(LIBCOMMON_ROOT)src/rwlock-self.c \
   $(LIBCOMMON_ROOT)src/sem.c \
   $(LIBCOMMON_ROOT)src/size.c \
   $(LIBCOMMON_ROOT)src/thread.c \
   $(LIBCOMMON_ROOT)src/time.c \
   $(LIBCOMMON_ROOT)src/trie.c \
   $(LIBCOMMON_ROOT)src/utf16dec.c \
   $(LIBCOMMON_ROOT)src/utf16enc.c \
   $(LIBCOMMON_ROOT)src/utf8common.c \
   $(LIBCOMMON_ROOT)src/utf8dec.c \
   $(LIBCOMMON_ROOT)src/utf8enc.c \
   $(LIBCOMMON_ROOT)src/waiter.c \
   \
   $(LIBCOMMON_ROOT)src/dtorqueue.cc \
   $(LIBCOMMON_ROOT)src/pstream.cc \
   $(LIBCOMMON_ROOT)src/stream.cc \
   $(LIBCOMMON_ROOT)src/linereader.cc \
   $(LIBCOMMON_ROOT)src/memorystream.cc \
   $(LIBCOMMON_ROOT)src/scheduler.cc \
   $(LIBCOMMON_ROOT)src/thread-cpp.cc \
   $(LIBCOMMON_ROOT)src/refcnt-cpp.cc \
   $(LIBCOMMON_ROOT)src/worker.cc \
   \
   $(LIBCOMMON_ROOT)src/crypto/hash.c \
   $(LIBCOMMON_ROOT)src/crypto/sha1.c \
   $(LIBCOMMON_ROOT)src/crypto/sha256.c \
   $(LIBCOMMON_ROOT)src/crypto/md5.c \
   $(LIBCOMMON_ROOT)src/crypto/rng.c \

ifeq ($(PLATFORM),darwin)
LIBCOMMON_SRC+=\
   $(LIBCOMMON_ROOT)src/bundle-apple.m \
   $(LIBCOMMON_ROOT)src/error-apple.m

LDFLAGS+=-framework Foundation
endif

ifdef WINDOWS
LIBCOMMON_SRC+=\
   $(LIBCOMMON_ROOT)src/error-windows.c \
   $(LIBCOMMON_ROOT)src/error-winnt.c \
   $(LIBCOMMON_ROOT)src/winutf.c \
   $(LIBCOMMON_ROOT)src/winuname.c \
   $(LIBCOMMON_ROOT)src/asprintf.c
USE_BSD_GETOPT:=y
else
LIBCOMMON_SRC+=\
   $(LIBCOMMON_ROOT)src/exec.c
endif

ifdef USE_BSD_GETOPT
BSD_GETOPT_DESCR:=Berkeley getopt(3)
LIB_DESCRS+=BSD_GETOPT_DESCR
LIBCOMMON_SRC+=\
   $(LIBCOMMON_ROOT)src/getopt.c
endif

ifeq ($(PLATFORM), linux)
LDFLAGS+=-ldl
endif

ifneq (, $(filter $(PLATFORM), freebsd openbsd))
LDFLAGS+=-lexecinfo
endif

LIBCOMMON_OBJS := $(shell $(SRC2OBJ) $(LIBCOMMON_SRC))

LIBCOMMON:=$(LIBCOMMON_ROOT)$(LIBPREFIX)common$(LIBSUFFIX)

$(LIBCOMMON): $(LIBCOMMON_OBJS)
	$(LIBWRAPPER) $(LIBCOMMON_OBJS)

LIBCOMMON_CFLAGS+=-I$(LIBCOMMON_ROOT)include
LIBCOMMON_CFLAGS+=-Wall
LIBCOMMON_CXXFLAGS+=${LIBCOMMON_CFLAGS}

-include $(LIBCOMMON_ROOT)depend.mk
-include $(LIBCOMMON_ROOT)depend.$(PLATFORM).mk

endif

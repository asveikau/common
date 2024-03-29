# This file was generated by "make depend".
#

$(LIBCOMMON_ROOT)src/appdata.o: $(LIBCOMMON_ROOT)src/appdata.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/logger.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/path.h $(LIBCOMMON_ROOT)include/common/size.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/asprintf.o: $(LIBCOMMON_ROOT)src/asprintf.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/backtrace.o: $(LIBCOMMON_ROOT)src/backtrace.c $(LIBCOMMON_ROOT)include/common/backtrace.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/buffer.o: $(LIBCOMMON_ROOT)src/buffer.c $(LIBCOMMON_ROOT)include/common/buffer.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/size.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/bundle.o: $(LIBCOMMON_ROOT)src/bundle.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/lazy.h $(LIBCOMMON_ROOT)include/common/path.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/closefrom.o: $(LIBCOMMON_ROOT)src/closefrom.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/copy.o: $(LIBCOMMON_ROOT)src/copy.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/path.h $(LIBCOMMON_ROOT)include/common/size.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/crashlog.o: $(LIBCOMMON_ROOT)src/crashlog.c $(LIBCOMMON_ROOT)include/common/backtrace.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/logger.h $(LIBCOMMON_ROOT)include/common/misc.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/error-libc.o: $(LIBCOMMON_ROOT)src/error-libc.c $(LIBCOMMON_ROOT)include/common/error.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/error-windows.o: $(LIBCOMMON_ROOT)src/error-windows.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/size.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/error-winnt.o: $(LIBCOMMON_ROOT)src/error-winnt.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/lazy.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/error.o: $(LIBCOMMON_ROOT)src/error.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/logger.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/exec.o: $(LIBCOMMON_ROOT)src/exec.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/exec.h $(LIBCOMMON_ROOT)include/common/misc.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/getopt.o: $(LIBCOMMON_ROOT)src/getopt.c $(LIBCOMMON_ROOT)include/common/getopt.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/lazy.o: $(LIBCOMMON_ROOT)src/lazy.c $(LIBCOMMON_ROOT)include/common/cas.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/lazy.h $(LIBCOMMON_ROOT)include/common/spin.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/logcallback.o: $(LIBCOMMON_ROOT)src/logcallback.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/logger.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/mutex.h $(LIBCOMMON_ROOT)include/common/path.h $(LIBCOMMON_ROOT)include/common/thread.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/logger.o: $(LIBCOMMON_ROOT)src/logger.c $(LIBCOMMON_ROOT)include/common/buffer.h $(LIBCOMMON_ROOT)include/common/logger.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/monotonic.o: $(LIBCOMMON_ROOT)src/monotonic.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/lazy.h $(LIBCOMMON_ROOT)include/common/time.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/mutex.o: $(LIBCOMMON_ROOT)src/mutex.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/mutex.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/path.o: $(LIBCOMMON_ROOT)src/path.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/path.h $(LIBCOMMON_ROOT)include/common/size.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/progname.o: $(LIBCOMMON_ROOT)src/progname.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/lazy.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/path.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/recmutex.o: $(LIBCOMMON_ROOT)src/recmutex.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/mutex.h $(LIBCOMMON_ROOT)include/common/recmutex.h $(LIBCOMMON_ROOT)include/common/sem.h $(LIBCOMMON_ROOT)include/common/waiter.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/refcnt.o: $(LIBCOMMON_ROOT)src/refcnt.c $(LIBCOMMON_ROOT)include/common/cas.h $(LIBCOMMON_ROOT)include/common/refcnt.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/remotepath.o: $(LIBCOMMON_ROOT)src/remotepath.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/path.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/rwlock-self.o: $(LIBCOMMON_ROOT)src/rwlock-self.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/mutex.h $(LIBCOMMON_ROOT)include/common/rwlock-self.h $(LIBCOMMON_ROOT)include/common/waiter.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/rwlock.o: $(LIBCOMMON_ROOT)src/rwlock.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/mutex.h $(LIBCOMMON_ROOT)include/common/rwlock-self.h $(LIBCOMMON_ROOT)include/common/rwlock.h $(LIBCOMMON_ROOT)include/common/waiter.h $(LIBCOMMON_ROOT)src/rwlock-xpsup.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/sem.o: $(LIBCOMMON_ROOT)src/sem.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/mutex.h $(LIBCOMMON_ROOT)include/common/sem.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/size.o: $(LIBCOMMON_ROOT)src/size.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/size.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/thread.o: $(LIBCOMMON_ROOT)src/thread.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/mutex.h $(LIBCOMMON_ROOT)include/common/thread.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/time.o: $(LIBCOMMON_ROOT)src/time.c $(LIBCOMMON_ROOT)include/common/time.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/trie.o: $(LIBCOMMON_ROOT)src/trie.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/trie.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/utf16dec.o: $(LIBCOMMON_ROOT)src/utf16dec.c $(LIBCOMMON_ROOT)include/common/utf.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/utf16enc.o: $(LIBCOMMON_ROOT)src/utf16enc.c $(LIBCOMMON_ROOT)include/common/utf.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/utf8common.o: $(LIBCOMMON_ROOT)src/utf8common.c
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/utf8dec.o: $(LIBCOMMON_ROOT)src/utf8dec.c $(LIBCOMMON_ROOT)include/common/utf.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/utf8enc.o: $(LIBCOMMON_ROOT)src/utf8enc.c $(LIBCOMMON_ROOT)include/common/utf.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/waiter.o: $(LIBCOMMON_ROOT)src/waiter.c $(LIBCOMMON_ROOT)include/common/cas.h $(LIBCOMMON_ROOT)include/common/spin.h $(LIBCOMMON_ROOT)include/common/waiter.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/winuname.o: $(LIBCOMMON_ROOT)src/winuname.c $(LIBCOMMON_ROOT)include/common/uname.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/winutf.o: $(LIBCOMMON_ROOT)src/winutf.c $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/dtorqueue.o: $(LIBCOMMON_ROOT)src/dtorqueue.cc $(LIBCOMMON_ROOT)include/common/c++/dtorqueue.h $(LIBCOMMON_ROOT)include/common/error.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LIBCOMMON_CXXFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CXXFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/linereader.o: $(LIBCOMMON_ROOT)src/linereader.cc $(LIBCOMMON_ROOT)include/common/c++/linereader.h $(LIBCOMMON_ROOT)include/common/c++/refcount.h $(LIBCOMMON_ROOT)include/common/c++/stream.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/refcnt.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LIBCOMMON_CXXFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CXXFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/memorystream.o: $(LIBCOMMON_ROOT)src/memorystream.cc $(LIBCOMMON_ROOT)include/common/c++/lock.h $(LIBCOMMON_ROOT)include/common/c++/new.h $(LIBCOMMON_ROOT)include/common/c++/refcount.h $(LIBCOMMON_ROOT)include/common/c++/stream.h $(LIBCOMMON_ROOT)include/common/cas.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/mutex.h $(LIBCOMMON_ROOT)include/common/refcnt.h $(LIBCOMMON_ROOT)include/common/rwlock-self.h $(LIBCOMMON_ROOT)include/common/rwlock.h $(LIBCOMMON_ROOT)include/common/thread.h $(LIBCOMMON_ROOT)include/common/waiter.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LIBCOMMON_CXXFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CXXFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/pstream.o: $(LIBCOMMON_ROOT)src/pstream.cc $(LIBCOMMON_ROOT)include/common/c++/new.h $(LIBCOMMON_ROOT)include/common/c++/refcount.h $(LIBCOMMON_ROOT)include/common/c++/stream.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/path.h $(LIBCOMMON_ROOT)include/common/refcnt.h $(LIBCOMMON_ROOT)include/common/size.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LIBCOMMON_CXXFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CXXFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/refcnt-cpp.o: $(LIBCOMMON_ROOT)src/refcnt-cpp.cc $(LIBCOMMON_ROOT)include/common/c++/lock.h $(LIBCOMMON_ROOT)include/common/c++/refcount.h $(LIBCOMMON_ROOT)include/common/cas.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/mutex.h $(LIBCOMMON_ROOT)include/common/refcnt.h $(LIBCOMMON_ROOT)include/common/rwlock-self.h $(LIBCOMMON_ROOT)include/common/rwlock.h $(LIBCOMMON_ROOT)include/common/thread.h $(LIBCOMMON_ROOT)include/common/waiter.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LIBCOMMON_CXXFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CXXFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/scheduler.o: $(LIBCOMMON_ROOT)src/scheduler.cc $(LIBCOMMON_ROOT)include/common/c++/scheduler.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/mutex.h $(LIBCOMMON_ROOT)include/common/sem.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LIBCOMMON_CXXFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CXXFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/stream.o: $(LIBCOMMON_ROOT)src/stream.cc $(LIBCOMMON_ROOT)include/common/c++/new.h $(LIBCOMMON_ROOT)include/common/c++/refcount.h $(LIBCOMMON_ROOT)include/common/c++/stream.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/path.h $(LIBCOMMON_ROOT)include/common/refcnt.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LIBCOMMON_CXXFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CXXFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/thread-cpp.o: $(LIBCOMMON_ROOT)src/thread-cpp.cc $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/mutex.h $(LIBCOMMON_ROOT)include/common/thread.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LIBCOMMON_CXXFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CXXFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/worker.o: $(LIBCOMMON_ROOT)src/worker.cc $(LIBCOMMON_ROOT)include/common/c++/lock.h $(LIBCOMMON_ROOT)include/common/c++/ring.h $(LIBCOMMON_ROOT)include/common/c++/scheduler.h $(LIBCOMMON_ROOT)include/common/c++/worker.h $(LIBCOMMON_ROOT)include/common/cas.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h $(LIBCOMMON_ROOT)include/common/mutex.h $(LIBCOMMON_ROOT)include/common/rwlock-self.h $(LIBCOMMON_ROOT)include/common/rwlock.h $(LIBCOMMON_ROOT)include/common/sem.h $(LIBCOMMON_ROOT)include/common/thread.h $(LIBCOMMON_ROOT)include/common/waiter.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) $(LIBCOMMON_CXXFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CXXFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/bundle-apple.o: $(LIBCOMMON_ROOT)src/bundle-apple.m
	$(CC) $(OBJCFLAGS) $(CFLAGS) $(LIBCOMMON_OBJCFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_OBJCFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/error-apple.o: $(LIBCOMMON_ROOT)src/error-apple.m $(LIBCOMMON_ROOT)include/common/error.h
	$(CC) $(OBJCFLAGS) $(CFLAGS) $(LIBCOMMON_OBJCFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_OBJCFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/crypto/hash.o: $(LIBCOMMON_ROOT)src/crypto/hash.c $(LIBCOMMON_ROOT)include/common/crypto/hash.h $(LIBCOMMON_ROOT)include/common/crypto/misc.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/crypto/md5.o: $(LIBCOMMON_ROOT)src/crypto/md5.c $(LIBCOMMON_ROOT)include/common/crypto/hash.h $(LIBCOMMON_ROOT)include/common/crypto/md5.h $(LIBCOMMON_ROOT)include/common/crypto/misc.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/crypto/rng.o: $(LIBCOMMON_ROOT)src/crypto/rng.c $(LIBCOMMON_ROOT)include/common/crypto/rng.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/lazy.h $(LIBCOMMON_ROOT)include/common/misc.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/crypto/sha1.o: $(LIBCOMMON_ROOT)src/crypto/sha1.c $(LIBCOMMON_ROOT)include/common/crypto/hash.h $(LIBCOMMON_ROOT)include/common/crypto/misc.h $(LIBCOMMON_ROOT)include/common/crypto/sha1.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<
$(LIBCOMMON_ROOT)src/crypto/sha256.o: $(LIBCOMMON_ROOT)src/crypto/sha256.c $(LIBCOMMON_ROOT)include/common/crypto/hash.h $(LIBCOMMON_ROOT)include/common/crypto/misc.h $(LIBCOMMON_ROOT)include/common/crypto/sha256.h $(LIBCOMMON_ROOT)include/common/error.h $(LIBCOMMON_ROOT)include/common/misc.h
	$(CC) $(CFLAGS) $(LIBCOMMON_CFLAGS) $(LATE_CFLAGS) -c -o $@ $<

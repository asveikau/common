# libcommon

This repo contains a grab bag of helpful utility functions and structures for C and C++.

Notable areas:

* Error handling and logging
* Synchronization primitives that wrap various platform-specific APIs
    - Thread safe lazy initialization
    - Mutex and semaphore
    - Thread creation
    - Compare and swap
    - Memory barrier
* Misc. filesystem helpers
    - Directory enumeration (wraps dirent or FindFirstFile)
    - App-local directory (FolderPath on Windows, ~/.config/... on *nix)
* C++ stream class
* C++ worker thread class

## Building

Building happens via [the makefiles submodule][1].

    $ git submodule update --init
    $ make                             # or "gmake" on some platforms, like BSD

On Windows, GNU make, nasm and msysgit should be on PATH, and the project is
typically tested with VS2015 with Windows SDK 10586.

[1]: https://github.com/asveikau/makefiles

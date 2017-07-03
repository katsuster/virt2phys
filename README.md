## virt2phys
A tool of translating virtual address of user space to physical address.

#### Requires
* Linux
* autotools (autoconf, automake)


## How to use
Get source code and configure.

    $ git clone https://github.com/katsuster/virt2phys
    $ cd virt2phys
    $ autoreconf --force --install
    $ ./configure

And build.

    $ make

Executable image 'v2p' is created and located on the ./src directory.


#### Run
For example translate the top of heap area address used by cat.

    $ cat
    [Press Ctrl+Z]
    [1]+  Stopped                 cat

NOTE: Do not press the Ctrl+C.

    $ ps
      PID TTY          TIME CMD
     2148 pts/4    00:00:00 bash
    14010 pts/4    00:00:00 cat  # Remember this process ID
    14015 pts/4    00:00:00 ps

Check the memory map of process to get the virtual address of the top of heap area.

    $ cat /proc/14010/maps
    55ff09a4f000-55ff09a57000 r-xp 00000000 08:11 5505114                    /bin/cat
    55ff09c56000-55ff09c57000 r--p 00007000 08:11 5505114                    /bin/cat
    55ff09c57000-55ff09c58000 rw-p 00008000 08:11 5505114                    /bin/cat
    55ff0b2cb000-55ff0b2ec000 rw-p 00000000 00:00 0                          [heap]    # Use this virtual address
    7fdbeb6a3000-7fdbeb838000 r-xp 00000000 08:11 4980859                    /lib/x86_64-linux-gnu/libc-2.24.so
    7fdbeb838000-7fdbeba38000 ---p 00195000 08:11 4980859                    /lib/x86_64-linux-gnu/libc-2.24.so
    ...(snip)...
    7ffdde79c000-7ffdde7be000 rw-p 00000000 00:00 0                          [stack]
    7ffdde7fb000-7ffdde7fd000 r--p 00000000 00:00 0                          [vvar]
    7ffdde7fd000-7ffdde7ff000 r-xp 00000000 00:00 0                          [vdso]

Let's translate virtual address to physical address.
NOTE: It requires super user priviledges.

    $ cd virt2phys
    $ sudo ./src/v2p 14010 0x55ff0b2cb000 0x4000
    pid: 14010:
     virt:0x55ff0b2cb000, phys:0x73a42b000
     virt:0x55ff0b2cc000, phys:(not present)
     virt:0x55ff0b2cd000, phys:(not present)
     virt:0x55ff0b2ce000, phys:(not present)

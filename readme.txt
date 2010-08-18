node-waf configure build
node ./crash.js


Crashes on line 57 in EIO_GetBufAsync function when calling Buffer::New(1024).


(gdb) r
Starting program: /usr/local/pkg/nodejs-0.1.103/bin/node ./crash.js
[Thread debugging using libthread_db enabled]
[New Thread 0xb59ecb70 (LWP 4797)]

Program received signal SIGSEGV, Segmentation fault.
[Switching to Thread 0xb59ecb70 (LWP 4797)]
node::Buffer::New (size=1024) at ../src/node_buffer.cc:152
152       return ObjectWrap::Unwrap<Buffer>(b);
(gdb) bt
#0  node::Buffer::New (size=1024) at ../src/node_buffer.cc:152
#1  0xb59f149a in RetBuf::EIO_GetBufAsync (req=0x845e360) at ../retbuf-async.cpp:57
#2  0x08118689 in eio_execute (thr_arg=0x845e3c8) at ../deps/libeio/eio.c:1679
#3  etp_proc (thr_arg=0x845e3c8) at ../deps/libeio/eio.c:1513
#4  0xb7ce0585 in start_thread (arg=0xb59ecb70) at pthread_create.c:300
#5  0xb7c6229e in clone () at ../sysdeps/unix/sysv/linux/i386/clone.S:130



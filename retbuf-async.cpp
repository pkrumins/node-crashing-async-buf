#include <cstdlib>
#include <node.h>
#include <node_buffer.h>

using namespace v8;
using namespace node;

Handle<Value>
ErrorException(const char *msg)
{
    HandleScope scope;
    return Exception::Error(String::New(msg));
}

Handle<Value>
VException(const char *msg) {
    HandleScope scope;
    return ThrowException(ErrorException(msg));
}

class RetBuf;

struct async_request {
    Persistent<Function> callback;
    Buffer *buf;
    RetBuf *obj;
};

class RetBuf : public ObjectWrap {
public:
    RetBuf() {}

    static void
    Initialize(Handle<Object> target) {
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(New);
        t->InstanceTemplate()->SetInternalFieldCount(1);
        NODE_SET_PROTOTYPE_METHOD(t, "getBuf", GetBuf);
        NODE_SET_PROTOTYPE_METHOD(t, "getBufAsync", GetBufAsync);
        target->Set(String::NewSymbol("RetBuf"), t->GetFunction());
    }

    static Handle<Value>
    GetBuf(const Arguments &args) {
        HandleScope scope;

        Buffer *b = Buffer::New(1024);
        return scope.Close(b->handle_); // returning buffer synchronously works just fine
    }

    static int 
    EIO_GetBufAsync(eio_req *req)
    {
        async_request *async_req = (async_request *)req->data;
        // here it crashes
        async_req->buf = Buffer::New(1024);  // line 57 ---------------------------------.
                                             //                                          |
        /*                                                                               |
        (gdb) r                                                                          |
        Starting program: /usr/local/pkg/nodejs-0.1.103/bin/node ./crash.js              |
        [Thread debugging using libthread_db enabled]                                    |
        [New Thread 0xb59ecb70 (LWP 4797)]                                               |
                                                                                         |
        Program received signal SIGSEGV, Segmentation fault.                             |
        [Switching to Thread 0xb59ecb70 (LWP 4797)]                                      |
        node::Buffer::New (size=1024) at ../src/node_buffer.cc:152                       |
        152       return ObjectWrap::Unwrap<Buffer>(b);                                  |
        (gdb) bt                                                                         v
        #0  node::Buffer::New (size=1024) at ../src/node_buffer.cc:152             
        #1  0xb59f149a in RetBuf::EIO_GetBufAsync (req=0x845e360) at ../retbuf-async.cpp:57
        #2  0x08118689 in eio_execute (thr_arg=0x845e3c8) at ../deps/libeio/eio.c:1679
        #3  etp_proc (thr_arg=0x845e3c8) at ../deps/libeio/eio.c:1513
        #4  0xb7ce0585 in start_thread (arg=0xb59ecb70) at pthread_create.c:300
        #5  0xb7c6229e in clone () at ../sysdeps/unix/sysv/linux/i386/clone.S:130
        */


        return 0;
    }

    static int 
    EIO_GetBufAsyncAfter(eio_req *req)
    {
        HandleScope scope;

        ev_unref(EV_DEFAULT_UC);
        async_request *async_req = (async_request *)req->data;

        Handle<Value> argv[1];
        argv[0] = async_req->buf->handle_;

        TryCatch try_catch;

        async_req->callback->Call(Context::GetCurrent()->Global(), 1, argv);

        if (try_catch.HasCaught())
            FatalException(try_catch);

        async_req->callback.Dispose();
        async_req->obj->Unref();
        free(async_req);

        return 0;
    }

    static Handle<Value>
    GetBufAsync(const Arguments &args)
    {
        HandleScope scope;

        if (args.Length() != 1)
            return VException("One argument required - callback function.");

        if (!args[0]->IsFunction())
            return VException("First argument must be a function.");

        Local<Function> callback = Local<Function>::Cast(args[0]);
        RetBuf *rb = ObjectWrap::Unwrap<RetBuf>(args.This());

        async_request *async_req = (async_request *)malloc(sizeof(*async_req));
        if (!async_req)
            return VException("malloc in GetBufAsync::GetBufAsync failed.");

        async_req->callback = Persistent<Function>::New(callback);
        async_req->obj = rb;

        eio_custom(EIO_GetBufAsync, EIO_PRI_DEFAULT, EIO_GetBufAsyncAfter, async_req);

        ev_ref(EV_DEFAULT_UC);
        rb->Ref();

        return Undefined();
    }


    static Handle<Value>
    New(const Arguments &args) {
        HandleScope scope;

        RetBuf *rb = new RetBuf;
        rb->Wrap(args.This());
        return args.This();
    }
};

extern "C" void
init(Handle<Object> target)
{
    HandleScope scope;

    RetBuf::Initialize(target);
}


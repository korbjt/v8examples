#include <v8.h>
#include <ev.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

using namespace v8;

ev_io input_listener;

ev_async async;

void call_func(const char *func) {
    HandleScope scope(Isolate::GetCurrent());
    Local<Context> context = Isolate::GetCurrent()->GetCurrentContext();

    Local<Value> foo = context->Global()->Get(String::New(func));
    Local<Function> fooFunc = Local<Function>::New(Handle<Function>::Cast(foo));

    Handle<Value> args[0];
    fooFunc->Call(fooFunc, 0, args);
}

long thread_id() {
    pthread_t ptid = pthread_self();
    long threadId = 0;
    memcpy(&threadId, &ptid, std::min(sizeof(threadId), sizeof(ptid)));
    return threadId;
}

void timer_callback(struct ev_loop *loop, ev_timer *w, int revents) {
    //std::cout << thread_id() << " Ping!" << std::endl;
    ev_async_send(EV_DEFAULT, &async);
}

void input_callback(struct ev_loop *loop, ev_io *w, int revents) {
    char buf[1024];
    int count = read(w->fd, &buf, 1023);
    buf[count] = 0;
    call_func("foo");
}

void log(const v8::FunctionCallbackInfo<v8::Value>& info) {
    //print the argument list as strings
    for(int i = 0; i < info.Length(); i++) {
        std::cout << *String::Utf8Value(info[i]->ToString()) << " ";
    }
    std::cout << std::endl;
}

void* start_loop(void *args) {
    ev_loop((struct ev_loop*)args, EVFLAG_AUTO);
    return NULL;
}

void start_loops() {
    for(int i = 0; i < 2; i++) {
        ev_timer *timer = (ev_timer *)malloc(sizeof(ev_timer));
        pthread_t thread;
        struct ev_loop *loop = ev_loop_new(EVFLAG_AUTO);
        ev_timer_init(timer, timer_callback, (float)i, 1.0 + i);
        ev_timer_start(loop, timer);
        pthread_create(&thread, NULL, start_loop, loop);
    }
}


void wakeup_call(struct ev_loop *loop, ev_async *w, int revents) {
    std::cout << thread_id() << " Im Up!" << std::endl;
    std::cout << w->data << std::endl;
}

int main(int argc, char **argv) {
    struct ev_loop *main_loop = EV_DEFAULT;
    
    start_loops();

    ev_async_init(&async, wakeup_call);
    ev_io_init(&input_listener, input_callback, 0, EV_READ);
    ev_io_start(main_loop, &input_listener);
    ev_async_start(main_loop, &async);

    Isolate *isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    
    //create an ObjectTemplate to store globals
    Handle<ObjectTemplate> global = ObjectTemplate::New();

    //Add a new funtion template to the global template
    global->Set(String::New("log"), FunctionTemplate::New(log));

    //Create the context with the global template
    Handle<Context> context = Context::New(isolate, NULL, global); 
    Context::Scope context_scope(context);
    //Any script running in this context will have access to `log`
    Handle<String> source = String::NewFromUtf8(isolate, "log('Hello', 5); function foo(){log('Hello From Foo');}");
    Handle<Script> script = Script::Compile(source);
    script->Run();

    call_func("foo");
    ev_run(main_loop, 0);
    return 0;
}

#include <v8.h>
#include <iostream>

using namespace v8;

void log(const v8::FunctionCallbackInfo<v8::Value>& info) {
    //print the argument list as strings
    for(int i = 0; i < info.Length(); i++) {
        std::cout << *String::Utf8Value(info[i]->ToString()) << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char **argv) {
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
    Handle<String> source = String::NewFromUtf8(isolate, "log('Hello', 5);");
    Handle<Script> script = Script::Compile(source);
    script->Run();

    return 0;
}

#include <v8.h>
#include <iostream>

int main(int argc, char **argv) {
    //Get the default isolate
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    
    //Create a new handle scope
    v8::HandleScope scope(isolate);

    //Create a context 
    v8::Handle<v8::Context> context = v8::Context::New(isolate);

    //Enter the created context
    v8::Context::Scope context_scope(context);
    
    //Create, compile and run a script in the current context
    v8::Handle<v8::String> source = v8::String::NewFromUtf8(isolate, "'Hello' + ', World!'");
    v8::Handle<v8::Script> script = v8::Script::Compile(source);
    v8::Handle<v8::Value> result = script->Run();

    //Get the result of running the script ('Hello, World!'), and print it
    v8::String::Utf8Value resultStr(result);
    std::cout << *resultStr << std::endl;

    return 0;
}

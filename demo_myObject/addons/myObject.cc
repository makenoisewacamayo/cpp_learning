#include "myObject.h"

using namespace Napi;

MyObject::MyObject(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MyObject>(info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 ) {
       Napi::TypeError::New(env, "Wrong Numbers of Arguments").ThrowAsJavaScriptException();
    }

    if (!info[0].IsString()) {
        Napi::TypeError::New(env, "Expected a string as the first argument").ThrowAsJavaScriptException();
    }

    this->_greeterName = info[0].As<Napi::String>().Utf8Value();
}

Napi::Value MyObject::Greet(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 ) {
       Napi::TypeError::New(env, "Wrong Numbers of Arguments").ThrowAsJavaScriptException();
    }

    if (!info[0].IsString()) {
        Napi::TypeError::New(env, "Expected a string as the first argument").ThrowAsJavaScriptException();
    }

    Napi::String name = info[0].As<Napi::String>();
    printf("Hello: %s\n", name.Utf8Value().c_str());
    printf("Hello: %s\n", name.Utf8Value().c_str());

    return Napi::Value();
}

Napi::Value MyObject::Add(const CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 ) {
       Napi::TypeError::New(env, "Wrong Numbers of Arguments").ThrowAsJavaScriptException();
    }

    if (!info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Expected two numbers as arguments").ThrowAsJavaScriptException();
    }

    double a = info[0].As<Napi::Number>().DoubleValue();
    double b = info[1].As<Napi::Number>().DoubleValue();
    return Napi::Number::New(env, a + b);
}

Napi::Function MyObject::GetClass(Napi::Env env) {
    return DefineClass(
            env, 
            "MyObject", {
                MyObject::InstanceMethod("greet", &MyObject::Greet),
                MyObject::InstanceMethod("add", &MyObject::Add)
    });
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    Napi::String name = Napi::String::New(env, "MyObject");
    exports.Set(name, MyObject::GetClass(env));
    return exports;
}

NODE_API_MODULE(addon, Init)
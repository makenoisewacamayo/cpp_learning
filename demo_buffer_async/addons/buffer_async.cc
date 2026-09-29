#include "buffer_async.h"
#include <algorithm>
#include <thread>
#include <chrono>


Napi::FunctionReference BufferAsync::constructor;

BufferAsync::BufferAsync(const Napi::CallbackInfo& info) : Napi::ObjectWrap<BufferAsync>(info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Expected a string as the first argument").ThrowAsJavaScriptException();
        return;
    }
    this->name_ = info[0].As<Napi::String>().Utf8Value();
}

Napi::Value BufferAsync::GetName(const Napi::CallbackInfo& info) {
    return Napi::String::New(info.Env(), this->name_);
}

 Napi::Value BufferAsync::AddAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber() ) {
        Napi::TypeError::New(env, "Expected two numbers").ThrowAsJavaScriptException();
        return env.Null();
    }

    double a = info[0].As<Napi::Number>().DoubleValue();
    double b = info[1].As<Napi::Number>().DoubleValue();
  
    auto deferred = Napi::Promise::Deferred::New(env);
    auto threadSafeFn = Napi::ThreadSafeFunction::New(
        env,
        Napi::Function::New(env, [](const Napi::CallbackInfo&) {}),  // dummy callback (unused)
        "AddAsyncThreadSafeFunction",
        0,
        1
    );

    std::thread([a, b, deferred, threadSafeFn]() mutable {
        // Simulate some work with a sleep (on the worker thread)
        std::this_thread::sleep_for(std::chrono::seconds(1));
        double result = a + b;

        // Schedule promise resolution on the JS thread
        threadSafeFn.BlockingCall([deferred, result](Napi::Env env, Napi::Function) mutable {
            deferred.Resolve(Napi::Number::New(env, result));
        });
        threadSafeFn.Release();
    }).detach();

  return deferred.Promise();
}

Napi::Value BufferAsync::ReverseBuffer(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsBuffer()) {
        Napi::TypeError::New(env, "Expected a Buffer").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();
    size_t len = buffer.Length();

    Napi::Buffer<uint8_t> out = Napi::Buffer<uint8_t>::Copy(env, buffer.Data(), len);
    std::reverse(out.Data(), out.Data() + len);
    return out;
}


void BufferAsync::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "BufferAsync", {
    InstanceMethod("getName", &BufferAsync::GetName),
    InstanceMethod("addAsync", &BufferAsync::AddAsync),
    InstanceMethod("reverseBuffer", &BufferAsync::ReverseBuffer),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();  
  exports.Set("BufferAsync", func);
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  BufferAsync::Init(env, exports);
  return exports;
}
NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)
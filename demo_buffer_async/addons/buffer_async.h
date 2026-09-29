#pragma once
#include <napi.h>
#include <string>

class BufferAsync : public Napi::ObjectWrap<BufferAsync> {
 public:
  static void Init(Napi::Env env, Napi::Object exports);
  BufferAsync(const Napi::CallbackInfo& info);

  Napi::Value GetName(const Napi::CallbackInfo& info);

  Napi::Value AddAsync(const Napi::CallbackInfo& info);

  Napi::Value ReverseBuffer(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;
  std::string name_;
};

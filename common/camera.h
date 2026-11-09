#pragma once
 
#include <napi.h>
 
Napi::Value OpenCamera(const Napi::CallbackInfo& info);
Napi::Value ReadFrame(const Napi::CallbackInfo& info);
Napi::Value CloseCamera(const Napi::CallbackInfo& info);
Napi::Object Init(Napi::Env env, Napi::Object exports);
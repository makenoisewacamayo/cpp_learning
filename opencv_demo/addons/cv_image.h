#pragma once
#include <napi.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>
#include <string>

class CvImage : public Napi::ObjectWrap<CvImage> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);
    static Napi::Object NewInstance(Napi::Env env, const cv::Mat& mat);  
    CvImage(const Napi::CallbackInfo& info);
    ~CvImage();  // JS methods
    Napi::Value ToBuffer(const Napi::CallbackInfo& info);        // toBuffer(ext, quality)
    Napi::Value Resize(const Napi::CallbackInfo& info);          // resize(width, height) -> CvImage (sync)
    Napi::Value ResizeAsync(const Napi::CallbackInfo& info);     // resizeAsync(width, height) -> Promise
    Napi::Value Width(const Napi::CallbackInfo& info);
    Napi::Value Height(const Napi::CallbackInfo& info);
    Napi::Value Release(const Napi::CallbackInfo& info);

  private:
    static Napi::FunctionReference constructor;
    cv::Mat mat_;
    Napi::Reference<Napi::Buffer<uint8_t>> bufferRef_; // optional: keep Buffer alive if zero-copy used

};

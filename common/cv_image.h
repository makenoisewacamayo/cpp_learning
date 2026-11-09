#pragma once

#include <napi.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <string>
#include <vector>

class CvImage : public Napi::ObjectWrap<CvImage> {
public:
    static void Init(Napi::Env env, Napi::Object exports);
    static Napi::Object NewInstance(Napi::Env env, const cv::Mat& mat);
    static bool IsInstance(Napi::Value value);

    CvImage(const Napi::CallbackInfo& info);
    ~CvImage();

    const cv::Mat& GetMat() const { return mat_; }

    Napi::Value ToBuffer(const Napi::CallbackInfo& info);
    Napi::Value Resize(const Napi::CallbackInfo& info);
    Napi::Value ResizeAsync(const Napi::CallbackInfo& info);
    Napi::Value Width(const Napi::CallbackInfo& info);
    Napi::Value Height(const Napi::CallbackInfo& info);
    Napi::Value Release(const Napi::CallbackInfo& info);

private:
    static Napi::FunctionReference constructor;
    cv::Mat mat_;
    Napi::Reference<Napi::Buffer<uint8_t>> bufferRef_;
};
#pragma once

#include <napi.h>
#include <opencv2/xobjdetect.hpp>

class FaceDetector : public Napi::ObjectWrap<FaceDetector> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Napi::Value DetectWithImage(const Napi::CallbackInfo& info);
    FaceDetector(const Napi::CallbackInfo& info);

private:
    Napi::Value Detect(const Napi::CallbackInfo& info);

    static Napi::Object RectToObject(
        Napi::Env env,
        const cv::Rect& rect,
        double scale
    );

    cv::CascadeClassifier cascade_;
    cv::CascadeClassifier nestedCascade_;
    double scale_ = 1.0;
    bool tryFlip_ = false;
};
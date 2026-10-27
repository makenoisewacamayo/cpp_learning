#pragma once

#include <napi.h>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>

class Classifier : public Napi::ObjectWrap<Classifier> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Classifier(const Napi::CallbackInfo& info);

private:
    Napi::Value Classify(const Napi::CallbackInfo& info);

    cv::dnn::Net net_;
    std::vector<std::string> labels_;

    int width_ = 224;
    int height_ = 224;
    int topK_ = 5;
    double scale_ = 1.0;
    cv::Scalar mean_ = cv::Scalar();
    cv::Scalar std_ = cv::Scalar(1, 1, 1);
    bool swapRB_ = false;
    bool crop_ = false;
};
#include "classifier.h"
#include "cv_image.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <fstream>
#include <numeric>
#include <sstream>

Napi::Object Classifier::Init(Napi::Env env, Napi::Object exports) {
    auto cls = DefineClass(env, "Classifier", {
        InstanceMethod("classify", &Classifier::Classify)
    });

    exports.Set("Classifier", cls);
    return exports;
}

// Usage:
// new Classifier(modelPath, labelsPath, options)
Classifier::Classifier(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<Classifier>(info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Expected an ONNX model path")
            .ThrowAsJavaScriptException();
        return;
    }

    try {
        const std::string modelPath =
            info[0].As<Napi::String>().Utf8Value();

        net_ = cv::dnn::readNetFromONNX(modelPath);
        net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

        if (info.Length() > 1 && info[1].IsString()) {
            const std::string labelsPath =
                info[1].As<Napi::String>().Utf8Value();

            std::ifstream file(labelsPath);
            std::string line;
            while (std::getline(file, line)) {
                labels_.push_back(line);
            }
        }

        if (info.Length() > 2 && info[2].IsObject()) {
            Napi::Object options = info[2].As<Napi::Object>();

            if (options.Has("width"))
                width_ = options.Get("width").ToNumber().Int32Value();
            if (options.Has("height"))
                height_ = options.Get("height").ToNumber().Int32Value();
            if (options.Has("topK"))
                topK_ = options.Get("topK").ToNumber().Int32Value();
            if (options.Has("scale"))
                scale_ = options.Get("scale").ToNumber().DoubleValue();
            if (options.Has("swapRB"))
                swapRB_ = options.Get("swapRB").ToBoolean().Value();
            if (options.Has("crop"))
                crop_ = options.Get("crop").ToBoolean().Value();

            if (width_ <= 0 || height_ <= 0 || topK_ <= 0 || scale_ <= 0) {
                Napi::RangeError::New(
                    env, "width, height, topK, and scale must be positive"
                ).ThrowAsJavaScriptException();
                return;
            }
        }
    } catch (const cv::Exception& e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    }
}

Napi::Value Classifier::Classify(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() != 1 || !CvImage::IsInstance(info[0])) {
        Napi::TypeError::New(env, "classify expects a CvImage")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    auto* cvImage = Napi::ObjectWrap<CvImage>::Unwrap(
        info[0].As<Napi::Object>()
    );
    const cv::Mat& frame = cvImage->GetMat();

    if (frame.empty()) {
        Napi::Error::New(env, "CvImage is empty or has been released")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        cv::Mat blob;
        cv::dnn::blobFromImage(
            frame, blob, scale_, cv::Size(width_, height_),
            mean_, swapRB_, crop_
        );

        if (std_ != cv::Scalar(1, 1, 1)) {
            cv::divide(blob, std_, blob);
        }

        net_.setInput(blob);
        cv::Mat output = net_.forward();

        if (output.depth() != CV_32F) {
            output.convertTo(output, CV_32F);
        }

        cv::Mat scores = output.reshape(1, 1);
        if (!scores.isContinuous()) {
            scores = scores.clone();
        }

        const float* values = scores.ptr<float>();
        const int count = static_cast<int>(scores.total());
        const int resultCount = std::min(topK_, count);

        std::vector<int> indices(count);
        std::iota(indices.begin(), indices.end(), 0);
        std::partial_sort(
            indices.begin(), indices.begin() + resultCount, indices.end(),
            [&](int a, int b) { return values[a] > values[b]; }
        );

        cv::Mat annotated = frame.clone();
        Napi::Array predictions = Napi::Array::New(env, resultCount);

        for (int i = 0; i < resultCount; ++i) {
            const int classId = indices[i];
            const std::string label =
                classId < static_cast<int>(labels_.size())
                    ? labels_[classId]
                    : "Class #" + std::to_string(classId);

            Napi::Object prediction = Napi::Object::New(env);
            prediction.Set("classId", classId);
            prediction.Set("label", label);
            prediction.Set("confidence", values[classId]);
            predictions.Set(i, prediction);

            cv::putText(
                annotated,
                label + ": " + cv::format("%.3f", values[classId]),
                cv::Point(20, 40 + i * 35),
                cv::FONT_HERSHEY_SIMPLEX,
                0.8,
                cv::Scalar(0, 255, 0),
                2
            );
        }

        std::vector<uchar> jpg;
        if (!cv::imencode(".jpg", annotated, jpg)) {
            Napi::Error::New(env, "Could not encode annotated image")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

        Napi::Object result = Napi::Object::New(env);
        result.Set("predictions", predictions);
        result.Set(
            "image",
            Napi::Buffer<uint8_t>::Copy(env, jpg.data(), jpg.size())
        );
        return result;
    } catch (const cv::Exception& e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}
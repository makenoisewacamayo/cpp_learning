#include "camera.h"
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>

static cv::VideoCapture camera;


Napi::Value OpenCamera(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
 
    int device = info.Length() > 0
        ? info[0].As<Napi::Number>().Int32Value() : 0;
    int width = info.Length() > 1
        ? info[1].As<Napi::Number>().Int32Value() : 640;
    int height = info.Length() > 2
        ? info[2].As<Napi::Number>().Int32Value() : 480;
 
    camera.release();
    if (!camera.open(device, cv::CAP_AVFOUNDATION)) {
        Napi::Error::New(env, "Could not open camera").ThrowAsJavaScriptException();
        return env.Undefined();
    }
 
    camera.set(cv::CAP_PROP_FRAME_WIDTH, width);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, height);
 
    int actualWidth = static_cast<int>(camera.get(cv::CAP_PROP_FRAME_WIDTH));
    int actualHeight = static_cast<int>(camera.get(cv::CAP_PROP_FRAME_HEIGHT));
 
    std::cout << "Camera resolution: "
              << actualWidth << "x" << actualHeight << std::endl;
 
    Napi::Object result = Napi::Object::New(env);
    result.Set("device", device);
    result.Set("width", actualWidth);
    result.Set("height", actualHeight);
    return result;
}

Napi::Value ReadFrame(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    cv::Mat frame;

    if (!camera.isOpened() || !camera.read(frame) || frame.empty()) {
        Napi::Error::New(env, "Could not read camera frame").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    std::vector<unsigned char> jpg;
    cv::imencode(".jpg", frame, jpg);
    return Napi::Buffer<unsigned char>::Copy(env, jpg.data(), jpg.size());
}

Napi::Value CloseCamera(const Napi::CallbackInfo& info) {
    camera.release();
    return info.Env().Undefined();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("open", Napi::Function::New(env, OpenCamera));
    exports.Set("readFrame", Napi::Function::New(env, ReadFrame));
    exports.Set("close", Napi::Function::New(env, CloseCamera));
    return exports;
}

NODE_API_MODULE(camera, Init)
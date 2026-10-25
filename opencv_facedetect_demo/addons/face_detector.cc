#include "face_detector.h"
#include "cv_image.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <cmath>
#include <string>
#include <vector>

Napi::Object FaceDetector::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function cls = DefineClass(env, "FaceDetector", {
        InstanceMethod("detect", &FaceDetector::Detect),
        InstanceMethod("detectWithImage", &FaceDetector::DetectWithImage)
    });

    exports.Set("FaceDetector", cls);
    return exports;
}

FaceDetector::FaceDetector(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<FaceDetector>(info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "Expected a face cascade path")
            .ThrowAsJavaScriptException();
        return;
    }

    const std::string facePath = info[0].As<Napi::String>().Utf8Value();
    std::string nestedPath;

    if (info.Length() > 1 && info[1].IsString()) {
        nestedPath = info[1].As<Napi::String>().Utf8Value();
    }

    if (!cascade_.load(facePath)) {
        Napi::Error::New(env, "Could not load face cascade: " + facePath)
            .ThrowAsJavaScriptException();
        return;
    }

    if (!nestedPath.empty() && !nestedCascade_.load(nestedPath)) {
        Napi::Error::New(env, "Could not load nested cascade: " + nestedPath)
            .ThrowAsJavaScriptException();
        return;
    }

    if (info.Length() > 2 && info[2].IsObject()) {
        Napi::Object options = info[2].As<Napi::Object>();

        if (options.Has("scale")) {
            scale_ = options.Get("scale").ToNumber().DoubleValue();
            if (!std::isfinite(scale_) || scale_ < 1.0) {
                Napi::RangeError::New(
                    env, "scale must be a finite number >= 1"
                ).ThrowAsJavaScriptException();
                return;
            }
        }

        if (options.Has("tryFlip")) {
            tryFlip_ = options.Get("tryFlip").ToBoolean().Value();
        }
    }
}

Napi::Object FaceDetector::RectToObject(
    Napi::Env env,
    const cv::Rect& rect,
    double scale
) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("x", cvRound(rect.x * scale));
    result.Set("y", cvRound(rect.y * scale));
    result.Set("width", cvRound(rect.width * scale));
    result.Set("height", cvRound(rect.height * scale));
    return result;
}

Napi::Value FaceDetector::Detect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() != 1 || !CvImage::IsInstance(info[0])) {
        Napi::TypeError::New(env, "detect expects a CvImage")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    auto* cvImage = Napi::ObjectWrap<CvImage>::Unwrap(
        info[0].As<Napi::Object>()
    );
    const cv::Mat& image = cvImage->GetMat();

    if (image.empty()) {
        Napi::Error::New(env, "CvImage is empty or has been released")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    try {
        cv::Mat gray;

        if (image.channels() == 1) {
            gray = image;
        } else if (image.channels() == 3) {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        } else if (image.channels() == 4) {
            cv::cvtColor(image, gray, cv::COLOR_BGRA2GRAY);
        } else {
            Napi::TypeError::New(env, "Unsupported image channel count")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

        cv::Mat smallImage;
        cv::resize(
            gray,
            smallImage,
            cv::Size(),
            1.0 / scale_,
            1.0 / scale_,
            cv::INTER_LINEAR_EXACT
        );
        cv::equalizeHist(smallImage, smallImage);

        std::vector<cv::Rect> faces;
        cascade_.detectMultiScale(
            smallImage, faces, 1.1, 2,
            cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30)
        );

        if (tryFlip_) {
            cv::Mat flipped;
            cv::flip(smallImage, flipped, 1);

            std::vector<cv::Rect> flippedFaces;
            cascade_.detectMultiScale(
                flipped, flippedFaces, 1.1, 2,
                cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30)
            );

            for (const cv::Rect& face : flippedFaces) {
                faces.emplace_back(
                    smallImage.cols - face.x - face.width,
                    face.y,
                    face.width,
                    face.height
                );
            }
        }

        Napi::Array result = Napi::Array::New(env, faces.size());

        for (size_t i = 0; i < faces.size(); ++i) {
            Napi::Object faceResult = RectToObject(env, faces[i], scale_);
            Napi::Array eyes = Napi::Array::New(env);

            if (!nestedCascade_.empty()) {
                std::vector<cv::Rect> detectedEyes;
                nestedCascade_.detectMultiScale(
                    smallImage(faces[i]),
                    detectedEyes,
                    1.1,
                    2,
                    cv::CASCADE_SCALE_IMAGE,
                    cv::Size(30, 30)
                );

                for (size_t j = 0; j < detectedEyes.size(); ++j) {
                    cv::Rect eye = detectedEyes[j];
                    eye.x += faces[i].x;
                    eye.y += faces[i].y;
                    eyes.Set(j, RectToObject(env, eye, scale_));
                }
            }

            faceResult.Set("eyes", eyes);
            result.Set(i, faceResult);
        }

        return result;
    } catch (const cv::Exception& e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
}

Napi::Value FaceDetector::DetectWithImage(
    const Napi::CallbackInfo& info
) {
    Napi::Env env = info.Env();

    // Run detection using the existing method.
    Napi::Value facesValue = Detect(info);
    if (facesValue.IsNull()) {
        return env.Null();
    }

    auto* cvImage = Napi::ObjectWrap<CvImage>::Unwrap(
        info[0].As<Napi::Object>()
    );
    const cv::Mat& image = cvImage->GetMat();

    try {
        // Use BGR so the colored rectangles also work for grayscale input.
        cv::Mat annotated;
        if (image.channels() == 1) {
            cv::cvtColor(image, annotated, cv::COLOR_GRAY2BGR);
        } else if (image.channels() == 4) {
            cv::cvtColor(image, annotated, cv::COLOR_BGRA2BGR);
        } else {
            annotated = image.clone();
        }

        Napi::Array faces = facesValue.As<Napi::Array>();

        for (uint32_t i = 0; i < faces.Length(); ++i) {
            Napi::Object face = faces.Get(i).As<Napi::Object>();

            cv::Rect faceRect(
                face.Get("x").ToNumber().Int32Value(),
                face.Get("y").ToNumber().Int32Value(),
                face.Get("width").ToNumber().Int32Value(),
                face.Get("height").ToNumber().Int32Value()
            );

            cv::rectangle(
                annotated, faceRect, cv::Scalar(0, 255, 0), 3
            );

            Napi::Array eyes = face.Get("eyes").As<Napi::Array>();
            for (uint32_t j = 0; j < eyes.Length(); ++j) {
                Napi::Object eye = eyes.Get(j).As<Napi::Object>();

                cv::Rect eyeRect(
                    eye.Get("x").ToNumber().Int32Value(),
                    eye.Get("y").ToNumber().Int32Value(),
                    eye.Get("width").ToNumber().Int32Value(),
                    eye.Get("height").ToNumber().Int32Value()
                );

                cv::rectangle(
                    annotated, eyeRect, cv::Scalar(255, 0, 0), 2
                );
            }
        }

        std::vector<uchar> jpg;
        if (!cv::imencode(".jpg", annotated, jpg)) {
            Napi::Error::New(env, "Could not encode annotated image")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

        Napi::Object result = Napi::Object::New(env);
        result.Set("faces", faces);
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
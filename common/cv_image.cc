#include "cv_image.h"
#include <napi.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include "face_detector.h"


Napi::FunctionReference CvImage::constructor;


CvImage::CvImage(const Napi::CallbackInfo& info) : Napi::ObjectWrap<CvImage>(info) {
    Napi::Env env = info.Env();
    if (info.Length() == 1 && info[0].IsExternal()) {
       Napi::External<cv::Mat> externalMat = info[0].As<Napi::External<cv::Mat>>();
       this->mat_ = *externalMat.Data();
    } else {
        Napi::TypeError::New(env, "Use CvImage.fromBuffer to create images").ThrowAsJavaScriptException();
        return;
    }
}

CvImage::~CvImage() {
  // mat_ will be released automatically
}

bool CvImage::IsInstance(Napi::Value value) {
    return value.IsObject() &&
           value.As<Napi::Object>().InstanceOf(constructor.Value());
}

void CvImage::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);  
  Napi::Function func = DefineClass(env, "CvImage", {
    InstanceMethod("toBuffer", &CvImage::ToBuffer),
    InstanceMethod("resize", &CvImage::Resize),
    InstanceMethod("resizeAsync", &CvImage::ResizeAsync),
    InstanceMethod("width", &CvImage::Width),
    InstanceMethod("height", &CvImage::Height),
    InstanceMethod("release", &CvImage::Release),
  });
  constructor = Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("CvImage", func);

  exports.Set("fromBuffer", Napi::Function::New(env, [](const Napi::CallbackInfo& info)->Napi::Value {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsBuffer()) {
      Napi::TypeError::New(env, "Expected Buffer as first argument").ThrowAsJavaScriptException();
      return env.Null();    
    }

    Napi::Buffer<uint8_t> buf = info[0].As<Napi::Buffer<uint8_t>>();
    std::vector<uchar> data(buf.Data(), buf.Data() + buf.Length());
    cv::Mat mat = cv::imdecode(data, cv::IMREAD_UNCHANGED);
    if (mat.empty()) {
      Napi::Error::New(env, "Failed to decode image").ThrowAsJavaScriptException();
      return env.Null();
    }

    auto* pm = new cv::Mat(mat);
    auto external = Napi::External<cv::Mat>::New(
      env,
      pm,
      [](Napi::Env, cv::Mat* value) {
          delete value;
      }
    );
    return constructor.New({ external });
  }));
}


Napi::Object CvImage::NewInstance(Napi::Env env, const cv::Mat& mat) {
    auto* pm = new cv::Mat(mat);
    auto external = Napi::External<cv::Mat>::New(
        env,
        pm,
        [](Napi::Env, cv::Mat* value) {
            delete value;
        }
    );
    return constructor.New({ external });
}


Napi::Value CvImage::ToBuffer(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env(); 
    std::string ext = ".jpg";
    int quality = 90;
    if (info.Length() >= 1 && info[0].IsString()) ext = info[0].As<Napi::String>().Utf8Value();
    if (info.Length() >= 2 && info[1].IsNumber()) quality = info[1].As<Napi::Number>().Int32Value();

    std::vector<int> params;
    if (ext == ".jpg" || ext == ".jpeg") params = {cv::IMWRITE_JPEG_QUALITY, quality};
    else if (ext == ".png") params = {cv::IMWRITE_PNG_COMPRESSION, std::max(0, 9 - (quality / 11))};
    
    std::vector<uchar> out;
    try {
        if (!cv::imencode(ext, mat_, out, params)) {
           Napi::Error::New(env, "imencode failed").ThrowAsJavaScriptException();
           return env.Null();
        }
    } catch (const cv::Exception& e) {
        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
        return env.Null();
    }
    return Napi::Buffer<uint8_t>::Copy(env, out.data(), out.size());
}

Napi::Value CvImage::Width(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, mat_.cols);
}
Napi::Value CvImage::Height(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, mat_.rows);
}

Napi::Value CvImage::Release(const Napi::CallbackInfo& info) {
  mat_.release();
  return info.Env().Undefined();
}

Napi::Value CvImage::Resize(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
    Napi::TypeError::New(env, "Expected width and height").ThrowAsJavaScriptException();
    return env.Null();
  }
  int width = info[0].As<Napi::Number>().Int32Value();
  int height = info[1].As<Napi::Number>().Int32Value();
  cv::Mat dst;
  try {
    cv::resize(mat_, dst, cv::Size(width, height));
  } catch (const cv::Exception& e) {
    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    return env.Null();
  }
  return NewInstance(env, dst);
}


class ResizeWorker : public Napi::AsyncWorker {
 public:
  ResizeWorker(Napi::Promise::Deferred deferred, cv::Mat src, int w, int h, Napi::Env env)
    : Napi::AsyncWorker(env), deferred_(deferred), src_(src), w_(w), h_(h) {}  ~ResizeWorker() {}  void Execute() override {
    try {
      cv::resize(src_, dst_, cv::Size(w_, h_));
    } catch (const cv::Exception& e) {
      SetError(e.what());
    }
  }  void OnOK() override {
    Napi::Env env = Env();
    Napi::Object obj = CvImage::NewInstance(env, dst_);
    deferred_.Resolve(obj);
  }  void OnError(const Napi::Error& e) override {
    deferred_.Reject(e.Value());
  } private:
  Napi::Promise::Deferred deferred_;
  cv::Mat src_;
  cv::Mat dst_;
  int w_;
  int h_;
};

Napi::Value CvImage::ResizeAsync(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
    Napi::TypeError::New(env, "Expected width and height").ThrowAsJavaScriptException();
    return env.Null();
  }
  int width = info[0].As<Napi::Number>().Int32Value();
  int height = info[1].As<Napi::Number>().Int32Value();  auto deferred = Napi::Promise::Deferred::New(env);
  // Capture a copy of mat_ for worker
  ResizeWorker* worker = new ResizeWorker(deferred, mat_.clone(), width, height, env);
  worker->Queue();
  return deferred.Promise();
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    CvImage::Init(env, exports);
    FaceDetector::Init(env, exports);
    return exports;
}
 

// Finalizer note: we allocated cv::Mat on heap when constructing via External.
// We must free it when the JS object is garbage-collected. Hook into a finalizer by overriding the destructor of external object.
// To do that, we need to intercept construction: when constructor receives External, it took ownership of new cv::Mat in NewInstance.
// So ensure external memory is deleted when JS object is finalized:
namespace {
    void FinalizeMat(Napi::Env env, cv::Mat* mat) {
        delete mat;
    }
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)
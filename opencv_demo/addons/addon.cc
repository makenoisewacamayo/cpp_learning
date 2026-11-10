#include <napi.h>
#include "cv_image.h"


static Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    CvImage::Init(env, exports);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)

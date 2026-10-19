#pragma once
#include <napi.h>
#include "opencv2/xobjdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <string>
#include <vector>
#include <memory>
#include <string>

class CvImage : public Napi::ObjectWrap<CvImage> {
  
};

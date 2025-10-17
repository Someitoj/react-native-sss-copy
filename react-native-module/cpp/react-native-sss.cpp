#include "react-native-sss.h"
#include "fast-opencv/ConvertImage.hpp"
#include "jsi-utils/Promise.h"
#include "jsi-utils/TypedArray.h"
#include "main/mainfunctions.hpp"

#include <opencv2/opencv.hpp>
#include <jsi/jsi.h>
#include <iostream>

using namespace mrousavy;

void Sss::installOpenCV(jsi::Runtime& runtime, std::shared_ptr<react::CallInvoker> callInvoker) {

  auto func = [=](jsi::Runtime& runtime,
    const jsi::Value& thisArg,
    const jsi::Value* args,
    size_t count) -> jsi::Value {
      auto plugin = std::make_shared<Sss>(callInvoker);
      auto result = jsi::Object::createFromHostObject(runtime, plugin);

      return result;
    };

  auto jsiFunc = jsi::Function::createFromHostFunction(runtime,
    jsi::PropNameID::forUtf8(runtime, "__loadSss"),
    1,
    func);

  runtime.global().setProperty(runtime, "__loadSss", jsiFunc);

};

Sss::Sss(std::shared_ptr<react::CallInvoker> callInvoker) : _callInvoker(callInvoker) {};

cv::CascadeClassifier classifier;

jsi::Value Sss::get(jsi::Runtime& runtime, const jsi::PropNameID& propNameId) {
  auto propName = propNameId.utf8(runtime);
  if (classifier.empty()) {
    classifier = cv::CascadeClassifier("..\\cpp\\main\\cascade.xml");
  }
  if (propName == "testFrame") {
    return jsi::Function::createFromHostFunction(
      runtime, jsi::PropNameID::forAscii(runtime, "testFrame"), 4,
      [=](jsi::Runtime& runtime, const jsi::Value& thisValue, const jsi::Value* arguments,
        size_t count) -> jsi::Object {
          auto rows = arguments[0].asNumber();
          auto cols = arguments[1].asNumber();
          auto channels = arguments[2].asNumber();
          auto input = arguments[3].asObject(runtime);

          auto type = -1;
          if (channels == 1) {
            type = CV_8U;
          }
          if (channels == 3) {
            type = CV_8UC3;
          }
          if (channels == 4) {
            type = CV_8UC4;
          }

          if (channels == -1) {
            throw std::runtime_error("Sss Error: Invalid channel count passed to frameBufferToMat!");
          }

          auto inputBuffer = getTypedArray(runtime, std::move(input));
          auto vec = inputBuffer.toVector(runtime);

          cv::Mat img(rows, cols, type);
          //cv::Mat dst;

          //cv::Canny(img, dst, 50, 100);

          std::string format = "jpeg";
          jsi::Object value(runtime);
          value.setProperty(runtime, "base64", jsi::String::createFromUtf8(runtime, ImageConverter::mat2str(functions::test_image(img, classifier), format)));
          return value;
      });
  } else if (propName == "processFrame") {
    return jsi::Function::createFromHostFunction(
      runtime, jsi::PropNameID::forAscii(runtime, "testFrame"), 4,
      [=](jsi::Runtime& runtime, const jsi::Value& thisValue, const jsi::Value* arguments,
        size_t count) -> jsi::Object {
          auto rows = arguments[0].asNumber();
          auto cols = arguments[1].asNumber();
          auto channels = arguments[2].asNumber();
          auto input = arguments[3].asObject(runtime);
          
          auto type = -1;
          if (channels == 1) {
            type = CV_8U;
          }
          if (channels == 3) {
            type = CV_8UC3;
          }
          if (channels == 4) {
            type = CV_8UC4;
          }

          if (channels == -1) {
            throw std::runtime_error("Sss Error: Invalid channel count passed to frameBufferToMat!");
          }

          auto inputBuffer = getTypedArray(runtime, std::move(input));
          auto vec = inputBuffer.toVector(runtime);

          cv::Mat img(rows, cols, type);
          

          string output = functions::process_image(img, classifier);

          jsi::Object value(runtime);
          value.setProperty(runtime, "output", jsi::String::createFromUtf8(runtime, output));
          return value;
      });
  } else if (propName == "test") {
    return jsi::Function::createFromHostFunction(
      runtime, jsi::PropNameID::forAscii(runtime, "test"), 1,
      [=](jsi::Runtime& runtime, const jsi::Value& thisValue, const jsi::Value* arguments,
        size_t count) -> jsi::Object {
          std::string base64 = arguments[0].asString(runtime).utf8(runtime);

          cv::Mat img = ImageConverter::str2mat(base64);
          cv::Mat dst;
          cv::Canny(img, dst, 50, 100);
          std::string format = "jpeg";
          jsi::Object value(runtime);
          value.setProperty(runtime, "base64", jsi::String::createFromUtf8(runtime, ImageConverter::mat2str(dst, format)));
          return value;
      });
  } else if (propName == "processImage") {
    return jsi::Function::createFromHostFunction(
      runtime, jsi::PropNameID::forAscii(runtime, "test"), 1,
      [=](jsi::Runtime& runtime, const jsi::Value& thisValue, const jsi::Value* arguments,
        size_t count) -> jsi::Object {
          std::string base64 = arguments[0].asString(runtime).utf8(runtime);

          cv::Mat img = ImageConverter::str2mat(base64);
          cv::Mat dst;

          string output = functions::process_image(img);

          jsi::Object value(runtime);
          value.setProperty(runtime, "output", jsi::String::createFromUtf8(runtime, output));
          return value;
      });
  }

  return jsi::HostObject::get(runtime, propNameId);
};

std::vector<jsi::PropNameID> Sss::getPropertyNames(jsi::Runtime& runtime) {
  std::vector<jsi::PropNameID> result;

  result.push_back(jsi::PropNameID::forAscii(runtime, "test"));
  result.push_back(jsi::PropNameID::forAscii(runtime, "testFrame"));
  result.push_back(jsi::PropNameID::forAscii(runtime, "processImage"));
  result.push_back(jsi::PropNameID::forAscii(runtime, "processFrame"));

  return result;
};

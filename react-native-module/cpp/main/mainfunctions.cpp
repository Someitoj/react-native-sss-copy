#include "mainfunctions.hpp"

#include "imageprocessing.hpp"
#include "objectdetect.hpp"

#include <opencv2/opencv.hpp>
#include <filesystem>

using namespace std;

namespace functions {
  string process_image(cv::Mat& inputImage, cv::CascadeClassifier& classifier) {
    cv::Mat cropped, proc_display;
    string text = "";
    objectdetect::cascade_classifier(inputImage, cropped, classifier);
    if (cropped.empty()) {
      /* this was for finding why the mobile application could use the cascade classifier
      for (const auto& entry : filesystem::directory_iterator(filesystem::current_path())) {
        text += entry.path().string();
        text += " ";
      }
       */
      text = "NO DISPLAY FOUND";
    }
    else {
      processing::preprocess_display(cropped, proc_display);
      text = objectdetect::detect_digits(proc_display);
    }
    return text;
  }

  string process_image(cv::Mat& inputImage) {
    cv::Mat init, processed;
    vector<vector<cv::Point>> rectangles;
    list<string> text_output;
    processing::preprocess_init(inputImage, init);

    //Part 1: Detecting Displays
    processing::preprocess_full(init, processed);
    objectdetect::detect_display(processed, rectangles);

    //Part 2: Detecting Digits
    for (vector<cv::Point> rectangle : rectangles) {
      cv::Mat display,proc_display;
      vector<cv::Rect> digits;
      string text = "";

      processing::crop_display(init, display, rectangle);
      processing::preprocess_display(display, proc_display);
      objectdetect::detect_digits(proc_display/*, digits*/);

      //Part 3: Recognising Digits
      for (cv::Rect d : digits) {
        cv::Mat crop_digit, proc_digit;

        processing::crop_digit(proc_display, crop_digit, d);
        processing::preprocess_digit(crop_digit,proc_digit);
        char c_digit = objectdetect::recognise_digit(proc_digit);
        text += c_digit;
      }
      text_output.push_back(text);
    }

    string return_text = "";
    for (auto t : text_output) {
      return_text += t + " ";
    }

    return return_text;
  }

  cv::Mat test_image(cv::Mat& inputImage, cv::CascadeClassifier& classifier)
  {
    
    cv::Mat returnImg;
    cv::Mat cropped, proc_display;
    string text = "";
    objectdetect::cascade_classifier(inputImage, cropped, classifier);
    if (cropped.empty()) {
      processing::preprocess_display(inputImage, proc_display);
    }
    else {
      processing::preprocess_display(cropped, proc_display);
    }
    return proc_display;
  }
}

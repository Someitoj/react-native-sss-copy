#include <opencv2/opencv.hpp>

using namespace std;


//Contains the detection functionality
namespace objectdetect {

  /**
  * Detects the display
  *
  * @param[in] inputImage - A (preprocessed) image in which a display could be.
  * @param[out] rectangles - Returns a list of rectangles that could be the display.
  */
  void detect_display(cv::Mat& inputImage, vector<vector<cv::Point>>& rectangles);

  void cascade_classifier(cv::Mat& inputImage, cv::Mat& outputImage, cv::CascadeClassifier &classifier);

  /**
  * Detects the digits on a display.
  *
  * @param[in] inputImage - A (preprocessed) image of a display.
  * @param[out] digits - Returns a list of rectangles that enclose digits.
  */
  string detect_digits(cv::Mat& inputImage);
  void detect_digits_old(cv::Mat& inputImage, vector<cv::Rect>& digits);

  /**
  * Recognises a digit.
  * 
  * @param[in] inputImage - A (preprocessed) image of a single digit.
  */
  char recognise_digit(cv::Mat& inputImage);
}

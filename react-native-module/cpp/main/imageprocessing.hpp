#include <opencv2/opencv.hpp>

using namespace std;

/* Contains the image processing functionality
*/
namespace processing {

  /**
  * Preprocesses the initial image with processing steps that would otherwise be repeated everywhere
  *
  * TODO: after implementing the other steps, actually do this.
  * 
  * @param[in] inputImage - Image to be processed
  * @param[out] outputImage - Returns the processed image
  */
  void preprocess_init(cv::Mat& inputImage, cv::Mat& outputImage);
  
  /**
  * Preprocesses the initial image for use in "detect_display"
  *
  * @param[in] inputImage - Image to be processed
  * @param[out] outputImage - Returns the processed image
  */
  void preprocess_full(cv::Mat& inputImage, cv::Mat& outputImage);

  /**
  * Crop the display from the original image.
  *
  * @param[in] inputImage - Full original image
  * @param[out] outputImage - Returns an image featuring only the display
  * @param[in] rectangle - A rectangle which must be isolated
  */
  void crop_display(cv::Mat& inputImage, cv::Mat& outputImage, vector<cv::Point> rectangle);

  /**
  * Preprocesses the image of the display for use in "detect_digits"
  *
  * @param[in] inputImage - Display to be processed
  * @param[out] outputImage - Returns the processed display
  */
  void preprocess_display(cv::Mat& inputImage, cv::Mat& outputImage);
  void old_preprocess_display(cv::Mat& inputImage, cv::Mat& outputImage);

  /**
  * Crop the digit from the display.
  *
  * @param[in] inputImage - Display
  * @param[out] outputImage - Returns an image featuring only the digit
  * @param[in] rectangle - A rectangle which must be isolated
  */
  void crop_digit(cv::Mat& inputImage, cv::Mat& outputImage, cv::Rect& rectangle);

  /**
  * Preprocesses the image of the digit for use in "recognise_digits"
  *
  * @param[in] inputImage - Digit to be processed
  * @param[out] outputImage - Returns the processed digit
  */
  void preprocess_digit(cv::Mat& inputImage, cv::Mat& outputImage);

  
}

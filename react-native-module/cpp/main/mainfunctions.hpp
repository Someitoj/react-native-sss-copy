#include <opencv2/opencv.hpp>

using namespace std;

/* Contains the exported functions
* Uses the functions defined in imageprocessing.hpp and objectdetect.hpp
*/
namespace functions {
  /**
  * For debug testing on the phone, seeing if a function can be called and an edited image can be returned.
  *
  * @param[in] inputImage - Image of a display.
  * @returns A Mat containing something.
  */
  cv::Mat test_image(cv::Mat& inputImage, cv::CascadeClassifier& classifier);
  /**
  * Reads the display of an image.
  *
  * @param[in] inputImage - Image of a display.
  * @returns A string containing the contents of the display.
  */
  string process_image(cv::Mat& inputImage);
  /**
  * Reads the display of an image.
  *
  * @param[in] inputImage - Image of a display.
  * @param[in] classifier - The cascade classifier used for finding the display in the image.
  * @returns A string containing the contents of the display.
  */
  string process_image(cv::Mat& inputImage, cv::CascadeClassifier& classifier);
}

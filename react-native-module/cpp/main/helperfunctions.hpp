#include <opencv2/opencv.hpp>

using namespace std;

namespace helperfunctions {
  /**
  * Find the local maxima in a histogram
  *
  * Based on https://answers.opencv.org/question/56701/find-all-peaks-for-mat-in-opencv-c/
  *
  * @param[in] histogram - Histogram
  * @param[in] scale - if set to 0.0, only returns the 2 biggest maxima. Else, returns all local maxima that are  at least max value * scale or larger
  * @param[out] peaks - Returns the peaks. { index , value }
  */
  vector<cv::Point> find_local_maxima(cv::InputArray& histogram, const float scale = 0.0);

  /**
  * Get historgam from grayscale image
  *
  * @param[in] inputImage - grayscale image
  * @returns vector containing local maxima's
  */
  cv::Mat get_histogram_from_gray(cv::Mat& inputImage);

  /**
  * Get a matrix from grayscale image that, for each part of the image, says what grayscale value appears most in that area.
  *
  * @param[in] inputImage - grayscale image
  * @param[in] divide - vector containing at what relative point the image is split. Must be atleast size 3.
  * @param[in] center - boolean, if true this will append a vector at the end with [0] the peak of the entire image and [1] the peak of the center of the image
  * @returns 2 or 3 matrixes. first one contains all the peaks, second one all the second peaks and (optional) third one has for the center
  *
  * @note
  * Intended use: returns 3 4x4 matrixes. the first 2 contain the peak and second peak of the quadrants, the third one has that info for the whole image and the center
  * You can use the divider { 0.0, 1.0 / 3, 2.0/3 ,1.0 } but this did not give me good results.
  */
  vector < vector<vector<int>>> get_peak_matrix_from_gray(cv::Mat& inputImage, vector<float> divide = { 0.0, 0.5, 1.0 }, bool center = true);

}

#include <opencv2/opencv.hpp>
#include <ctime>

using namespace std;

/* Contains the exact same as mainfunctions.hpp
* But with debug features, timers, etc...
*/
namespace debug_main {
  /**
  * Reads the display of an image.
  *
  * @param[in] inputImage - Image of a display.
  * @returns A string containing the contents of the display.
  */
  string process_image(cv::Mat& inputImage);
}

/**
Class containing functions that allow testing this library as a console app.
Should only be used in files that run the code as such !
*/
class ImageHandler {
public:
  std::string img_folder = "C:\\test-images";                   //Folder where the full images are stored
  std::string output_folder = "C:\\output-images";              //Folder where the output images are stored
  std::vector<int> window_size = { 1000,750 };                 //Maximum size of the image when displaying it

  ImageHandler(string img_folder = "C:\\test-images", string output_folder = "C:\\output-images") {
    this->img_folder = img_folder;
    this->output_folder = output_folder;
  }
  /**
   * Provides a list of the names of all the images in your folders.
   *
   * @param print - Set to true if you to print the list to console.
   * @return 2 vectors with file names, first one contains the full images and the second vector contains the cropped images.
   */
  std::vector<std::string> list_images(bool print = true);

  /**
   * Loads an image
   *
   * @param img_name - Name of the file
   * @param cropped - False if it's a full image, True if it's a cropped image
   *
   * @return OpenCV Mat containing the image
   */
  cv::Mat load_image(std::string img_name);

  /**
  * Shows an image. Will make a local resized copy based on the window_size.
  *
  * @param img - The image
  * @param img_name - Will be used for the window name of the pop-up window showing the image.
  */
  void show_image(const cv::Mat img, std::string img_name = " ");

  /**
  * Save an image
  *
  * @param img - The image
  * @param img_name - The name of the image
  * @param folder - Subfolder where the image has to be saved to
  *
  * @return OpenCV Mat containing the image
  */
  void save_image(cv::Mat img, std::string img_name, std::string folder = "");

};

class Timer {
public:
  static Timer create();
  void start();
  float end();
  float getTime();
private:
  clock_t starttime;
  float time;
  Timer();
};

//https://stackoverflow.com/a/54062826
//https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797#colors--graphics-mode
//Following is purely to help output stand out.
namespace ConsoleColor {
  const string normal = "\033[0m";
  const string timer = "\x1B[46;30m";
  const string result = "\x1B[47;32m";
  const string function = "\x1B[31m";
}

namespace debug {
  /**
   * draws rectangles on image
   *
   * @param[in] inputImage - The original image
   * @param[out] showImage - The resulting image
   * @param[in] rectangles - Rectangles to draw
   */
  void drawRectangles(cv::Mat& inputImage, cv::Mat& showImage, vector<vector<cv::Point>>& rectangles);
  /**
   * draws rectangles on image
   *
   * @param[in] inputImage - The original image
   * @param[out] showImage - The resulting image
   * @param[in] rectangles - Rectangles to draw
   */
  void drawRectangles(cv::Mat& inputImage, cv::Mat& showImage, vector<cv::Rect>& rectangles);
  /**
   * draws histogram as result of doing colour profiling
   *
   * @param[in] inputImage - The original image
   * @param[out] showImage - The resulting image
   */
  void draw_profiling_histogram(cv::Mat& inputImage, cv::Mat& showImage);
  /**
   * draws given histogram
   *
   * @param[in] histogram - The histogram to draw
   * @param[out] showImage - The resulting image
   * @param[in] scale - determines the width of each bin of the histogram
   * @param[in] heigth - height of the image. If negative, determined automatically.
   */
  void draw_histogram(cv::Mat& histogram, cv::Mat& showImage, int scale = 2, int height = -1, int marge = 10, bool inv = false);
  /**
   * draws the effects of reduce on a grayscale image
   *
   * @param[in] inputImage - The grayscale image
   * @param[out] showImage - The resulting image
   * @param[in] marge - How large the added histograms are
   * @param[in] rows - Draw for the rows
   * @param[in] cols - Draw for the cols
   */
  void draw_reduce(cv::Mat& inputImage, cv::Mat& showImage, int marge = 200, bool rows = true, bool cols = true);
}

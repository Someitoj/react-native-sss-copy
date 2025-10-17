#include "debugfunctions.hpp"
#include "helperfunctions.hpp"
#include "imageprocessing.hpp"
#include "objectdetect.hpp"

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <vector>
#include <tuple>
#include <ctime>

using namespace std;

namespace debug_main {
  string process_image(cv::Mat& inputImage) {
    /*Init the output params
    *
    * Allows in debug to easily access in-between states of the image.
    * If the program is compiled with "debug = false",
    * the compiling process should make sure that this does not keep unneeded copies.
    */

    Timer t_total = Timer::create();

    cv::Mat init;
    cv::Mat processed;
    vector<vector<cv::Point>> rectangles;
    list<cv::Mat> displays;
    list<cv::Mat> proc_displays;
    list<string> text_output;

    ImageHandler hf = ImageHandler();
    cv::Mat showImage;

    Timer t0 = Timer::create();
    processing::preprocess_init(inputImage, init);
    t0.end();

    Timer t1 = Timer::create();
    processing::preprocess_full(init, processed);
    t1.end();

    Timer t2 = Timer::create();
    objectdetect::detect_display(processed, rectangles);
    t2.end();

    std::cout << ConsoleColor::function << "processing::preprocess_init\t" << ConsoleColor::timer << t0.getTime() << " sec" << ConsoleColor::normal << endl;
    std::cout << ConsoleColor::function << "processing::preprocess_full\t" << ConsoleColor::timer << t1.getTime() << " sec" << ConsoleColor::normal << endl;
    std::cout << ConsoleColor::function << "objectdetect::detect_display\t" << ConsoleColor::timer << t2.getTime() << " sec" << ConsoleColor::normal << endl;
    std::cout << ConsoleColor::result << "Displays Detected: " << rectangles.size() << ConsoleColor::normal << endl;

    //debug::drawRectangles(inputImage, showImage, rectangles);
    //hf.show_image(showImage);

    int display_counter = 0;

    Timer t3 = Timer::create();
    for (vector<cv::Point> rectangle : rectangles) {
      display_counter++;
      displays.push_back(cv::Mat());
      proc_displays.push_back(cv::Mat());
      vector<cv::Rect> digits;
      string text = "";

      Timer t3_1 = Timer::create();
      processing::crop_display(init, displays.back(), rectangle);
      t3_1.end();

      Timer t3_2 = Timer::create();
      processing::preprocess_display(displays.back(), proc_displays.back());
      t3_2.end();

      Timer t3_3 = Timer::create();
      objectdetect::detect_digits(proc_displays.back()/*, digits*/);
      t3_3.end();

      std::cout << ConsoleColor::function << "processing::crop_display\t[" << display_counter << "]\t" << ConsoleColor::timer << t3_1.getTime() << " sec" << ConsoleColor::normal << endl;
      std::cout << ConsoleColor::function << "processing::preprocess_display\t[" << display_counter << "]\t" << ConsoleColor::timer << t3_2.getTime() << " sec" << ConsoleColor::normal << endl;
      std::cout << ConsoleColor::function << "objectdetect::detect_digits\t[" << display_counter << "]\t" << ConsoleColor::timer << t3_3.getTime() << " sec" << ConsoleColor::normal << endl;
      std::cout << ConsoleColor::result << "[" << display_counter << "] Digits Detected: " << digits.size() << ConsoleColor::normal << endl;

      //hf.show_image(displays.back());
      //hf.show_image(proc_displays.back());
      //debug::drawRectangles(displays.back(), showImage, digits);
      //hf.show_image(showImage);

      int digit_counter = 0;
      Timer t3_4 = Timer::create();
      for (cv::Rect d : digits) {
        digit_counter++;
        cv::Mat crop_digit;
        cv::Mat proc_digit;

        Timer t3_d_1 = Timer::create();
        processing::crop_digit(proc_displays.back(), crop_digit, d);
        t3_d_1.end();

        Timer t3_d_2 = Timer::create();
        processing::preprocess_digit(crop_digit, proc_digit);
        t3_d_2.end();

        Timer t3_d_3 = Timer::create();
        char c_digit = objectdetect::recognise_digit(proc_digit);
        t3_d_3.end();

        text += c_digit;
        std::cout << ConsoleColor::function << "processing::crop_display\t[" << display_counter << "." << digit_counter << "]\t" << ConsoleColor::timer << t3_d_1.getTime() << " sec" << ConsoleColor::normal << endl;
        std::cout << ConsoleColor::function << "processing::preprocess_display\t[" << display_counter << "." << digit_counter << "]\t" << ConsoleColor::timer << t3_d_2.getTime() << " sec" << ConsoleColor::normal << endl;
        std::cout << ConsoleColor::function << "objectdetect::detect_digits\t[" << display_counter << "." << digit_counter << "]\t" << ConsoleColor::timer << t3_d_3.getTime() << " sec" << ConsoleColor::normal << endl;

        //hf.show_image(proc_digit);
      }
      text_output.push_back(text);
      t3_4.end();
      std::cout << ConsoleColor::function << "processed all digits      \t[" << display_counter << "]\t" << ConsoleColor::timer << t3_4.getTime() << " sec" << ConsoleColor::normal << endl;
    }
    t3.end();

    std::cout << ConsoleColor::function << "processed all displays   \t" << ConsoleColor::timer << t3.getTime() << " sec" << ConsoleColor::normal << endl;

    t_total.end();
    std::cout << ConsoleColor::function << "functions::process_image\t" << ConsoleColor::timer << t_total.getTime() << " sec" << ConsoleColor::normal << endl;

    string return_text = "";
    for (auto t : text_output) {
      return_text += t + " ";
    }
    return return_text;
  }
}

cv::Mat ImageHandler::load_image(std::string img_name) {
  std::string path = this->img_folder + "\\" + img_name;

  cv::Mat img = cv::imread(path, cv::IMREAD_COLOR);

  if (img.empty()) {
    std::cout << "Could not read the image: " << path << std::endl;
  }

  return img;
}

void ImageHandler::show_image(const cv::Mat img, std::string img_name) {

  cv::Mat resized;
  if (img.cols >= window_size[0] || img.rows >= window_size[1]) {
    if (img.cols / window_size[0] > img.rows / window_size[1]) {
      cv::resize(img, resized, cv::Size(window_size[0], int(round(img.rows / (img.cols / window_size[0])))));
    }
    else {
      cv::resize(img, resized, cv::Size(int(round(img.cols / (img.rows / window_size[1]))), window_size[1]));
    }
    cv::imshow("Display window: " + img_name, resized);
    cv::waitKey(0);
    cv::destroyAllWindows();
  }
  else {
    cv::imshow("Display window: " + img_name, img);
    cv::waitKey(0);
    cv::destroyAllWindows();
  }

}

void ImageHandler::save_image(cv::Mat img, std::string img_name, std::string folder) {
  string dir = this->output_folder + (folder == "" ? folder : "\\" + folder);
  if (!std::filesystem::is_directory(dir)) {
    std::filesystem::create_directory(dir);
  }
  cv::imwrite(dir + "\\" + img_name, img);
}

std::vector<std::string> ImageHandler::list_images(bool print) {
  std::vector<std::string> imgs = {};

  for (const auto& entry : std::filesystem::directory_iterator(img_folder))
    if (entry.path().has_extension()) { imgs.push_back(entry.path().filename().string()); }

  if (print) {
    std::cout << "Images: " << img_folder << std::endl;
    for (std::string item : imgs) {
      std::cout << "\t" << item << std::endl;
    }
  }
  return imgs;
}


Timer::Timer() {
  this->starttime = clock();
  this->time = 0.0;
}


Timer Timer::create() {
  return Timer();
}

void Timer::start() {
  this->starttime = clock();
}

float Timer::end() {
  this->time = (float)(clock() - this->starttime) / CLOCKS_PER_SEC;
  return this->time;
}

float Timer::getTime() {
  return this->time;
}

namespace debug {
  void drawRectangles(cv::Mat& inputImage, cv::Mat& showImage, vector<vector<cv::Point>>& rectangles) {
    if (inputImage.type() == CV_8UC1) {
      cv::cvtColor(inputImage, showImage, cv::COLOR_GRAY2RGB);
    }
    else {
      inputImage.copyTo(showImage);
    }
    cv::drawContours(showImage, rectangles, -1, cv::Scalar(0, 0, 250), 5);
  }

  void drawRectangles(cv::Mat& inputImage, cv::Mat& showImage, vector<cv::Rect>& rectangles) {
    if (inputImage.type() == CV_8UC1) {
      cv::cvtColor(inputImage, showImage, cv::COLOR_GRAY2RGB);
    }
    else {
      inputImage.copyTo(showImage);
    }
    for (cv::Rect r : rectangles) {
      cv::rectangle(showImage, r, cv::Scalar(0, 0, 250), 5);
    }
  }

  void draw_profiling_histogram(cv::Mat& inputImage, cv::Mat& showImage) {
    //Literally just the first step of preprocess_display
    cv::Mat gray;
    cv::cvtColor(inputImage, gray, cv::COLOR_BGR2GRAY);

    cv::Mat sample(gray(cv::Range(inputImage.rows / 4, inputImage.rows * 3 / 4), cv::Range(inputImage.cols / 4, inputImage.cols * 3 / 4)));
    cv::Mat hist = helperfunctions::get_histogram_from_gray(sample);
    
    vector<cv::Point> peaks = helperfunctions::find_local_maxima(hist);
    int center = (peaks[0].y + peaks[1].y) / 2;
    int buffer = abs(peaks[0].y - peaks[1].y) / 6;
    
    int x_low = center - buffer;
    int x_high = center + buffer;
    //cout << "CENTER: " << center << "\tLOW: " << x_low << "\tHIGH: " << x_high << endl;
    //The Drawing
    int scale = 3, height = 600, marge = 10;
    cv::Mat histImage;
    draw_histogram(hist, histImage, scale, height, marge);

    cv::line(histImage, cv::Point(center * scale, 0), cv::Point(center * scale, height + marge), cv::Scalar(0, 0, 255), 2, 8);
    cv::line(histImage, cv::Point(x_low * scale, 0), cv::Point(x_low * scale, height + marge), cv::Scalar(0, 255, 0), 2, 8);
    cv::line(histImage, cv::Point(x_high * scale, 0), cv::Point(x_high * scale, height + marge), cv::Scalar(0, 255, 0), 2, 8);
    histImage.copyTo(showImage);
  }

  void draw_histogram(cv::Mat& histogram, cv::Mat& showImage, int scale, int height, int marge, bool inv) {
    cv::Mat hist;
    if (histogram.rows == 1) {
      cv::transpose(histogram, hist);
    } else {
      histogram.copyTo(hist);
    }
    cv::normalize(hist, hist, 0, hist.rows, cv::NORM_MINMAX, -1, cv::Mat());

    double min, max;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(hist, &min, &max, &minLoc, &maxLoc);
    
    int hist_h = (height > 0)? height : cvRound(hist.at<float>(maxLoc));
    float h_scale = hist_h / hist.at<float>(maxLoc); //Make sure histogram fits
    //cout << "MAX: " << cvRound(hist.at<float>(maxLoc)) << " Hist_h: " << hist_h << " H_SCALE: " << h_scale << endl;
    cv::Mat histImage(hist_h + marge, hist.rows * scale, CV_8UC3, cv::Scalar(0, 0, 0));
    
    for (int i = 1; i < hist.rows; i++)
    {
      //cout << "POINT " << cvRound(hist.at<float>(i - 1)) << endl;
      cv::line(histImage, cv::Point(scale * (i - 1), hist_h + cvRound(marge/2) - cvRound(hist.at<float>(i - 1) * h_scale)),
        cv::Point(scale * (i), hist_h + cvRound(marge / 2) - cvRound(hist.at<float>(i) * h_scale)),
        cv::Scalar(255, 0, 0), 2, 8, 0);
    }
    if (inv) {
      cv::transpose(histImage, showImage);
    }
    else {
      histImage.copyTo(showImage);
    }
  }

  void draw_reduce(cv::Mat& inputImage, cv::Mat& showImage, int marge, bool rows, bool cols) {
    if (not rows and not cols) {
      inputImage.copyTo(showImage);
      return;
    }
    cv::Mat row, col, row_h, col_h, color, h_img,v_img;
    cv::cvtColor(inputImage, color, cv::COLOR_GRAY2RGB);
    if (rows) {
      cv::reduce(inputImage, row, 0, cv::REDUCE_SUM, CV_32F);
      debug::draw_histogram(row, row_h, 1, marge, 0, false);
      if (not cols) {
        cv::vconcat(color, row_h, showImage);
        return;
      }
    }
    if (cols) {
      cv::reduce(inputImage, col, 1, cv::REDUCE_SUM, CV_32F);
      debug::draw_histogram(col, col_h, 1, marge, 0, true);
      cv::hconcat(color, col_h, h_img);
      if (rows) {
        cv::Mat corner(marge, marge, CV_8UC3, cv::Scalar(0, 0, 0));
        cv::hconcat(row_h, corner, v_img);
        cv::vconcat(h_img, v_img, showImage);
      }
      else {
        h_img.copyTo(showImage);
      }
    }
    
    
    
  }
}

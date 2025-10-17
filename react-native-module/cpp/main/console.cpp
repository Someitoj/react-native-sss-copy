/*
This file is to run the program as a console application
*/
#include <iostream>
#include <filesystem>
#include <source_location>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <iomanip>

#include "mainfunctions.hpp"
#include "debugfunctions.hpp"
#include "helperfunctions.hpp"
#include "imageprocessing.hpp"
#include "objectdetect.hpp"

using namespace std;

void showCascade(cv::CascadeClassifier classifier, cv::Mat& inputImage, cv::Mat& outputImage);
void testCascade(cv::CascadeClassifier classifier, cv::Mat& inputImage, vector<cv::Rect>& rects);
vector<string> testImages(function<string(ImageHandler, cv::Mat&, string)> process_image, string folder = "C:\\test-images", string assert_file = "C:\\test-images\\assert.txt");

void runCascadeTest(int nr = -1);
void runReconTest(string display_folder = "C:\\test-images\\displays-only", string display_annotation = "C:\\test-images\\displays_annot.txt");
void runFullTest();

void editTest(string folder = "C:\\test-images", vector<string> images = {"example.jpg"});
void editTestSingleImage(ImageHandler ih, string img_name);


int main(){
  cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);
  //runFullTest();
  //runCascadeTest(12);
  //runReconTest();
  //editTest();
  return 0;
}

/*
* This function is for quick tests on a limited set of images
* Set the folder where the images are and what images you'd like to test
*/
void editTest(string folder, vector<string> images) {
  ImageHandler ih = ImageHandler(folder);

  vector<string> images = {
    
  };
  
  for (string img_name : images) {
    editTestSingleImage(ih, img_name);
  }
  
}

/* Test something on a single image */
void editTestSingleImage(ImageHandler ih, string img_name) {
  cv::Mat img = ih.load_image(img_name);
  ih.show_image(img, img_name);
  
  //TODO

}

/* Test the recognition code on all images */
void runReconTest(string display_folder, string display_annotation) {
  std::cout << "Testing Recognition Code" << endl;
  Timer t = Timer::create();
  t.start();
  vector<string> results = testImages([](ImageHandler ih, cv::Mat& inputImage, string assertion) {
    //DO RECOGN
    cv::Mat proc_display;
    processing::preprocess_display(inputImage, proc_display);
    string text = objectdetect::detect_digits(proc_display);
    //ih.show_image(proc_display, assertion);
    return text + " Real: " + assertion;
    }, display_folder
    , display_annotation);
  
  t.end();

  //HERE interpret results
  int count = 0, hit = 0, no_punct=0;
  for (string result : results) {
    count++;
    size_t pos = result.find(" Real: ");
    string outp = result.substr(0, pos);
    string real = result.substr(pos + 7);
    //cout << outp << " " << real << endl;

    if (outp.compare(real) == 0) {
      hit++;
      //cout <<  "HIT: " << result << endl;
    }
    else {
      string short_real = real;
      string short_outp = outp;
      std::erase(short_real, ':');
      std::erase(short_real, '.');
      std::erase(short_outp, ':');
      std::erase(short_outp, '.');
      if (short_outp.compare(short_real) == 0) {
        no_punct++;
        //cout << "PARTIALLY: " << result << endl;
      }
      else {
        //cout << "MISS: " << result << endl;
      }
    }
  }
  std::cout << "Timer " << ConsoleColor::timer << t.getTime() << " sec" << ConsoleColor::normal << " (avg " << ConsoleColor::timer << t.getTime() / count << " sec" << ConsoleColor::normal << ")" << endl;
  std::cout << "Total: " << count << " Hit: " << hit << " (" << ConsoleColor::result << ((double)hit / count) * 100 << "%" << ConsoleColor::normal << ")" << endl;
  std::cout << "Ignoring ':' and '.' " << hit + no_punct << " (" << ConsoleColor::result << ((double)(hit + no_punct) / count) * 100 << "%" << ConsoleColor::normal << ")" << endl;

}

/* Testing the cascade classifier
Multiple classifiers can be tested, -1 to test them all*/
void runCascadeTest(int nr) {
  std::cout << "Testing Cascade Classifier " << endl;
  std::cout << "NOTE: edit the variable TEST_SETS to add several image sources to their annotation file" << endl;
  map<string, string> TEST_SETS = {};
  TEST_SETS["C:\\test-images\\WitBlauw"] = "C:\\test-images\\witblauw_annot.txt";
  TEST_SETS["C:\\test-images\\Rood"] = "C:\\test-images\\rood_annot.txt";
  TEST_SETS["C:\\test-images\\Groen"] = "C:\\test-images\\groen_annot.txt";
  TEST_SETS["C:\\test-images\\Zwart"] = "C:\\test-images\\zwart_annot.txt";

  int start_class = 0;
  int highest_class = 13;
  if (nr != -1) {
    start_class = nr;
    highest_class = nr;
  }
  for (int i = start_class; i < highest_class + 1; i++) {
    cv::CascadeClassifier classifier = cv::CascadeClassifier("C:/Users/Administrator/Desktop/Cascade/GREY/Classifier_" + to_string(i) + "/cascade.xml");
    vector<string> results = {};
    Timer t = Timer::create();
    t.start();
    for (auto const& [key, val] : TEST_SETS) {
      vector<string> temp = testImages([classifier](ImageHandler ih, cv::Mat& inputImage, string assertion) {
        vector<cv::Rect> rects;
        testCascade(classifier, inputImage, rects);
        //Set to TRUE to draw the result
        if (false) {
          cv::Mat showImage;
          debug::drawRectangles(inputImage, showImage, rects);
          ih.show_image(showImage);
        }
        bool hit = false, fp = false;
        string assert_copy = assertion;
        std::size_t pos;
        std::stoi(assert_copy, &pos);
        assert_copy.erase(0, pos);
        int p1 = std::stoi(assert_copy, &pos);
        assert_copy.erase(0, pos);
        int p2 = std::stoi(assert_copy, &pos);
        assert_copy.erase(0, pos);
        int p3 = std::stoi(assert_copy, &pos);
        assert_copy.erase(0, pos);
        int p4 = std::stoi(assert_copy, &pos);
        cv::Rect display = cv::Rect(p1, p2, p3, p4);
        for (cv::Rect rect : rects) {
          double intersect = cv::rectangleIntersectionArea(display, rect);
          if (intersect > 0.0) { hit = true; }
          else { fp = true; }
        }
        string r_hit = hit ? "t" : "f";
        string r_fp = fp ? "t" : "f";
        return r_hit + " " + r_fp + " " + to_string(rects.size());
        }, key
        , val);
      results.insert(results.end(), temp.begin(), temp.end());
    }
    t.end();
    int count = 0, hit = 0, fp = 0, rect_count = 0;
    for (string result : results) {
      count++;
      if (result.at(0) == 't') { hit++; }
      if (result.at(2) == 't') { fp++; }
      rect_count += std::stoi(result.substr(4));
    }
    std::cout << "Classifier " << i << " => " << ConsoleColor::timer << t.getTime() << " sec" << ConsoleColor::normal << " (avg " << ConsoleColor::timer << t.getTime() / count << " sec" << ConsoleColor::normal << ")" << endl;
    std::cout << "Total: " << count << " Hit: " << hit << " (" << ConsoleColor::result << ((double)hit / count) * 100 << "%" << ConsoleColor::normal << ")\tFP: " << fp << " (" << ConsoleColor::result << ((double)fp / count) * 100 << "%" << ConsoleColor::normal << ")" << endl;
    std::cout << "Total Rects: " << rect_count << " (Avg: " << ConsoleColor::result << (double)rect_count / count << ConsoleColor::normal << ")" << endl;
  }
}



vector<string> testImages(function<string(ImageHandler, cv::Mat&, string)> process_image, string folder, string assert_file) {
  vector<string> outputs = {};
  std::map<string, string> assertions = {};
  std::ifstream file(assert_file);
  std::string str;
  while (std::getline(file, str))
  {
    size_t pos = str.find(" ");
    string name = str.substr(0, pos);
    string assert = str.substr(pos + 1);
    assertions[name] = assert;
  }

  ImageHandler ih = ImageHandler(folder);
  std::vector<std::string> imgs = ih.list_images(false);
  for (string img_name : imgs) {
    //cout << img_name << endl;
    cv::Mat img = ih.load_image(img_name);
    outputs.push_back(process_image(ih, img, assertions[img_name]));
  }
  return outputs;
}

void testCascade(cv::CascadeClassifier classifier, cv::Mat& inputImage, vector<cv::Rect>& rects) {
  cv::Mat grey;
  cv::cvtColor(inputImage, grey, cv::COLOR_BGR2GRAY);
  double min = std::min(inputImage.rows, inputImage.cols) / 20;
  double maximum = std::max(inputImage.rows, inputImage.cols) / 3;
  classifier.detectMultiScale(grey, rects, 1.1, 3, 0, cv::Size(min / 2, min), cv::Size(maximum / 2, maximum));
}

void runFullTest() {
  std::cout << "Testing Entire Imageprocessing pipeline " << endl;
  std::cout << "NOTE: edit the variable TEST_SETS to add several image sources to their annotation file" << endl;
  map<string, string> TEST_SETS = {};
  TEST_SETS["C:\\test-images\\WitBlauw"] = "C:\\test-images\\witblauw_annot.txt";
  TEST_SETS["C:\\test-images\\Rood"] = "C:\\test-images\\rood_annot.txt";
  TEST_SETS["C:\\test-images\\Groen"] = "C:\\test-images\\groen_annot.txt";
  TEST_SETS["C:\\test-images\\Zwart"] = "C:\\test-images\\zwart_annot.txt";

  vector<string> results = {};
  cv::CascadeClassifier classifier = cv::CascadeClassifier("C:\\Users\\Administrator\\Documents\\GitHub\\react-native-sss\\cpp\\main\\cascade.xml");
  // For application"..\\cpp\\main\\cascade.xml";
  Timer t = Timer::create();
  t.start();
  for (auto const& [key, val] : TEST_SETS) {
    vector<string> temp = testImages([&classifier](ImageHandler ih, cv::Mat& inputImage, string assertion) {
      string text = functions::process_image(inputImage, classifier);
      //ih.show_image(proc_display, assertion);
      return text + " Real: " + assertion;
      }, key
      , val);
    results.insert(results.end(), temp.begin(), temp.end());
  }
  t.end();


  
  //HERE interpret results
  int count = 0, hit = 0, no_punct = 0;
  for (string result : results) {
    count++;
    size_t pos = result.find(" Real: ");
    string outp = result.substr(0, pos);
    string real = result.substr(pos + 7);
    //cout << outp << " " << real << endl;

    if (outp.compare(real) == 0) {
      hit++;
      //cout <<  "HIT: " << result << endl;
    }
    else {
      string short_real = real;
      string short_outp = outp;
      std::erase(short_real, ':');
      std::erase(short_real, '.');
      std::erase(short_outp, ':');
      std::erase(short_outp, '.');
      if (short_outp.compare(short_real) == 0) {
        no_punct++;
        //cout << "PARTIALLY: " << result << endl;
      }
      else {
        //cout << "MISS: " << result << endl;
      }
    }
  }
  
  std::cout << "Timer " << ConsoleColor::timer << t.getTime() << " sec" << ConsoleColor::normal << " (avg " << ConsoleColor::timer << t.getTime() / count << " sec" << ConsoleColor::normal << ")" << endl;
  std::cout << "Total: " << count << " Hit: " << hit << " (" << ConsoleColor::result << ((double)hit / count) * 100 << "%" << ConsoleColor::normal << ")" << endl;
  std::cout << "Ignoring ':' and '.' " << hit + no_punct << " (" << ConsoleColor::result << ((double)(hit + no_punct) / count) * 100 << "%" << ConsoleColor::normal << ")" << endl;
}

void showCascade(cv::CascadeClassifier classifier, cv::Mat& inputImage, cv::Mat& outputImage) {
  //HOW TO TRAIN A MODEL: open cmd in Cascade Classifier folder
  //C:/opencv/build/x64/vc16/bin/opencv_annotation.exe --annotations=pos_digit.txt --images=pos/ --maxWindowHeight=1000 --resizeFactor=4
  //C:/Users/Administrator/Downloads/opencvcopy/opencv/build/x64/vc15/bin/opencv_createsamples.exe -info pos_digit.txt -w 24 -h 24 -num 1000 -vec pos_digit.vec
  //C:/Users/Administrator/Downloads/opencvcopy/opencv/build/x64/vc15/bin/opencv_traincascade.exe -data Classifier/ -vec pos_digit.vec -bg neg.txt -...(other params)
  vector<cv::Rect> rects;
  cv::Mat grey;
  cv::cvtColor(inputImage, grey, cv::COLOR_BGR2GRAY);
  double min = std::min(inputImage.rows, inputImage.cols) / 20;
  classifier.detectMultiScale(grey, rects, 1.1, 3, 0, cv::Size(min / 2, min));
  debug::drawRectangles(inputImage, outputImage, rects);
}

/*
This file contains functions that don't only fit imageprocessing and objectdetection
*/
#include "helperfunctions.hpp"

#include <opencv2/opencv.hpp>

const int peak_dist = 25;
namespace helperfunctions {
  
  vector<cv::Point> find_local_maxima(cv::InputArray& histogram, const float scale) {
    //https://answers.opencv.org/question/56701/find-all-peaks-for-mat-in-opencv-c/
    // Unlike the original, we asume there will be 2 local maxima. so we look for 2

    cv::Mat hist = histogram.getMat();
    // die if histogram image is not the correct type
    CV_Assert(hist.type() == CV_32F);

    // find the min and max values of the hist image
    double min_val, max_val;
    cv::Point min_pnt, max_pnt;
    cv::minMaxLoc(hist, &min_val, &max_val, &min_pnt, &max_pnt);

    cv::Mat mask;
    cv::GaussianBlur(hist, hist, cv::Size(9, 9), 0); // smooth a bit in order to obtain better result

    // find pixels that are equal to the local neighborhood not maximum (including 'plateaus')
    cv::dilate(hist, mask, cv::Mat());
    cv::compare(hist, mask, mask, cv::CMP_GE);

    // optionally filter out pixels that are equal to the local minimum ('plateaus')

    cv::Mat non_plateau_mask;
    cv::erode(hist, non_plateau_mask, cv::Mat());
    cv::compare(hist, non_plateau_mask, non_plateau_mask, cv::CMP_GT);
    cv::bitwise_and(mask, non_plateau_mask, mask);


    vector<cv::Point> maxima;   // output, locations of non-zero pixels
    cv::findNonZero(mask, maxima);
    cv::Point second_max = min_pnt;//(maxima[0] != max_pnt) ? maxima[0] : maxima[1];
    //cout << max_pnt << " " << maxima << endl;
    
    for (vector<cv::Point>::iterator it = maxima.begin(); it != maxima.end();)
    {
      cv::Point pnt = *it;
      if (pnt == max_pnt || second_max == pnt || pnt.y == 256 /* || pnt.y == 0*/) { it++; continue; }
      float val = hist.at<float>(pnt.y);
      //cout << pnt <<": " << val << endl;
      //cout << "Second: " << hist.at<float>(second_max.y) << endl;
      // filter peaks
      if (scale == 0.0) {
        if ((val > hist.at<float>(second_max.y))
          and (pnt.y - peak_dist > max_pnt.y or pnt.y + peak_dist < max_pnt.y))
        { second_max = pnt; }
        ++it;
      }
      else {
        if ((val > max_val * scale))
          ++it;
        else
          it = maxima.erase(it);
      }
    }
    if (scale == 0.0) {
      cv::minMaxLoc(hist(cv::Range(min(max_pnt.y ,second_max.y), max(max_pnt.y, second_max.y)), cv::Range(0,1)), NULL, NULL, &min_pnt, NULL);
      //cout << "Max: " << max_pnt.y << "\t Second: " << second_max.y<< "\t MIN: " << min_pnt.y + min(max_pnt.y, second_max.y) << endl;
      min_pnt.y += min(max_pnt.y, second_max.y);
      return { max_pnt, second_max, min_pnt };
      //return { cv::Point(0,60), cv::Point(0,60) };
    }
    else {
      return maxima;
    }

  }

  cv::Mat get_histogram_from_gray(cv::Mat& inputImage) {
    cv::Mat hist;
    float range[] = { 0 , 256 };
    const float* histRange[] = { range };
    int histSize = 256;
    cv::calcHist(&inputImage, 1, 0, cv::Mat(), hist, 1, &histSize, histRange, true, false);
    cv::normalize(hist, hist, 0, 256, cv::NORM_MINMAX, -1, cv::Mat());
    return hist;
  }

  vector < vector<vector<int>>> get_peak_matrix_from_gray(cv::Mat& inputImage, vector<float> divide, bool center) {
    vector<vector<int>> peak(divide.size() - 1,vector<int>(divide.size() - 1));
    vector<vector<int>> second(divide.size() - 1, vector<int>(divide.size() - 1));

    double min_val, max_val;
    cv::Point min_pnt, max_pnt;

    for (int i = 0; i < divide.size() - 1; i++) {
      for (int j = 0; j < divide.size() - 1; j++) {
        //cout << "PART [" << i + 1 << "," << j + 1 << "]" << endl;
        cv::Mat part(inputImage(cv::Range(inputImage.rows * divide[i], inputImage.rows * divide[i + 1]), cv::Range(inputImage.cols * divide[j], inputImage.cols * divide[j + 1])));
        cv::Mat hist = get_histogram_from_gray(part);
        vector<cv::Point> loc = find_local_maxima(hist);

        peak[i][j]  = loc[0].y;
        second[i][j] = loc[1].y;
      }
    }
    vector<vector<vector<int>>> result = {peak,second};
    if (center) {
      vector<vector<int>> addon(divide.size() - 1, vector<int>(divide.size() - 1,0));

      cv::Mat part(inputImage(cv::Range(inputImage.rows * 1.0/4, inputImage.rows * 3.0/4), cv::Range(inputImage.cols * 1.0/4, inputImage.cols * 3.0/4)));
      cv::Mat hist = get_histogram_from_gray(part);
      vector<cv::Point> loc = find_local_maxima(hist);
      addon[1][0] = loc[0].y;
      addon[1][1] = loc[1].y;

      hist = get_histogram_from_gray(inputImage);
      loc = find_local_maxima(hist);
      addon[0][0] = loc[0].y;
      addon[0][1] = loc[1].y;

      result.push_back(addon);
    }
    
    return result;
  }

}

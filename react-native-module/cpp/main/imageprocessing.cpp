#include "imageprocessing.hpp"
#include "helperfunctions.hpp"

#include <opencv2/opencv.hpp>
#include <numeric>

using namespace std;
/*
* TODO
* * Make more readable: Confine operations to distinct functions instead of one large megafunction
*/
namespace processing {
  //Helperfunctions (defined below)
  void colour_profiling(cv::Mat& inputImage, cv::Mat& outputImage);
  void crop_border(cv::Mat& inputImage, cv::Mat& outputImage);
  void convert_white_BG(cv::Mat& inputImage, cv::Mat& outputImage);

  //Helperfunctions for Helperfunctions
  void calc_crop(cv::Mat& sum, int& low, int& high, const float alpha = 0.95);
  void calc_crop_aggresive(cv::Mat& sum, int& low, int& high, const float alpha = 0.95);
  vector<int> find_quad_outlier(vector<int> values, const int maxrange);
  void do_matrix_mult(cv::Mat& inputImage, cv::Mat& outputImage, int prim, int sec, int center = -1);
  void apply_mask(cv::Mat& inputImage, cv::Mat& mask, int diff, cv::Mat& outputImage);


  void preprocess_init(cv::Mat& inputImage, cv::Mat& outputImage) {
    inputImage.copyTo(outputImage);
  }

  void preprocess_full(cv::Mat& inputImage, cv::Mat& outputImage) {
    cv::Mat gauss;
    cv::Mat canny;

    cv::GaussianBlur(inputImage, gauss, { 13, 13 }, 0);
    cv::Canny(gauss, outputImage, 50, 100);
    
  }

  void crop_display(cv::Mat& inputImage, cv::Mat& outputImage, vector<cv::Point> rectangle) {
    //CURRENTLY JUST TAKES BOUNDING BOX. SHOULD DO HOMOGRAPHY OR SOMETHING TO ACCOUNT FOR ROTATED DISPLAYS
    int minX = rectangle[0].x;
    int maxX = rectangle[0].x;
    int minY = rectangle[0].y;
    int maxY = rectangle[0].y;
    
    for (int i = 1; i < rectangle.size(); i++) {
      if (rectangle[i].x < minX) { minX = rectangle[i].x; }
      if (rectangle[i].x > maxX) { maxX = rectangle[i].x; }
      if (rectangle[i].y < minY) { minY = rectangle[i].y; }
      if (rectangle[i].y > maxY) { maxY = rectangle[i].y; }
    }
    outputImage = inputImage(cv::Range(minY,maxY), cv::Range(minX, maxX));
  }

  //Todo: remove
  void old_preprocess_display(cv::Mat& inputImage, cv::Mat& outputImage) {
    cv::Mat gauss;
    cv::GaussianBlur(inputImage, gauss, { 13, 13 }, 0);
    cv::inRange(gauss, cv::Scalar(0, 0, 0), cv::Scalar(100, 100, 100), outputImage);
  } 

  void preprocess_display(cv::Mat& inputImage, cv::Mat& outputImage) {
    cv::Mat gray, profiled, white_bg, blur, thresh;
    //TODO: Convert image to grey at start of entire process, instead of repeating it so often.
    //Note: Colour information might have to be used for better processing, so don't do that
    cv::cvtColor(inputImage, gray, cv::COLOR_RGB2GRAY);
    
    
    
    //Colour Profiling: determined what colour the background is, the segments have and convert to black and white.
    colour_profiling(gray,profiled);
    
    // Make sure it has white background, black segments
    convert_white_BG(profiled, white_bg);

    //The following statements reduce noise and make the image binary
    cv::GaussianBlur(white_bg, blur,{13,13}, 0);
    cv::threshold(blur, thresh, 127, 255, cv::THRESH_BINARY);

    //Remove borders aroundif they are present
    crop_border(thresh, outputImage);

    //Return values for debugging purposes
    //white_bg.copyTo(outputImage);
    //thresh.copyTo(outputImage);
    //profiled.copyTo(outputImage);
  }

  void crop_digit(cv::Mat& inputImage, cv::Mat& outputImage, cv::Rect& rectangle) {
    outputImage = inputImage(rectangle);
  }

  void preprocess_digit(cv::Mat& inputImage, cv::Mat& outputImage) {
    cv::Mat resized;

    cv::resize(inputImage, resized, cv::Size(240, 600),0.0,0.0, cv::INTER_LINEAR);
    cv::threshold(resized, outputImage, 128, 255, cv::THRESH_BINARY);
  }
  /*******************
  * Helper Functions *
  ********************/
  /*
   * Inspired by https://github.com/scottmudge/SegoDec/blob/master/segodec.py "apply_brightness_contrast()"
   *
   * Short version: Instead of doing 4 seperate matrix multiplications with static values, we do 1 matrix multiplication.
   * The values we use for this operation are based on the colour of the background and the colour of the segments (hence 'colour profiling').
   *
   * Full explaination detailed in my thesis papers.
   */
  void colour_profiling(cv::Mat& inputImage, cv::Mat& outputImage) {
    const int fixed_row_size = 400;
    double clahe_clip_size = 0.50;
    int new_cols = inputImage.cols * fixed_row_size / inputImage.rows;
    //cout << "FROM " << inputImage.rows << " x " << inputImage.cols << " to " << fixed_row_size << " x " << new_cols << endl;
    cv::resize(inputImage, inputImage, cv::Size(new_cols, fixed_row_size));
    /*
    if (inputImage.rows < 100) { kernel_size = cv::Size(3, 3); clahe_clip_size = 1.0; }
    else if (inputImage.rows < 400) { kernel_size = cv::Size(5, 5); clahe_clip_size = 1.0; }
    else if (inputImage.rows < 700) { kernel_size = cv::Size(7, 7); clahe_clip_size = 1.0; }
    else { kernel_size = cv::Size(9, 9); clahe_clip_size = 3.0; }
    */
    cv::Mat morph;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(4, 4));
    cv::morphologyEx(inputImage, morph, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(morph, inputImage, cv::MORPH_OPEN, kernel);
    bool simple = true;
    if(simple){
      // ORGINAL IMPLEMENTATION (Singular BG calculation)
      cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clahe_clip_size, cv::Size(9, 9));
      //clahe->apply(inputImage, inputImage);
      //cv::morphologyEx(inputImage, morph, cv::MORPH_CLOSE, kernel);
      //cv::morphologyEx(morph, inputImage, cv::MORPH_OPEN, kernel);
      cv::Mat sample(inputImage(cv::Range(inputImage.rows / 4, inputImage.rows * 3 / 4), cv::Range(inputImage.cols / 4, inputImage.cols * 3 / 4)));
      
      cv::Mat hist = helperfunctions::get_histogram_from_gray(sample);

      vector<cv::Point> peaks = helperfunctions::find_local_maxima(hist);

      do_matrix_mult(inputImage, outputImage, peaks[0].y, peaks[1].y, peaks[2].y);
      
      //inputImage.copyTo(outputImage);
      return;
    }
    
    vector<vector<vector<int>>> peakmatrix = helperfunctions::get_peak_matrix_from_gray(inputImage);

    //Parsing the peakmatrix to a more readable form {full, quad1, quad2, quad3, quad4, center}
    vector<int> primary   = { peakmatrix[2][0][0], peakmatrix[0][0][0], peakmatrix[0][0][1], peakmatrix[0][1][0], peakmatrix[0][1][1], peakmatrix[2][1][0] };
    vector<int> secondary = { peakmatrix[2][0][1], peakmatrix[1][0][0], peakmatrix[1][0][1], peakmatrix[1][1][0], peakmatrix[1][1][1], peakmatrix[2][1][1] };
    const int max_range = 10;

    // 1) Swap around values to achieve the following: all lowest peaks in primary and all highest in secondary
    int prim_sum = 0, sec_sum = 0;
    int default_vals = 0;
    for (int i = 0; i < 6; i++) {
      if (primary[i] > secondary[i]) { swap(primary[i], secondary[i]); }
      if (primary[i] == 0) {
        //Default value for secondary peak is 0. So if no secondary peak is found, that will be the value.
        default_vals++;
      }
      else {
        prim_sum += primary[i];
        sec_sum += secondary[i];
      }
      
    }
    if (default_vals == 6) {
      //cout << "ERROR: All secondary peaks are 0" << endl;
      //If all primary peaks are 0, it's either because the peak is actually 0 or there is just only a single peak
      primary = { 1,1,1,1,1,1 };
      //inputImage.copyTo(outputImage);
      //return;
    }
    double prim_avg = prim_sum / double(6 - default_vals);
    double sec_avg = sec_sum / double(6 - default_vals);

    // 2) See if all peaks are concistent.
    // If there are default values, we also check if the found value falls in line with either of the vectors.
    //!!! AVERAGE IS NOT A GOOD METRIC!!! need outlier detection. standart deviation ?
    
    bool is_monotone = true;
    for (int i = 0; i < 6; i++) {
      if (primary[i] == 0) {
        if (secondary[i] < prim_avg + max_range and secondary[i] > prim_avg - max_range) {
          swap(primary[i], secondary[i]);
          is_monotone = false;
        }
        else if (secondary[i] > sec_avg + max_range or secondary[i] < sec_avg - max_range) {
          is_monotone = false;
        }

      }
      else {
        if (primary[i] > prim_avg + max_range or primary[i] < prim_avg - max_range) {
          is_monotone = false;
          //cout << "Prim " << i << " VAL: " << primary[i] << "AVG: " << prim_avg << endl;
        }
        if (secondary[i] > sec_avg + max_range or secondary[i] < sec_avg - max_range) {
          is_monotone = false;
          //cout << "Sec " << i << " VAL: " << secondary[i] << "AVG: " << sec_avg << endl;
        }
      }

    }

    if (is_monotone) {
      //Classic implentation!
      //cout << "classic" << endl;
      //do_matrix_mult(inputImage, outputImage, prim_avg, sec_avg);
      
      cv::Mat sample(inputImage(cv::Range(inputImage.rows / 4, inputImage.rows * 3 / 4), cv::Range(inputImage.cols / 4, inputImage.cols * 3 / 4)));
      cv::Mat hist = helperfunctions::get_histogram_from_gray(sample);

      vector<cv::Point> peaks = helperfunctions::find_local_maxima(hist);

      do_matrix_mult(inputImage, outputImage, peaks[0].y, peaks[1].y, peaks[2].y);
      
    }
    else {
      cout << "NOT implemented yet " << endl;
      inputImage.copyTo(outputImage);
      //Handel the imperfect images
      /*
      //attempt 1: each quadrant seperate
      vector<cv::Mat> parts = { inputImage(cv::Range(0, inputImage.rows * 0.5),               cv::Range(0, inputImage.cols * 0.5)),
                                inputImage(cv::Range(0, inputImage.rows * 0.5),               cv::Range(inputImage.cols * 0.5, inputImage.cols)),
                                inputImage(cv::Range(inputImage.rows * 0.5, inputImage.rows), cv::Range(0, inputImage.cols * 0.5)),
                                inputImage(cv::Range(inputImage.rows * 0.5, inputImage.rows), cv::Range(inputImage.cols * 0.5, inputImage.cols)) };
      vector<cv::Mat> profs(4, cv::Mat());
      for (int i = 0; i < 4; i++) {
        int prim = primary[i + 1], sec = secondary[i + 1];
        if (prim == 0) { prim = prim_avg;}
        if (sec == 0) { sec = sec_avg;}

        do_matrix_mult(parts[i], profs[i], prim, sec);
      }
      cv::Mat h1, h2;
      cv::hconcat(profs[0], profs[1], h1);
      cv::hconcat(profs[2], profs[3], h2);
      cv::vconcat(h1, h2, outputImage);
      
      //Attempt 2: just apply clahe
      //cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(3.0, cv::Size(3, 3));
      //clahe->apply(inputImage, inputImage);


      //inputImage.copyTo(outputImage);

      //WHERE is the outlier ?
      //Several factors: possible edges, bright center
      //If only the center is different, we might want to take steps to address that one
      

      //Are the 4 quadrants mono ?
      int quad_prim_sum = std::accumulate(primary.begin() + 1,primary.end() - 1, 0);
      int quad_sec_sum = std::accumulate(secondary.begin() + 1, secondary.end() - 1, 0);
      double quad_prim_avg = quad_prim_sum / 5.0;
      double quad_sec_avg = quad_sec_sum / 5.0;
      bool quad_mono = true;
      for (int i = 1; i < 5;i++) {
        if (primary[i] > quad_prim_avg + max_range or primary[i] < quad_prim_avg - max_range) {
          quad_mono = false;
          break;
        }
        if (secondary[i] > quad_sec_avg + max_range or secondary[i] < quad_sec_avg - max_range) {
          quad_mono = false;
          break;
        }
      }
      if (quad_mono) {
        //cout << "QUAD MONO" << endl;
        do_matrix_mult(inputImage, outputImage, quad_prim_avg, quad_sec_avg);
      }
      else {
        //FIND Problamatic quadrants
        vector<int> prim_out = find_quad_outlier(vector<int>(primary.begin() + 1, primary.end() - 1), max_range);
        vector<int> sec_out =  find_quad_outlier(vector<int>(secondary.begin() + 1, secondary.end() - 1), max_range);
        
        //cout << "Prim groups: " << *max_element(prim_out.begin(), prim_out.end()) << " Sec groups: " << *max_element(sec_out.begin(), sec_out.end()) << endl;
        //cout << "Equal: " << equal(prim_out.begin(), prim_out.end(), sec_out.begin(), sec_out.end()) << endl;
        
        //CASE 1: both peaks differ in the same quadrants
        if (equal(prim_out.begin(), prim_out.end(), sec_out.begin(), sec_out.end())) {
          
        }
        
        //CALC max diff
        primary.erase(remove(primary.begin(), primary.end(), 0), primary.end());
        secondary.erase(remove(secondary.begin(), secondary.end(),0), secondary.end());
        int max_prim = *max_element(primary.begin(), primary.end());
        int max_sec = *max_element(secondary.begin(), secondary.end());
        int min_prim = *min_element(primary.begin(), primary.end());
        int min_sec = *min_element(secondary.begin(), secondary.end());
        int prim_avg = (max_prim + max_sec) / 2;
        int sec_avg = (min_prim + min_sec) / 2;

        //int center = (prim + sec) / 2;
        //float buffer = abs(prim - sec) / (float)6;

        int x_low = sec_avg;
        int x_high = prim_avg;

        float alpha = 255 / (max(x_high - x_low, 1));
        float gamma = alpha * -1 * x_low;
        cv::Mat mask, temp;
        cv::addWeighted(inputImage, alpha, inputImage, 0, gamma, mask);
        int diff = prim_avg - sec_avg;
        //cout << "DIFF: " << diff << endl;
        //do_matrix_mult(inputImage, outputImage, prim_avg, sec_avg);
        apply_mask(inputImage, mask, 4*diff, temp);
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(3.0, cv::Size(5, 5));
        clahe->apply(temp, temp);
        //cv::equalizeHist(temp, temp);
        
        cv::Mat morph;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(5, 5));
        cv::morphologyEx(temp, morph, cv::MORPH_CLOSE, kernel);
        cv::morphologyEx(morph, temp, cv::MORPH_OPEN, kernel);
        
        cv::Mat sample(temp(cv::Range(inputImage.rows / 4, inputImage.rows * 3 / 4), cv::Range(inputImage.cols / 4, inputImage.cols * 3 / 4)));
        cv::Mat hist = helperfunctions::get_histogram_from_gray(sample);

        vector<cv::Point> peaks = helperfunctions::find_local_maxima(hist);

        do_matrix_mult(temp, outputImage, peaks[0].y, peaks[1].y, peaks[2].y);
        //do_matrix_mult(temp, outputImage, (max_prim + min_prim) / 2, (max_sec + min_sec) / 2);
        temp.copyTo(outputImage);
        
      }
      */
    }

  }

  void apply_mask(cv::Mat& inputImage, cv::Mat& mask, int diff, cv::Mat& outputImage) {
    cv::Mat calcMask(mask);
    cv::bitwise_not(calcMask, calcMask);
    cv::multiply(diff/255.0, calcMask, calcMask);
    cv::Mat morph;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(inputImage.cols/6, inputImage.rows / 6));
    cv::morphologyEx(calcMask, morph, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(morph, calcMask, cv::MORPH_OPEN, kernel);
    //calcMask = (mask / 255) * diff;
    //cv::bitwise_not(calcMask, calcMask);
    cv::add(inputImage,calcMask,outputImage);
    //cv::equalizeHist(outputImage, outputImage);
    calcMask.copyTo(outputImage);
  }

  void do_matrix_mult(cv::Mat &inputImage, cv::Mat &outputImage, int prim, int sec, int center) {

    int x_low;
    int x_high;
    if (center == -1) {
      int cent = (prim + sec) / 2;
      float buffer = abs(prim - sec) / (float)6;

      x_low = cent - buffer;
      x_high = cent + buffer;
    }
    else {
      x_low = center - 5;
      x_high = center + 5;
    }

    float alpha = 255 / (max(x_high - x_low, 1));
    float gamma = alpha * -1 * x_low;

    cv::addWeighted(inputImage, alpha, inputImage, 0, gamma, outputImage);
  }
  //this is to find the outlier among the quadrants
  vector<int> find_quad_outlier(vector<int> values, const int maxrange) {
    vector<int> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    vector<int> grouped(4, 0);
    //cout << sorted[0] << " " << sorted[1] << " " << sorted[2] << " " << sorted[3] << " " << endl;
    int group = 0;
    int index = std::find(values.begin(), values.end(), sorted[0]) - values.begin();
    grouped[index] = group;
    for (int i = 1; i < sorted.size(); i++) {
      if (sorted[i] - sorted[i - 1] > maxrange) {
        group++;
      }
      index = std::find(values.begin(), values.end(), sorted[i]) - values.begin();
      if (sorted[i] == sorted[i - 1]) {
        index = std::find(values.begin() + index + 1, values.end(), sorted[i]) - values.begin();
      }
      grouped[index] = group;     
    }
    return grouped;
  }

  void convert_white_BG(cv::Mat& inputImage, cv::Mat& outputImage) {
    cv::Mat hist;
    hist = helperfunctions::get_histogram_from_gray(inputImage);
    vector<cv::Point> peaks = helperfunctions::find_local_maxima(hist);
    
    if (peaks[0].y < peaks[1].y) {
      cv::bitwise_not(inputImage, outputImage);
    }
    else {
      inputImage.copyTo(outputImage);
    }
  }

  void crop_border(cv::Mat& inputImage, cv::Mat& outputImage) {
    cv::Mat col_sum, row_sum;
    int x_low, x_high, y_low, y_high;

    cv::reduce(inputImage, row_sum, 1, cv::REDUCE_SUM, CV_32F);
    calc_crop_aggresive(row_sum, y_low, y_high);

    cv::reduce(inputImage(cv::Range(y_low, y_high), cv::Range::all()), col_sum, 0, cv::REDUCE_SUM, CV_32F);
    calc_crop_aggresive(col_sum, x_low, x_high);

    cv::Mat cropped = inputImage(cv::Range(y_low, y_high), cv::Range(x_low, x_high));
    /* // DRAW instead of crop
    cv::Mat showImg;
    cv::cvtColor(inputImage, showImg, cv::COLOR_GRAY2RGB);

    cv::line(showImg, cv::Point(0, y_high), cv::Point(inputImage.cols, y_high), cv::Scalar(255, 0, 255), 3);
    cv::line(showImg, cv::Point(0, y_low), cv::Point(inputImage.cols, y_low), cv::Scalar(0, 255, 0), 3);
    cv::line(showImg, cv::Point(x_high, 0), cv::Point(x_high, inputImage.rows), cv::Scalar(255, 0, 255), 3);
    cv::line(showImg, cv::Point(x_low, 0), cv::Point(x_low, inputImage.rows), cv::Scalar(0, 255, 0), 3);

    //cv::line(showImg, cv::Point(0, inputImage.rows / 2), cv::Point(inputImage.cols, inputImage.rows / 2), cv::Scalar(0, 0, 255), 3);
    //cv::line(showImg, cv::Point(inputImage.cols / 2, 0), cv::Point(inputImage.cols / 2, inputImage.rows), cv::Scalar(0, 0, 255), 3);

    showImg.copyTo(outputImage);
    */
    cropped.copyTo(outputImage);
  }

  //Prevent repeated code in crop_border
  void calc_crop(cv::Mat& sum, int& low, int& high, const float alpha) {
    double min_val_l, max_val_l, threshold_l;
    double min_val_h, max_val_h, threshold_h;
    int sum_len;

    if (sum.rows == 1) {
      sum_len = sum.cols;
      cv::minMaxLoc(sum(cv::Range::all(), cv::Range(0, sum_len / 2)), &min_val_l, &max_val_l);
      cv::minMaxLoc(sum(cv::Range::all(), cv::Range(sum_len / 2, sum_len - 1)), &min_val_h, &max_val_h);
    }
    else {
      sum_len = sum.rows;
      cv::minMaxLoc(sum(cv::Range(0, sum_len / 2), cv::Range::all()), &min_val_l, &max_val_l);
      cv::minMaxLoc(sum(cv::Range(sum_len / 2, sum_len - 1), cv::Range::all()), &min_val_h, &max_val_h);
    }

    low = 0;
    high = sum_len - 1;

    threshold_l = min_val_l + alpha * (max_val_l - min_val_l);
    threshold_h = min_val_h + alpha * (max_val_h - min_val_h);

    for (int i = 0; i < sum_len / 2;i++) {
      if (sum.at<float>(i) > threshold_l) {
        low = i;
        break;
      }
    }
    for (int j = sum_len - 1; j > (sum_len / 2);j--) {
      if (sum.at<float>(j) > threshold_h) {
        high = j;
        break;
      }
    }
  }

  //CHANGE:
  //Will crop closer to digits, instead of stopping once past borders
  void calc_crop_aggresive(cv::Mat& sum, int& low, int& high, const float alpha) {
    double min_val_l, max_val_l, threshold_l;
    double min_val_h, max_val_h, threshold_h;
    int sum_len;

    if (sum.rows == 1) {
      sum_len = sum.cols;
      cv::minMaxLoc(sum(cv::Range::all(), cv::Range(0, sum_len / 2)), &min_val_l, &max_val_l);
      cv::minMaxLoc(sum(cv::Range::all(), cv::Range(sum_len / 2, sum_len - 1)), &min_val_h, &max_val_h);
    }
    else {
      sum_len = sum.rows;
      cv::minMaxLoc(sum(cv::Range(0, sum_len / 2), cv::Range::all()), &min_val_l, &max_val_l);
      cv::minMaxLoc(sum(cv::Range(sum_len / 2, sum_len - 1), cv::Range::all()), &min_val_h, &max_val_h);
    }

    low = 0;
    high = sum_len - 1;
    int low_shift = 0, high_shift = sum_len - 1;

    threshold_l = min_val_l + alpha * (max_val_l - min_val_l);
    threshold_h = min_val_h + alpha * (max_val_h - min_val_h);

    const float shift_val = 0.2;

    float last_value = sum.at<float>(0);
    bool bg_l = false, bg_h = false;
    for (int i = 0; i < sum_len / 2;i++) {
      if (low_shift == 0 and (sum.at<float>(i) > last_value * (1.0 + shift_val) or sum.at<float>(i) < last_value * (1.0 - shift_val))) {
        low_shift = i;
      }
      if (sum.at<float>(i) > threshold_l) {
        bg_l = true;
        low = i;
      }
      else if (bg_l) {
        if (i > (sum_len / 3)) {
          low = low_shift;
        }
        break;
      }
    }
    last_value = sum.at<float>(sum_len - 1);
    for (int j = sum_len - 1; j > (sum_len / 2);j--) {
      if (high_shift == sum_len - 1 and (sum.at<float>(j) > last_value * (1.0 + shift_val) or sum.at<float>(j) < last_value * (1.0 - shift_val))) {
        high_shift = j;
      }
      if (sum.at<float>(j) > threshold_h) {
        bg_h = true;
        high = j;
      }
      else if (bg_h) {
        if (j < (sum_len * 2 / 3)) {
          high = high_shift;
        }
        break;
      }
    }
  }

  

}

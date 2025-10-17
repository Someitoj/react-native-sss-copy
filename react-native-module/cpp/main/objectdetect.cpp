#include "objectdetect.hpp"
#include "helperfunctions.hpp"

#include <opencv2/opencv.hpp>

using namespace std;

/*************************
* Local Helper Functions *
*************************/

const int MIN_LENGTH = 10; //How many pixels a side of a rect must at least be to be concidered a display.
const int CLOSE_LENGTH = 5; // How many pixels apart do two point have to be to be concidered close ?

//Is a given contour possibly a display ?
static bool is_display(vector<cv::Point>& contour, vector<cv::Point>& approx, cv::Moments& moments ,vector<vector<cv::Point>>& rects, vector<cv::Moments>& moments_list) {
  //Is it a rectangle ?
  if (approx.size() != 4) { return false; }

  //Minimum length ?
  if (cv::norm(approx[0] - approx[1]) < MIN_LENGTH
    || cv::norm(approx[1] - approx[2]) < MIN_LENGTH
    || cv::norm(approx[2] - approx[3]) < MIN_LENGTH
    || cv::norm(approx[3] - approx[0]) < MIN_LENGTH
    ) {
    return false;
  }
  

  //Minimum Area ?
  if (moments.m00 < MIN_LENGTH * MIN_LENGTH) { return false; }

  //Prevent same rect from being detected twice
  // TODO: it seems contours like to give a clockwise and counterclockwise contour version of the same object. Simpler solution ?
  for (int i = 0; i < rects.size(); i++) {
    //Compare size of area
    //cout << "list area: " << moments_list[i].m00 << " new area: " << moments.m00 << endl;
    if (!(moments_list[i].m00 * 1.1 > moments.m00 && moments_list[i].m00 / 1.1 < moments.m00)) { continue; }

    //Compare centroid
    cv::Point cent_i = cv::Point(int(moments_list[i].m10 /moments_list[i].m00), int(moments_list[i].m01 / moments_list[i].m00));
    cv::Point cent   = cv::Point(int(moments.m10 / moments.m00), int(moments.m01 / moments.m00));
    //cout << "list centre: " << cent_i << " new centre: " << cent << endl;
    if (cv::norm(cent_i - cent) > CLOSE_LENGTH) { continue; }

    //Finally, see if points are close
    //This is still more efficient than having the entire rest of the program run twice on the same rectangle
    bool full_match = true;
    for (cv::Point p : approx) {
      bool point_match = false;
      for (cv::Point p_i : rects[i]) {
        if (cv::norm(p_i - p) < CLOSE_LENGTH) {
          point_match = true;
          break;
        }
      }
      if (!point_match) {
        full_match = false;
        break;
      }
    }
    // TODO: If they match which one do i want to save ?
    if (full_match) { return false; }
  }

  return true;
}

bool horizon_align(cv::Rect& r1, cv::Rect& r2) {
  //We assume one rect does not contain another

  //For it to be horizontally alligned, either one or both need to be fully contained
  // or halve of the box needs to overlap
  //THUS => look if the center of the other one is contained
  int center_y_1 = r1.y + (r1.height / 2), center_y_2 = r2.y + (r2.height / 2);
  if ((r1.y < center_y_2 and r1.y + r1.height > center_y_2)
    or (r2.y < center_y_1 and r2.y + r2.height > center_y_1)) {
    //This is left open in case more things need to added.
    //Otherwise this function could just be "return (r1.y < center_y_2 and r1.y + r1.height > center_y_2) or (r2.y < center_y_1 and r2.y + r2.height > center_y_1);"
    return true;
  }
  else {
    return false;
  }
}

//
static bool is_rect_digit(cv::Rect r) {
  //Correct Shape ?
  if (r.width * 1.5 > r.height || r.width * 2.5 < r.height) { return false; }
  //Not to small ?
  if (r.width < MIN_LENGTH || r.height < MIN_LENGTH) { return false; }

  return true;
}
/** Lookuptable for 7 segment digit. Return 'X' if not found.
* 
* . -1- .
* 
* 6 . . 2
* 
* . -7- .
* 
* 5 . . 3
* 
* . -4- .
* 
* @param digit - A number representing the active digits if represented as binary.
* 
*   Example: Digit 3 represented by sequence 1 1 1 1 1 0 0 1
* 
*   1 1 1 1 1 0 0 1 = 64 + 32 + 16 + 8 + 0 + 0 + 1 = 121
* 
*/
static char digit_lookup(int digit) {
  switch (digit) {
    case(126): return '0'; //1 1 1 1 1 1 0
    case( 48): return '1'; //0 1 1 0 0 0 0
    case(109): return '2'; //1 1 0 1 1 0 1
    case(121): return '3'; //1 1 1 1 0 0 1
    case( 51): return '4'; //0 1 1 0 0 1 1
    case( 91): return '5'; //1 0 1 1 0 1 1
    case( 95): return '6'; //1 0 1 1 1 1 1
    case(112): return '7'; //1 1 1 0 0 0 0
    case(127): return '8'; //1 1 1 1 1 1 1
    case(123): return '9'; //1 1 1 1 0 1 1

    case(  1): return '-'; //0 0 0 0 0 0 1
    case(  8): return '_'; //0 0 0 1 0 0 0
    case(119): return 'A'; //1 1 1 0 1 1 1
    case(125): return 'a'; //1 1 1 1 1 0 1
    case( 31): return 'b'; //0 0 1 1 1 1 1
    case( 78): return 'C'; //1 0 0 1 1 1 0
    case( 13): return 'c'; //0 0 0 1 1 0 1
    case( 61): return 'd'; //0 1 1 1 1 0 1
    case( 79): return 'E'; //1 0 0 1 1 1 1
    case(111): return 'e'; //1 1 0 1 1 1 1
    case( 71): return 'F'; //1 0 0 0 1 1 1
    case( 94): return 'G'; //1 0 1 1 1 1 0
    case( 55): return 'H'; //0 1 1 0 1 1 1
    case( 23): return 'h'; //0 0 1 0 1 1 1
    case( 16): return 'i'; //0 0 1 0 0 0 0
    case( 60): return 'J'; //0 1 1 1 1 0 0
    case( 56): return 'j'; //0 1 1 1 0 0 0
    case( 14): return 'L'; //0 0 0 1 1 1 0
    case( 21): return 'n'; //0 0 1 0 1 0 1
    case( 29): return 'o'; //0 0 1 1 1 0 1
    case(103): return 'P'; //1 1 0 0 1 1 1
    case(  5): return 'r'; //0 0 0 0 1 0 1
    case( 15): return 't'; //0 0 0 1 1 1 1
    case( 62): return 'U'; //0 1 1 1 1 1 0
    case( 28): return 'u'; //0 0 1 1 1 0 0
    case( 59): return 'y'; //0 1 1 1 0 1 1
    //TODO: add more characters?
    default: return 'X';
  }
}

const int segment_value[7] = { 64,32,16,8,4,2,1 }; //For summation

namespace objectdetect {
  void cascade_classifier(cv::Mat& inputImage, cv::Mat& outputImage, cv::CascadeClassifier &classifier) {
    

    //Do Classifier Detection
    vector<cv::Rect> rects;
    cv::Mat grey;
    cv::cvtColor(inputImage, grey, cv::COLOR_BGR2GRAY);
    double minimum = std::min(inputImage.rows, inputImage.cols) / 20;
    //double maximum = std::max(inputImage.rows, inputImage.cols) / 3;
    classifier.detectMultiScale(grey, rects, 1.1, 3, 0, cv::Size(minimum / 2, minimum)/*, cv::Size(maximum / 2, maximum)*/);

    //HERE CLUSTER GROUPED RECTS
    //We want them to be horizontally aligned
    if (rects.size() < 1) {
      //cout << "NOTHING DETECTED" << endl;
      //inputImage.copyTo(outputImage);
    }
    else if (rects.size() == 1) {
      //cout << "SINGLE DETECTION" << endl;
      inputImage(rects[0]).copyTo(outputImage);
    }
    else {
      //Rects need to be HORIZONTALLY aligned, since often digits appear next to each other
      //SORT rects by center y value
      sort(rects.begin(), rects.end(), [](cv::Rect& r1, cv::Rect& r2) {
        return r1.y + (r1.height / 2) < r2.y + (r2.height / 2);
        });

      vector<int> team(rects.size(), 0);
      int most_occ_team = 0, most_occ = 0, counter = 1;
      for (int i = 1;i < rects.size();i++) {
        if (horizon_align(rects[i - 1], rects[i])) {
          team[i] = team[i - 1];
          counter++;
        }
        else {
          team[i] = team[i - 1] + 1;
          if (counter > most_occ) {
            most_occ_team = team[i - 1];
            most_occ = counter;
          }
          counter = 0;
        }
        //cout << team[i] << endl;
      }
      if (counter > most_occ) {
        most_occ_team = team[team.size() - 1];
        most_occ = counter;
      }
      //Loop through rects again, only the ones on the winning team, to create a surrounding rect

      int x = -1, y = 0, x2 = 0, y2 = 0;
      for (int i = 0;i < rects.size();i++) {
        if (team[i] == most_occ_team) {
          //WHY min and max ? bounding boxes can go outside image
          if (x == -1) {
            x = max(rects[i].x, 0);
            y = max(rects[i].y, 0);
            x2 = min(rects[i].x + rects[i].width, inputImage.cols);
            y2 = min(rects[i].y + rects[i].height, inputImage.rows);
          }
          else {
            if (x > rects[i].x) { x = max(rects[i].x, 0); }
            if (y > rects[i].y) { y = max(rects[i].y, 0); }
            if (x2 < rects[i].x + rects[i].width) { x2 = min(rects[i].x + rects[i].width, inputImage.cols); }
            if (y2 < rects[i].y + rects[i].height) { y2 = min(rects[i].y + rects[i].height, inputImage.rows); }
          }
        }
      }
      cv::Rect surrounding_rect(x, y, x2 - x, y2 - y);
      outputImage = inputImage(surrounding_rect);
    }
  }

  void detect_display(cv::Mat& inputImage, vector<vector<cv::Point>>& rectangles){
    vector<vector<cv::Point>> contours;
    vector<cv::Moments> moments_list;
    
    cv::findContours(inputImage,contours,cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    //cv::drawContours(inputImage, contours, -1, cv::Scalar(0, 0, 0),5);
    for (vector<cv::Point> contour : contours) {
      vector<cv::Point> approx;
      cv::approxPolyDP(contour, approx, 0.01 * cv::arcLength(contour, true), true);
      cv::Moments moments = cv::moments(contour);
      if (moments.m00 != 0) {
        //Here some logic has to come that decides if the area could be a display.
        if (is_display(contour, approx, moments, rectangles, moments_list)) {
          rectangles.push_back(approx);
          moments_list.push_back(moments);
        }
      }
    }
  }
  //NOTE: Back-to-Front!!!
  void set_values(cv::Mat& array, int start, int end, float value) {
    for (int i = start; i > end; i--) {
      array.at<float>(i) = value;
    }
  }

  /*Extracts info from a halve image
  * 
  * @param[in] inputHalve - Half of a black and white display
  * @param[out] outputArray - 1 dimentional array with information
  *
  * 255 => Nothing here
  * 200 => Noise
  * 150 => Other
  * 100 => Horizontal Segment
  * 0 => Vertical Segment
  */
  void determine_halve(cv::Mat& inputHalve, cv::Mat& outputArray) {
    cv::Mat sum_histogram;
    cv::reduce(inputHalve, sum_histogram, 0, cv::REDUCE_SUM, CV_32F);
    double min_val, max_val;
    cv::minMaxLoc(sum_histogram, &min_val, &max_val);

    double threshold_vert = min_val + 0.3 * (max_val - min_val);
    double threshold_nothing = min_val + 0.9 * (max_val - min_val);

    cv::Mat copy(sum_histogram);
    int start = 0, end = sum_histogram.cols - 1;


    for (int i = sum_histogram.cols - 1; i >= 0; i--) {
      float value = sum_histogram.at<float>(i);
      if (value < threshold_vert) {
        copy.at<float>(i) = (float)0;
        if (start != 0) {
          end = i;
        }
      } else if (value > threshold_nothing) {
        copy.at<float>(i) = (float)255;
        if (start != 0) {
          end = i;
        }
      } else {
        if (start != 0){
        
        } else {
          start = i;
        }
      }
      if (end != sum_histogram.cols - 1) {
        cv::Mat mean, stdDev;
        if (start - end < 5) {
          set_values(copy, start, end, (float)255);
        }
        else {
          //cv::meanStdDev(sum_histogram(cv::Range::all(),cv::Range(end,start)), mean, stdDev);
          //cout << "FROM " << end << " TO " << start << endl;
          //cout << "MEAN: " << mean << "\tSTDDEV: " << stdDev;
          //cout << "\tCOMP: " << std::ceil((mean.at<double>(0) / stdDev.at<double>(0) * 100.0)) / 100.0 << endl;
          //TODO: Actually use standart deviation to cut out noise if needed ?

          set_values(copy, start, end, (float)150);
        }
        
        start = 0;
        end = sum_histogram.cols - 1;
      }
    }
    copy.copyTo(outputArray);
  }

  string recognise_thin(cv::Mat& inputImage) {
    //cv::imshow("Display window", inputImage);
    //cv::waitKey(0);
    //cv::destroyAllWindows();
    
    //Here we'll check for '1', '.' or ':' as these are the most common
    cv::Mat sum_histogram;
    cv::reduce(inputImage, sum_histogram, 1, cv::REDUCE_SUM, CV_32F);
    double min_val, max_val;
    cv::minMaxLoc(sum_histogram, &min_val, &max_val);
    bool started = false;
    int first = sum_histogram.rows;
    int points = 0;
    for (int i = 0; i < sum_histogram.rows;i++) {
      if (started) {
        //Something
        if (sum_histogram.at<float>(i) < max_val/2) {
          continue;
        }
        //Nothing
        else {
          points++;
          first = sum_histogram.rows;
          started = false;
        }
      }
      else {
        //Something
        if (sum_histogram.at<float>(i) < max_val / 2) {
          first = i;
          started = true;
        }
        //Nothing
        else {
          continue;
        }
      }
      
    }
    switch (points) {
      case 1: return ".";
      case 2: return ":";
      default: return "";
    }
  }

  string detect_digits(cv::Mat& inputImage) {
    //INPUT is a BLACK AND WHITE img
    //detect_digits_old(inputImage,digits);
    if (inputImage.rows < 20 or inputImage.cols < 20) {
      //cout << "Error: Cropped Displat too small! (rows: " << inputImage.rows << ", cols: " << inputImage.cols << ")" << endl;
      return "ERROR";
    }
    //The max length a segment can be, is the height of the image divided by 2
    int max_len = inputImage.rows * 1.1 / 2;

    //1 Cut image in halve
    cv::Mat upper(inputImage, cv::Range(0, inputImage.rows / 2), cv::Range::all());
    cv::Mat lower(inputImage, cv::Range(inputImage.rows / 2 + 1, inputImage.rows), cv::Range::all());

    //For each halve, determine the existance of segments
    cv::Mat low_arr, up_arr;
    determine_halve(lower, low_arr);
    determine_halve(upper, up_arr);

    cv::Mat comb;
    cv::min(low_arr, up_arr, comb);

    string text = "";

    //Aside from '-' and '_', all digits have atleast a vertical segment. Shown in the combined sum as 255
    int hor = 0, ver = 0; //Indexes for possible start horizontal/vertical segment
    bool second_ver = false; //Only used if there are 2 vertical segments
    bool started = false,finished = false;
    //Basically try to find positions where digits could be and apply recognise_digit to them.
    for (int i = comb.cols - 1;i >= 0;i--) {
      if (started) {
        int start = 0, end = 0;
        if (second_ver) {
          if (comb.at<float>(i) > (float)50) {
            start = max(ver, hor);
            end = i;
            finished = true;
            second_ver = false;
          }
        }
        //If NOTHING
        else if (comb.at<float>(i) > (float)175) {
          start = max(ver, hor);
          end = i;
          finished = true;
        }
        else {
          //If VERT
          if (comb.at<float>(i) <= (float)50) {
            //Already a vert ?
            if (ver != 0) {
              //In the middle of a vert
              if (comb.at<float>(i + 1) <= (float)50) {
                continue;
              }
              // A second vertical segment
              else {
                if (ver - i > max_len) {
                  //cout << "POSSIBLE ERROR: LONG DISTANCE BETWEEN VERT" << endl;
                }
                second_ver = true;
              }
            }
            //No vert yet
            else {
              ver = i;
              //In case there is a very short hor.
              if (hor != 0) {
                if (hor - i < max_len * 0.2) {
                  //Very closeby, probably just the tip of the vertical segment => ignore
                }
                else if (hor - i < max_len * 0.7) {
                  //This might be something
                  //cout << "From " << i << " to " << hor << " maybe something" << endl;
                  //TODO: Cut out seperately anf pass to small check
                  cv::Mat digit = inputImage(cv::Range::all(), cv::Range(i, hor));
                  string s = recognise_thin(digit);
                  //cout << s << endl;
                  text = s + text;
                  hor = 0;
                }
              }
            }
          }
          //If HOR
          else {
            //No hor yet ?
            if (hor == 0) {
              hor = i;
              continue;
            }
            //Already a hor
            else {
              if (max(hor, ver) - i > max_len) {
                //cout << "POSSIBLE ERROR: Too long for a segment" << endl;
              }
            }
          }
        }

        if (i == 0) {
          int start = max(ver, hor);
          int end = 0;
          finished = true;
        }

        if (finished) {
          finished = false;
          started = false;
          //DO RECOGN
          cv::Mat digit = inputImage(cv::Range::all(), cv::Range(end, start));
          if (start - end > max_len * 0.7) {
            char c = recognise_digit(digit);
            //cout << c << endl;
            text = c + text;
          }
          else if (start - end > max_len * 0.1){
            
            if (ver != 0) {
              text = "1" + text;
              //cout << "THIN: 1" << endl;
            }
            else {
              string s = recognise_thin(digit);
              text = s + text;
              //cout << "THIN: " << s << endl;
            }
          }
          
          
          hor = 0;
          ver = 0;
        }
      }
      //Not started yet
      else {
        //If NOTHING
        if (comb.at<float>(i) > (float)175) { continue; }
        //If HOR
        if (comb.at<float>(i) > (float)50) { hor = i; }
        //If VERT
        else { ver = i; }
        started = true;
      }
    }
    /*
    if (started) {
      int start = max(ver, hor);
      int end = 0;
      cv::Mat digit = inputImage(cv::Range::all(), cv::Range(end, start));
      if (start - end > max_len * 0.7) {
        char c = recognise_digit(digit);
        cout << c << endl;
        text = c + text;
      }
      else if (start - end > max_len * 0.1) {
        char c = recognise_thin(digit);
        cout << "THIN: " << c << endl;
        text = "_" + text;
      }
    }
    */
    //cout << "Display Says: " << text << endl;
    return text;
  }

  void detect_digits_old(cv::Mat& inputImage, vector<cv::Rect>& digits) {
    vector<vector<cv::Point>> contours;
    //vector<cv::Moments> moments_list;

    cv::findContours(inputImage, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    for (vector<cv::Point> contour : contours) {
      vector<cv::Point> approx;
      cv::approxPolyDP(contour, approx, 0.01 * cv::arcLength(contour, true), true);
      cv::Moments moments = cv::moments(contour);
      if (moments.m00 != 0) {
        //Here some logic has to come that decides if the area could be a digit.
        if (approx.size() >= 4) {
          //moments_list.push_back(moments);
          cv::Rect r = cv::boundingRect(contour);
          if (is_rect_digit(r)) {
            digits.push_back(r);
          }
        }
      }
    }
  }
  

  char recognise_digit(cv::Mat& inputImage) {
    //cv::imshow("Display window", inputImage);
    //cv::waitKey(0);
    //cv::destroyAllWindows();
    // Based on SSOCR: https://github.com/jiweibo/SSOCR
    int w = inputImage.cols;
    int h = inputImage.rows;

    int segment_w = (int)(max(int(w * 0.15), 1) + max(int(h * 0.15), 1)) / 2;
    int line_w = 5;

    int y[3] = { int(h * 0.25), int(h * 0.5), int(h * 0.75) };
    int cent_x = int(w * 0.5);

    /*####          1 1
     #    #       6     2
     #    #       6     2
      ####   ==>    7 7
     #    #       5     3
     #    #       5     3
      ####          4 4*/ 

    cv::Rect segments[7] = {
      cv::Rect(cent_x - line_w,   0,                 2 * line_w,    2 * segment_w),  //1
      cv::Rect(w - 2 * segment_w, y[0] - line_w ,    2 * segment_w, 2 * line_w), //2
      cv::Rect(w - 2 * segment_w, y[2] - line_w ,    2 * segment_w, 2 * line_w), //3
      cv::Rect(cent_x - line_w,   h - 2 * segment_w, 2 * line_w,    2 * segment_w),  //4
      cv::Rect(0,                 y[2] - line_w ,    2 * segment_w, 2 * line_w), //5
      cv::Rect(0,                 y[0] - line_w ,    2 * segment_w, 2 * line_w), //6
      cv::Rect(cent_x - line_w,   y[1] - segment_w,  2 * line_w,    2 * segment_w)   //7
    };

    int digit_sum = 0;
    for (int i = 0; i < 7; i++) {
      int sum = cv::countNonZero(inputImage(segments[i]));
      //cout << int(sum / (float)segments[i].area() < 0.75) << " ";
      if (sum / (float)segments[i].area() < 0.75) {
        digit_sum += segment_value[i];
      }
    }

    //cout << " Digit Sum: " << digit_sum << endl;
    
    //Draw the segment rectangles
    /*
    for (cv::Rect r : segments) {
      cv::rectangle(inputImage, r, (0,0,0), 1);
    }
    */

    return digit_lookup(digit_sum);
  }
}

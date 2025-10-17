import cv2, os
import numpy as np
import matplotlib.pyplot as plt
### THIS IS SSOCR BUT SLIGHTLY EDITED###

H_W_Ratio = 1.9
arc_tan_theta = 6.0
DIGITS_LOOKUP = {
    (1, 1, 1, 1, 1, 1, 0): 0,
    (1, 1, 0, 0, 0, 0, 0): 1,
    (1, 0, 1, 1, 0, 1, 1): 2,
    (1, 1, 1, 0, 0, 1, 1): 3,
    (1, 1, 0, 0, 1, 0, 1): 4,
    (0, 1, 1, 0, 1, 1, 1): 5,
    (0, 1, 1, 1, 1, 1, 1): 6,
    (1, 1, 0, 0, 0, 1, 0): 7,
    (1, 1, 1, 1, 1, 1, 1): 8,
    (1, 1, 1, 0, 1, 1, 1): 9,
    (0, 0, 0, 0, 0, 1, 1): '-'
}


def recognize_digits_line_method(input_img):
    roi = input_img
    h, w = roi.shape
    x0, y0 = 0,0
    x1, y1 = w,h
    suppose_W = max(1, int(h / H_W_Ratio))

    if x1 - x0 < 25 and cv2.countNonZero(roi) / ((y1 - y0) * (x1 - x0)) < 0.2:
        return "FAILED"

    if w < suppose_W / 2:
        x0 = max(x0 + w - suppose_W, 0)
        roi = input_img[y0:y1, x0:x1]
        w = roi.shape[1]

    center_y = h // 2
    quater_y_1 = h // 4
    quater_y_3 = quater_y_1 * 3
    center_x = w // 2
    line_width = 5  # line's width
    width = (max(int(w * 0.15), 1) + max(int(h * 0.15), 1)) // 2
    print(f"Shape: {w},{h} Width:{width}")
    small_delta = int(h / arc_tan_theta) // 4
    segments = [
        ((w - 2 * width, quater_y_1 - line_width), (w, quater_y_1 + line_width)),
        ((w - 2 * width, quater_y_3 - line_width), (w, quater_y_3 + line_width)),
        ((center_x - line_width - small_delta, h - 2 * width), (center_x - small_delta + line_width, h)),
        ((0, quater_y_3 - line_width), (2 * width, quater_y_3 + line_width)),
        ((0, quater_y_1 - line_width), (2 * width, quater_y_1 + line_width)),
        ((center_x - line_width, 0), (center_x + line_width, 2 * width)),
        ((center_x - line_width, center_y - line_width), (center_x + line_width, center_y + line_width)),
    ]
    on = [0] * len(segments)

    for (i, ((xa, ya), (xb, yb))) in enumerate(segments):
        seg_roi = roi[ya:yb, xa:xb]
        #plt.imshow(seg_roi, 'gray')
        #plt.show()
        total = cv2.countNonZero(seg_roi)
        area = (xb - xa) * (yb - ya) * 0.9
        # print('prob: ', total / float(area))
        if total / float(area) > 0.25:
            on[i] = 1
    # print('encode: ', on)
    if tuple(on) in DIGITS_LOOKUP.keys():
        digit = DIGITS_LOOKUP[tuple(on)]
    else:
        digit = '*'

    return digit
import cv2, os, math, numpy as np
from matplotlib import pyplot as plt

import helperfunctions as hf
import codetiming, humanfriendly
def show_img(img,title="IMAGE"):
    cv2.imshow(title, img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

############
### GOAL ### Find rectangle in picture and extract coordinates
############
BASEIMG = hf.load_image(hf.list_images()[1])

t0 = codetiming.Timer("total",lambda secs:f"Total Run Time: {humanfriendly.format_timespan(secs, True)}")
t1 = codetiming.Timer("step1",lambda secs:f"Find Display: {humanfriendly.format_timespan(secs, True)}")
t2 = codetiming.Timer("step3",lambda secs:f"Finding and Recognising Digits: {humanfriendly.format_timespan(secs, True)}")
t0.start()

#Dient zur Anpassung der Fenstergröße
def resize(Images, ImagesWidth):
	ImagesLen = len(Images)
	height = Images[0].shape[0]
	width = Images[0].shape[1] * ImagesLen
	try:
		widthDim = int(Images[0].shape[1] - (width - ImagesWidth) / ImagesLen) 	#Die nötige Länge eines Bildes wird ausgerechnet
		heightDim = int(height / ((width / ImagesWidth)))						#Die Höhe wird ausgerechnet
	except:
		widthDim = heightDim = 1
	ResizedImages = []
	for Image in Images:
		ResizedImages.append(cv2.resize(Image, (widthDim, heightDim), interpolation = cv2.INTER_AREA))
	return ResizedImages

def DetectObject(mask, InputImage):
    """
    Rewriten version from https://github.com/tobiasctrl/Opencv-Rectangle-and-circle-detection/blob/master/rechteck-kreis%20erkennung.py
    """
    
    ### TODO: edit Contouring using https://stackoverflow.com/a/14286280
    
    Contours, hierarchy = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)	#Konturen werden gesucht
    OutputImage = InputImage
    centerX = None
    centerY = None
    approx_out = None
    for cnt in Contours:
        approx = cv2.approxPolyDP(cnt,0.01*cv2.arcLength(cnt,True),True)		#Aproximation des Objekts
        ###
        ### https://docs.opencv.org/3.4/dd/d49/tutorial_py_contour_features.html
        ###
        
        M = cv2.moments(cnt)	#In späterer Folge zur ermittlung des Mittelpunkts erforderlich
        ###
        ### Momenten: https://en.wikipedia.org/wiki/Image_moment 
        ### https://docs.opencv.org/2.4/modules/imgproc/doc/structural_analysis_and_shape_descriptors.html 
        ###
        
        if M["m00"] != 0:
            if len(approx) == 4:

                OutputImage = cv2.drawContours(InputImage, [cnt], -1, (0,255,0), 5)
                
                #Mittelpunkte werden ermittelt
                centerY = int(M["m01"] / M["m00"]) 
                centerX = int(M["m10"] / M["m00"])

                #Einzeichnen des Mittlelpunkts
                cv2.circle(OutputImage, (centerX, centerY),5,(0,0,255), 2)	

                #Try and locate corners
                cv2.circle(OutputImage, approx[0][0],5,(0,0,255), -1)	
                cv2.circle(OutputImage, approx[1][0],5,(0,0,255), -1)	
                cv2.circle(OutputImage, approx[2][0],5,(0,0,255), -1)	
                cv2.circle(OutputImage, approx[3][0],5,(0,0,255), -1)	
                #Wenn ein Rechteck erkannt wird
                
                cv2.putText(OutputImage, "Rectangle",(centerX, centerY),cv2.FONT_HERSHEY_SIMPLEX, 1, 255)
                approx_out = approx
            else:
                OutputImage = cv2.drawContours(InputImage, [cnt], -1, (255,255,0), 5)
                
                
    return OutputImage, centerX, centerY, approx_out

def canny(img):
    return cv2.Canny(img,50,100)

def sobel(img):
    return cv2.Sobel(img, cv2.CV_64F,1,1, 5)

def do_range(img):
    return cv2.inRange(img, (0,0,0), (100,100,100))

def do_detection(image,mask_func, detect_func):
    blur = cv2.GaussianBlur(image,(13,13),0) #Verringerung von Bild rauschen
    mask = mask_func(blur)

    RGB = cv2.cvtColor(blur, cv2.COLOR_BGR2RGB)
    # 1) Find rectangle (as done in rectangledetection.py)

    WindowWidth = 1100					#Fenster Breite
    lower_Color_val = (0, 0, 0) 		#Untere RGB Farbgrenze
    higher_Color_val = (100, 100, 100)	#Obere RGB Farbgrenze

    #mask = cv2.inRange(RGB, lower_Color_val, higher_Color_val) #Grenzwerte
    OutputImage, centerX, centerY, approx = detect_func(mask, RGB)

    #print("Mittelpunkt:\n""X: "+ str(centerX) + " pixel" + "\nY: " + str(centerY) + " pixel") #Der Abstand vom Kreis Objekt Mittelpunkt relativ zur oberen linken Ecke wird ausgegeben

    thresholdblur = cv2.bitwise_and(blur, blur, mask=mask)	#Das Bild mit den eingestellten Grenzwerten wird erstellt

    blur, thresholdblur, OutputImage = resize((blur, thresholdblur, OutputImage), WindowWidth)

    CombinedImages = np.concatenate((blur, thresholdblur, OutputImage), axis=1) #Bilder werden kombiniert
        
    return approx,CombinedImages

t1.start()
crop_area, before = do_detection(BASEIMG, canny, DetectObject)
t1.stop()

# 2) CROP

def find_surr_rect(points):
    minX, minY = points[0][0][0],points[0][0][1]
    maxX, maxY = 0,0
    for point in points:
        if point[0][0] < minX: minX = point[0][0]
        if point[0][1] < minY: minY = point[0][1]
        if point[0][0] > maxX: maxX = point[0][0]
        if point[0][1] > maxY: maxY = point[0][1]
        
    return (int(minX),int(minY)),(int(maxX),int(maxY))
        
def get_crop(img, start, end):
    
    return img[start[1]:end[1],start[0]:end[0]]
start,end = find_surr_rect(crop_area)
cropped = get_crop(BASEIMG,start,end)
#cropped = hf.load_image("images\cropped\keukenweegschaal.jpg")
#cv2.imshow("Cropped", cropped)
#cv2.waitKey(0)
#cv2.destroyAllWindows()


# 3) FIND DIGITS
# OUTPUT : Zou in princiepe 2 hoofdkleuren moeten bevatten
## OPTIES ##
## Zoeken op rechthoeken (segmenten van cijfers)
## Zoek naar omringende rechthoek (zou vr alles behalve 1 moeten werken)

# Mogelijkheid om segmentherkenning op contouren te doen ?
# Timers toevoegen in code
def apply_brightness_contrast(input_img, brightness=0, contrast=0):
    if brightness != 0:
        if brightness > 0:
            shadow = brightness
            highlight = 255
        else:
            shadow = 0
            highlight = 255 + brightness
        alpha_b = (highlight - shadow) / 255
        gamma_b = shadow

        buf = cv2.addWeighted(input_img, alpha_b, input_img, 0, gamma_b)
    else:
        buf = input_img.copy()

    if contrast != 0:
        f = 131 * (contrast + 127) / (127 * (131 - contrast))
        alpha_c = f
        gamma_c = 127 * (1 - f)

        buf = cv2.addWeighted(buf, alpha_c, buf, 0, gamma_c)

    return buf

def proc_image(inp: np.ndarray) -> np.ndarray:
    clip = 4.1
    grid = (5, 3)
    #kernel = np.ones((8, 8), np.uint8)
    clahe = cv2.createCLAHE(clipLimit=clip, tileGridSize=grid)

    # inp = cv.fastNlMeansDenoising(inp, None, 5, 7, 21)
    # inp = cv2.morphologyEx(inp, cv.MORPH_OPEN, kernel)
    np = clahe.apply(inp)
    # morph = cv2.morphologyEx(morph, cv.MORPH_CLOSE, kernel)
    return apply_brightness_contrast(apply_brightness_contrast(clahe.apply(inp), 90, 80), -100, 100)

def determine_segment(img: np.ndarray) -> int:
    SegmentMask = (
        (1, 1, 1, 1, 1, 1, 0),  # 0
        (0, 1, 1, 0, 0, 0, 0),  # 1
        (1, 1, 0, 1, 1, 0, 1),  # 2
        (1, 1, 1, 1, 0, 0, 1),  # 3
        (0, 1, 1, 0, 0, 1, 1),  # 4
        (1, 0, 1, 1, 0, 1, 1),  # 5
        (1, 0, 1, 1, 1, 1, 1),  # 6
        (1, 1, 1, 0, 0, 1, 0),  # 7
        (1, 1, 1, 1, 1, 1, 1),  # 8
        (1, 1, 1, 1, 0, 1, 1)   # 9
    )

    NumSegments = 7
    ThresholdPct = 0.5
    Invert = True
    UseFuzzy = True
    TestWindowSize: int = 3
    SegmentTestPoints = (
        ((29, 20),  (49, 20)),       # 0
        ((68, 30),  (68, 56)),       # 1
        ((68, 93),  (68, 113)),      # 2
        ((29, 130), (49, 130)),      # 3
        ((13, 93),  (13, 113)),      # 4
        ((13, 30),  (13, 56)),       # 5
        ((29, 74),  (49, 74)),       # 6
    )
    #img = cv2.cvtColor(img,cv2.COLOR_RGB2GRAY)
    if len(img.shape) > 2 and img.shape[2] != 1:
        raise TypeError("Input image is not grayscale.")

    half_window = int(TestWindowSize / 2)
    max_val = np.iinfo(img.dtype).max
    is_seg_active = list()

    for seg in range(NumSegments):
        test_pts = SegmentTestPoints[seg]
        num_pts = len(test_pts)
        pt_vals = list()

        for pt_idx in range(num_pts):
            num_px = 0
            px_val: int = 0
            pt = test_pts[pt_idx]
            for x in range(-half_window, half_window):
                for y in range(-half_window, half_window):
                    px_val += int(img.item(pt[1] + y, pt[0] + x))
                    num_px += 1
            pt_vals.append(int(px_val / num_px))

        seg_mean = sum(pt_vals) / num_pts
        pct = float(seg_mean / max_val)
        active = 0

        if not Invert:
            if pct <= ThresholdPct:
                active = 1
        else:
            if pct >= ThresholdPct:
                active = 1

        is_seg_active.append(active)

    if UseFuzzy:
        digit_confidences = list()

        # Get confidence for each possible segment
        for seg_num, mask in enumerate(SegmentMask):
            if len(is_seg_active) != len(mask):
                raise ValueError("seg_active size != mask size")

            match_conf = 0.0

            for x, seg_val in enumerate(mask):
                if seg_val == is_seg_active[x]:
                    match_conf += 1.0

            match_conf /= NumSegments
            digit_confidences.append(match_conf)

        return digit_confidences.index(max(digit_confidences))
    else:
        for seg_num, mask in enumerate(SegmentMask):
            if len(is_seg_active) != len(mask):
                raise ValueError("seg_active size != mask size")

            matched = True

            for x, seg_val in enumerate(mask):
                if seg_val != is_seg_active[x]:
                    matched = False
                    break

            if matched:
                return seg_num
    return -1

from recogniseDigits import recognize_digits_line_method
DIGITS = []
def DetectObject2(mask, InputImage):
    """
    Rewriten version from https://github.com/tobiasctrl/Opencv-Rectangle-and-circle-detection/blob/master/rechteck-kreis%20erkennung.py
    """
    Contours, hierarchy = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)	#Konturen werden gesucht
    ORIG = np.copy(InputImage)
    OutputImage = InputImage
    centerX = None
    centerY = None
    approx_out = None
    count = 0
    global DIGITS
    for cnt in Contours:
        
        approx = cv2.approxPolyDP(cnt,0.01*cv2.arcLength(cnt,True),True)
        M = cv2.moments(cnt)

        if M["m00"] != 0:
            if len(approx) >= 4:
                min,max = find_surr_rect(approx)
                width = max[0] - min[0]
                height = max[1] - min[1]
                
                if not (width * 1.5 > height or width * 2.5 < height):
                    print(f"{width}, {height}")
                    count += 1
                    OutputImage = cv2.drawContours(InputImage, [cnt], -1, (255,255,0), 2)
                    
                    #Mittelpunkte werden ermittelt
                    centerY = int(M["m01"] / M["m00"]) 
                    centerX = int(M["m10"] / M["m00"])

                    #Einzeichnen des Mittlelpunkts
                    cv2.circle(OutputImage, (centerX, centerY),2,(0,0,255), -1)	

                    #Wenn ein Rechteck erkannt wird
                    
                    
                    approx_out = approx
                
                    cv2.rectangle(OutputImage,min,max,(100,100,20),3)
                    bigger = cv2.resize(ORIG[min[1] - 3:max[1]+ 5,min[0] - 3 :max[0] + 3], (240,600), interpolation= cv2.INTER_LINEAR)
                    t = codetiming.Timer(text=lambda secs: f"Recognise Nr. {count}: {humanfriendly.format_timespan(secs, True)}")
                    t.start()
                    a = cv2.cvtColor(bigger,cv2.COLOR_RGB2GRAY)
                    a = proc_image(a)
                    digit = recognize_digits_line_method(a)
                    t.stop()
                    cv2.putText(OutputImage, f"{digit}",(centerX, centerY),cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0,0,255),2)
                    print(f"Nr. {count}: {digit}")
                    DIGITS.append(a)
            #else:
                #OutputImage = cv2.drawContours(InputImage, [cnt], -1, (255,255,0), 5)
                
                
    return OutputImage, centerX, centerY, approx_out

t2.start()
_, after = do_detection(cropped, lambda img: cv2.inRange(img,50,100), DetectObject2)
t2.stop()
t0.stop()

hf.show_img(before, "BEFORE")
hf.show_img(after, "AFTER")
for d in DIGITS:
    hf.show_img(d, "DIGIT")
# TODO : Alle (?) gevonden rectangles meegeven aan segodoc ?

# Another attempt at linefitting (DID NOT WORK)
#cdstP = np.copy(cropped)
#linesP = cv2.HoughLinesP(do_range(cropped), 1, np.pi / 180, 100, None, 30, 30)
    
#if linesP is not None:
#    for i in range(0, len(linesP)):
#        l = linesP[i][0]
#        cv2.line(cdstP, (l[0], l[1]), (l[2], l[3]), (0,100,200), 1, cv2.LINE_AA)




#c_start,c_size = get_crop_coord(approx)
#segodoc_simple(file=os.path.join(os.path.dirname(__file__),"images\microwave.jpg"),numchars=2,
#               cropStart=c_start,cropSize=c_size, thresholdPct=0.5, invert=False, charStart=(0,0),
#               charSize=(int(c_size[0]/3),c_size[0]),charSpacing=(1,1))

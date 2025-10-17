from segodec import proc_image
from ssocr import preprocess
from helperfunctions import *
import codetiming, humanfriendly
import os

imgs = list_images("images\\cropped") #+ list_images()

times = {}
counter = 0
KEY = ""

def run_process(proc_function, im, title):
    timer = codetiming.Timer("timer",lambda secs:f"{title}: \t {humanfriendly.format_timespan(secs, True)}") 
    timer.start()
    proc_img = proc_function(im)
    timer.stop()
    return (timer.last,proc_img)

def save(img: cv2.Mat, folder: str, img_name: str):
    if not os.path.exists("out\\Code\\" + folder):
        os.makedirs("out\\Code\\" + folder)
    cv2.imwrite("out\\Code\\" + folder + "\\" + img_name,img)

for img in imgs:
    img_name = os.path.basename(img)
    print("Processing ",img_name)
    counter = counter + 1
    image = load_image(img)
    gray = cv2.cvtColor(image,cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray,(7,7),0)
    
    #KEY = "Sego"
    #time, proc_img = run_process(proc_image,gray,KEY)
    #times[KEY] = times.get(KEY,0.0) + time
    
    #time, proc_img = run_process(lambda img: preprocess(img,10,morph=False),blurred,"SSOCR (no morph)")
    #times["SSOCR (no morph)"] = times.get("SSOCR (no morph)",0.0) + time
    
    #time, proc_img = run_process(lambda img: preprocess(img,10),blurred,"SSOCR (morph)")
    #times["SSOCR (morph)"] = times.get("SSOCR (morph)",0.0) + time
    
    KEY = "canny"
    time, proc_img = run_process(lambda img: cv2.Canny(img,50,100),blurred,KEY)
    times[KEY] = times.get(KEY,0.0) + time
    save(proc_img,KEY,img_name)
    
    KEY = "sobel"
    time, proc_img = run_process(lambda img: cv2.Sobel(img, cv2.CV_64F,1,0, 5),blurred,KEY)
    times[KEY] = times.get(KEY,0.0) + time
    save(proc_img,KEY,img_name)
    
    KEY = "sobel_h"
    time, proc_img = run_process(lambda img: cv2.Sobel(img, cv2.CV_64F,0,1, 5),blurred,KEY)
    times[KEY] = times.get(KEY,0.0) + time
    save(proc_img,KEY,img_name)
    
    print("")
    
print("Processed ",counter, " images.\nAverage process time:")    
for name, time in times.items():
    print(name,"\t",time/counter)
    
    
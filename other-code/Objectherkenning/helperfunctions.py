import cv2, os, math, numpy as np
from matplotlib import pyplot as plt

def list_images(relativePathDir="images"):
    """
    Lists all files in a directory.
    """
    dir =  os.path.join(os.path.dirname(__file__), relativePathDir)
    return [os.path.join(relativePathDir,f) for f in os.listdir(dir) if os.path.isfile(os.path.join(dir, f))]

def load_image(img: str):
    """
    Loads an image to an OpenCV object.
    """
    return cv2.imread(os.path.join(os.path.dirname(__file__), img))

def load_gray_image(img: str):
    """
    Loads an image to an OpenCV object.
    """
    return cv2.imread(os.path.join(os.path.dirname(__file__), img),cv2.IMREAD_GRAYSCALE)

def show_img(img,title="IMAGE"):
    cv2.imshow(title, resize(img))
    cv2.waitKey(0)
    cv2.destroyAllWindows()
    
def resize(img: cv2.Mat):
    print(img.shape)
    if img.shape[0] > img.shape[1]:
        return cv2.resize(img,(800,int(img.shape[0]*800/img.shape[1])))
    else:
        return cv2.resize(img,(int(img.shape[1]*800/img.shape[0]),800))

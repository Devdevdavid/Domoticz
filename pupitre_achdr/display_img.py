#!/usr/bin/python

# use a Tkinter label as a panel/frame with a background image
# note that Tkinter only reads gif and ppm images
# use the Python Image Library (PIL) for other image formats
# free from [url]http://www.pythonware.com/products/pil/index.htm[/url]
# give Tkinter a namespace to avoid conflicts with PIL
# (they both have a class named Image)
import sys
if sys.version_info[0] == 2:  # the tkinter library changed it's name from Python 2 to 3.
    import Tkinter as tk
else:
    import tkinter as tk

# https://pypi.org/project/Pillow/2.2.1/
from PIL import Image, ImageTk
from tkinter.ttk import Frame, Button, Style
import time

def main():
    root = tk.Tk()
    root.title('Home')

    # https://stackoverflow.com/questions/7966119/display-fullscreen-mode-on-tkinter
    #root.attributes("-fullscreen", True)

    # pick an image file you have .bmp  .jpg  .gif.  .png
    # load the file and covert it to a Tkinter image object
    imageObj = Image.open("fullscreen_img.jpg")

    winHeight = root.winfo_height()
    winWidth = root.winfo_width()
    imgHeight = imageObj.height
    imgWidth = imageObj.width

    print(winHeight, winWidth, imgHeight, imgWidth)

    if (imgHeight > imgWidth):
        newImgHeight = winHeight
        newImgWidth = int(newImgHeight * imgWidth / imgHeight)
    else:
        newImgWidth = winWidth
        newImgHeight = int(newImgWidth * imgHeight / imgWidth)
    
    # https://stackoverflow.com/questions/24061099/tkinter-resize-background-image-to-window-size
    imageObj = imageObj.resize((newImgWidth, newImgHeight))
    image1 = ImageTk.PhotoImage(imageObj)

    # root has no image argument, so use a label as a panel
    panel1 = tk.Label(root, image=image1)
    panel1.configure(background='black')
    panel1.pack(side=tk.TOP, fill=tk.BOTH, expand=tk.YES)

    root.mainloop()

if __name__ == '__main__':
    main()
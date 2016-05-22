import numpy as np
import cv2
import spout

spout1 = spout.Spoutnumpy()
spout2 = spout.Spoutnumpy()
spout3 = spout.Spoutnumpy()

img = cv2.imread('messi5.jpg',-1)
img2 = cv2.imread('RGBA-Colour-Palatte.png',cv2.IMREAD_UNCHANGED)
img3 = np.ones((20,20,3), dtype=np.uint8)

cv2.imshow('image',img)
cv2.imshow('image2',img2)
cv2.imshow('image3',img3)

spout1.send('image', img)
spout2.send('image2', img2)
spout3.send('image3', img3)
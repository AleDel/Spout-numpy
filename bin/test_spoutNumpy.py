import numpy as np
import cv2
import spout

spout1 = spout.Spoutnumpy()
spout2 = spout.Spoutnumpy()
spout3 = spout.Spoutnumpy()
spout4 = spout.Spoutnumpy()

img = cv2.imread('messi5.jpg',-1)
img2 = cv2.imread('RGBA-Colour-Palatte.png',cv2.IMREAD_UNCHANGED)
img3 = np.ones((20,20,3),dtype=np.uint8)
img3.fill(120)
cap = cv2.VideoCapture(0)

while(True):
    
    # Capture frame-by-frame
    ret, frame = cap.read()

    # Our operations on the frame come here
    #gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    cv2.imshow('frame',frame)

    spout1.send('image', img)
    spout2.send('image2', img2)
    spout3.send('image3', img3)
    spout4.send('image4', frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break 

# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()

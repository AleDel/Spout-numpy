# Spout-numpy
Send [numpy](http://www.numpy.org/) bytes image

```python
import numpy as np
import spout

spout1 = spout.Spoutnumpy()

img = np.ones((20,20,3), dtype=np.uint8) #20 x 20 pixels 3 channels image
img.fill(120)

spout1.send('image', img)
```
## Info
* used [pybind11](https://github.com/pybind/pybind11). to bind [spout](http://spout.zeal.co/) c++
* convert 3 or 4 channels image buffer to dx11 texture
* the numpy array need be 3 dimensions

## Installation
* Change includes: python, numpy and pybind11 (Property Manager windows in visualstudio)
* Compile, copy 'spout.pyd' to accessible folder



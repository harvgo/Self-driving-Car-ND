# Semantic Segmentation Project (Advanced Deep Learning)
## Motivation
The project has as objective the construction of a Fully Convolutional Neural network (FCN), based on the VGG-16 image classifier network. The idea is to identify the road where the vehicle is capable of driving, using images from a mounted camera on the car's dash. The images used belongs to the KITTI road data set.

## Description
### Architecture
The neural network can be find in the ```main.py``` script. Initialy, the pre-trained VGG-16 is downloaded by the script and extract the inout image, the keep probability, and layers 3, 4 and 7 (lines 36-42). With them, it is possible to create the rest of the network as follows (function layers, lines 48-103):

* A 1x1 convolutional layer from VGG's layer 7
* An upsampling layer with kernel 4 and stride 2 from the previous layer
* A 1x1 convolutional layer from VGG's layer 4
* A first skip layer based on the last two layers
* An upsampling layer with kernel 4 and stride 2 from the first skip layer
* A 1x1 convolutional layer from VGG's layer 3
* A second skip layer based on the last two layers
* An unsampling layer with kernel 16 and stride 8 from the second skip layer

The network has cross-entropy as los function, using an Adam optimizer method.

### Training
The hyperparameters used for the network training were:

* keep probability: 0.5
* learning rate: 1e-5
* Epochs: 48
* batch size: 5

## Result
The images below show some samples, about the outcome of the FCN. The safe area for driving is shown as a green layer upon the original image.

![sample1](/samples/um_000019.png)
![sample2](/samples/umm_000024.png)
![sample3](/samples/umm_000041.png)
![sample4](/samples/uu_000049.png)
![sample5](/samples/uu_000074.png)

In general terms, the outcome performance is acceptable, athough is not 100% accurate. Other regions apart from the road are mistakenly identified, like:
![sample6](/samples/umm_000060.png)

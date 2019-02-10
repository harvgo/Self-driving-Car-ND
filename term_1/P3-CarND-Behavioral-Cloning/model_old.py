# Importing Libraries
from keras.models import Sequential
from keras.layers import Dense, Dropout, Activation, Flatten, Convolution2D, MaxPooling2D, SpatialDropout2D
from keras.layers.normalization import BatchNormalization
from keras.optimizers import Adam
from keras.callbacks import ModelCheckpoint, EarlyStopping

import numpy as np
import pandas as pd
import os
import json

from scipy import ndimage
from scipy.misc import imresize

# ***Import Udacity driving data***

# 'driving_log.csv' contains driving data.

print('Loading Udacity driving angles...')

udata = pd.read_csv('driving_log.csv',header=None)
udata.columns = ('Center','Left','Right','Angle','Throttle','Brake','Speed')
udata = np.array(udata['Angle'])
udata = udata[1:]
labels = np.array(udata,dtype=np.float32)

print('Udacity driving angles successfully loaded!')

# 'IMG' folder contains Udacity driving images
# Constructing arrays for Udacity driving images located in 'IMG' directory

images = np.asarray(os.listdir("IMG/"))
center = np.ndarray(shape=(len(angles), 40, 80, 3))
right = np.ndarray(shape=(len(angles), 40, 80, 3))
left = np.ndarray(shape=(len(angles), 40, 80, 3))

# Populating Udacity Driving Dataset
# Images have been cropped 60 pixels from the top and 20 pixels at the bottom.
# Images are then resized to 40x80x3 to increase training speeds.

print('Loading Udacity driving images...')

count = 0
for image in images:
    image_file = os.path.join('IMG', image)
    if image.startswith('center'):
        image_data = ndimage.imread(image_file).astype(np.float32)
        image_data = image_data[60:140,:,:]
        center[count % len(angles)] = imresize(image_data, (40, 80, 3))
    elif image.startswith('right'):
        image_data = ndimage.imread(image_file).astype(np.float32)
        image_data = image_data[60:140,:,:]
        right[count % len(angles)] = imresize(image_data, (40, 80, 3))
    elif image.startswith('left'):
        image_data = ndimage.imread(image_file).astype(np.float32)
        image_data = image_data[60:140,:,:]
        left[count % len(angles)] = imresize(image_data, (40, 80, 3))
    count += 1


print('Udacity driving images successfully loaded!')

print('Concatenating center, right and left camera images to training database...')
X_train = np.concatenate((X_train, center, right, left), axis=0)
y_train = np.concatenate((y_train, labels, (labels - 0.27), (labels + 0.27)),axis=0)
print('Length of X_train is: ', len(X_train))
print('Length of y_train is: ',len(y_train))

# ***Import recovery data***

# 'r_driving_log.csv' contains recovery data.

print('Loading recovery data angles...')

rdata = pd.read_csv('r_driving_log.csv', header = None)
rdata.columns = ('Center','Left','Right','Angle','Throttle','Brake','Speed')
rdata = np.array(rdata['Angle'])
rdata = rdata[1:]

labels = np.array(rdata,dtype=np.float32) 

print('Recovery data successfully loaded!')

# 'RIMG' folder contains recovery images
# Constructing arrays for recovery driving images located in 'RIMG' directory

images = np.asarray(os.listdir("RIMG/"))
center = np.ndarray(shape=(len(rdata), 40, 80, 3))
right = np.ndarray(shape=(len(rdata), 40, 80, 3))
left = np.ndarray(shape=(len(rdata), 40, 80, 3))

# Populating recovery driving datasets
# Images have been cropped 60 pixels from the top and 20 pixels at the bottom
# Images are then resized to 40x80x3 to increase training speeds

print('Loading recovery data images...')

count = 0
for image in images:
    image_file = os.path.join('RIMG', image)
    if image.startswith('center'):
        image_data = ndimage.imread(image_file).astype(np.float32)
        image_data = image_data[60:140,:,:]
        center[count % len(angles)] = imresize(image_data, (40, 80, 3))
    elif image.startswith('right'):
        image_data = ndimage.imread(image_file).astype(np.float32)
        image_data = image_data[60:140,:,:]
        right[count % len(angles)] = imresize(image_data, (40, 80, 3))
    elif image.startswith('left'):
        image_data = ndimage.imread(image_file).astype(np.float32)
        image_data = image_data[60:140,:,:]
        left[count % len(angles)] = imresize(image_data, (40, 80, 3))
    count += 1

print('Recovery data images successfully loaded!')

print('Concatenating center, right and left camera images to training database...')

X_train = np.concatenate((center, right, left), axis=0)
y_train = np.concatenate( (labels, (labels - 0.27), (labels + 0.27)),axis=0)

print('New length of X_train is: ', len(X_train))
print('New length of y_train is: ',len(y_train))


# Mirror all images and inverting their angles.
print('Mirror images & inverting angles...')
mirror = np.ndarray(shape=(X_train.shape))
count = 0
for i in range(len(X_train)):
    mirror[count] = np.fliplr(X_train[i])
    count += 1
mirror.shape

# Create mirror image labels
mirror_angles = y_train * -1

print('Reflected data successfully created!')

# Concatenating new mirrored data
X_train = np.concatenate((X_train, mirror), axis=0)
y_train = np.concatenate((y_train, mirror_angles),axis=0)
print('New length of X_train is: ', len(X_train))
print('New length of y_train is: ',len(y_train))


# *** Training Model ***

# This model has been adapted from Nvidia End to End Learning Paper
# It has 5 Convolutional Layers and 4 Fully Connected Layers
# The input has been changed to 40 x 80 x 3 from 66 x 200 x 3

# Generator for fit_generator

def generator():
    while 1: # Loop forever, generator never ends
        i=0
        for i in range(1372):
            yield X_train[i*64:(i+1)*64], y_train[i*64:(i+1)*64]

print('Compiling & Training Model...')
model = Sequential()

# Normalize
model.add(Lambda(lambda x: (x/255.0) - 0.5))
# 3 5x5 Convolution layers
model.add(Convolution2D(24, 5, 5, border_mode="same", subsample=(2,2), activation="elu"))
model.add(SpatialDropout2D(0.2))
model.add(Convolution2D(36, 5, 5, border_mode="same", subsample=(2,2), activation="elu"))
model.add(SpatialDropout2D(0.2))
model.add(Convolution2D(48, 5, 5, border_mode="same", subsample=(2,2), activation="elu"))
model.add(SpatialDropout2D(0.2))

model.add(Convolution2D(64, 3, 3, border_mode="valid", activation="elu"))
model.add(SpatialDropout2D(0.2))
model.add(Convolution2D(64, 3, 3, border_mode="valid", activation="elu"))
model.add(SpatialDropout2D(0.2))

model.add(Flatten())
model.add(Dropout(0.5))
model.add(Dense(100, activation="elu"))
model.add(Dense(50, activation="elu"))
model.add(Dense(10, activation="elu"))
model.add(Dropout(0.5))
model.add(Dense(1))
model.summary()

# Compiling model with Adam optimizer and learning rate of .0001
adam = Adam(lr=0.001)
model.compile(loss='mse', optimizer=adam, metrics=['accuracy'])

# Model saves the weights whenever validation loss improves
checkpoint = ModelCheckpoint(filepath = 'model.h5', verbose = 1, save_best_only=True, monitor='val_loss')

# Discontinue training when validation loss fails to decrease
callback = EarlyStopping(monitor='val_loss', patience=2, verbose=1)

# Training model for 10 epochs and a batch size of 128
#model.fit(X_train,y_train,nb_epoch=10, verbose=1,batch_size=128,shuffle=True,validation_split=0.01,callbacks=[checkpoint,callback])
model.fit_generator(generator(), samples_per_epoch = 87834, nb_epoch = 10, verbose=2,
                     validation_data=None, class_weight=None, nb_worker=1)
print('Model Trained Successfully!')

json_model = model.to_json()
with open('model.json', 'w') as json_file:
    json_file.write(json_model)
print("Model saved!")

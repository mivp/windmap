"""
Input: topleft and bottomright points in (lat,lon) + output size
Output: interpolated wind map
"""

import numpy as np
import pygrib
from PIL import Image
from math import sqrt
import matplotlib.pyplot as plt

tl = {}
tl['lat'] = -36.9998611
tl['lon'] = 140.9998611

br = {}
br['lat'] = -39.0001389
br['lon'] = 144.0001389

xz_scale = 0.3

def latLon2xz(lon, lat):
    oneDegreeLength = 30.86666667 * 3647
    x = (lon - tl['lon']) * oneDegreeLength * xz_scale
    z = (tl['lat'] - lat) * oneDegreeLength * xz_scale
    return (x, z)


# windmap
print('top left pos', latLon2xz(tl['lon'], tl['lat']))
print('bottom right pos', latLon2xz(br['lon'], br['lat']))

# get control points inside the area defined by tl and br from grib files
u_grbs = pygrib.open('utmp.grib')
u_grb = u_grbs.select()[0]
lat,lon = u_grb.latlons()
u_data=u_grb.values
print (u_data.shape)

v_grbs = pygrib.open('vtmp.grib')
v_grb = v_grbs.select()[0]
#lat,lon = v_grb.latlons()
v_data=v_grb.values
print (v_data.shape)

nrows = u_data.shape[0]
ncols = u_data.shape[1]
print('nrows', nrows, 'ncols', ncols)

u_low = np.zeros( (9, 13), dtype = np.float32)
v_low = np.zeros( (9, 13), dtype = np.float32)
for r in range(nrows):
    for c in range(ncols):
        if lat[r,c] < tl['lat'] and lat[r,c] > br['lat'] and lon[r,c] > tl['lon'] and lon[r,c] < br['lon']:
            latind = int((-37 - lat[r,c])/0.25)
            lonind = int((lon[r,c]-141)/0.25)
            u_low[latind, lonind] = u_data[r, c]
            v_low[latind, lonind] = v_data[r, c]

print(u_low)
print(v_low)


# save to image
print('u_low: ', np.amin(u_low), np.amax(u_low))
print('v_low: ', np.amin(v_low), np.amax(v_low))
nrows = u_low.shape[0]
ncols = u_low.shape[1]
img = np.zeros( (nrows, ncols, 4), dtype=np.uint8 )
u_diff_val = np.amax(u_low) - np.amin(u_low)
img[:, :, 0] = 255 * ( (u_low-np.amin(u_low)) / u_diff_val )
v_diff_val = np.amax(v_low) - np.amin(v_low)
img[:, :, 1] = 255 * ( (v_low-np.amin(v_low)) / v_diff_val )
img[:, :, 2] = 0
img[:, :, 3] = 255
im = Image.fromarray(img)
im = im.resize( (512, 512), Image.ANTIALIAS )
im.save('../winddata/windmap_southwest.png')

print('Done!')
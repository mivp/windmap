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

#xz_scale = 0.3
xz_scale = 1

def latLon2xz(lon, lat):
    oneDegreeLength = 30.86666667 * 3647
    x = (lon - tl['lon']) * oneDegreeLength * xz_scale
    z = (tl['lat'] - lat) * oneDegreeLength * xz_scale
    return (x, z)


def calGridValue(control_points, x, z,  power = 2, smoothing = 0):

    nominator = [0, 0]
    denominator = [0, 0]
    value = [0, 0]

    for i in range(len(control_points)):
        cp = control_points[i]
        px = cp['x']
        pz = cp['z']
        v0 = cp['wind'][0]
        v1 = cp['wind'][1]

        dist = sqrt( (x-px)*(x-px) + (z-pz)*(z-pz) + smoothing*smoothing )

        if dist < 0.0000000001:
            value = [v0, v1]
            break

        nominator[0] = nominator[0] + (v0 / pow(dist, power))
        nominator[1] = nominator[1] + (v1 / pow(dist, power))

        denominator[0] = denominator[0] + (1 / pow(dist, power))
        denominator[1] = denominator[1] + (1 / pow(dist, power))

        for j in range(2):
            if denominator[j] > 0:
                value[j] = nominator[j] / denominator[j]
            else:
                value[j] = -100000

    return value


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

control_points = []
for r in range(nrows):
    for c in range(ncols):
        if lat[r,c] < tl['lat'] and lat[r,c] > br['lat'] and lon[r,c] > tl['lon'] and lon[r,c] < br['lon']:
            cp = {}
            cp['lat'] = lat[r,c]
            cp['lon'] = lon[r,c]
            xz = latLon2xz(lon[r,c], lat[r,c])
            cp['x'] = xz[0]
            cp['z'] = xz[1]
            cp['wind'] = (2*u_data[r,c], 2*v_data[r,c])
            control_points.append(cp)
print(len(control_points))
print(control_points)

# generate 2d array to store windmap
grid_size = [256, 256] # [rows cols]
br_xz = latLon2xz(br['lon'], br['lat'])
dr = br_xz[1] / grid_size[0]
dc = br_xz[0] / grid_size[1]
grid_delta = [dr, dc]
print('grid_delta', grid_delta)

u_data = np.zeros( (grid_size[0], grid_size[1]), dtype = np.float32 )
v_data = np.zeros( (grid_size[0], grid_size[1]), dtype = np.float32 )
for r in range(grid_size[0]):
    print('row', r)
    for c in range(grid_size[1]):
        x = c * dc
        z = r * dr
        value = calGridValue(control_points, x, z)
        #print (value)
        u_data[r, c] = value[0]
        v_data[r, c] = value[1]

#U, V = np.meshgrid(np.arange(0, grid_size[0], 1), np.arange(0, grid_size[1], 1))
#plt.figure()
#plt.quiver(U, V, u_data, v_data, units='width')
#plt.show()

# save to image
nrows = u_data.shape[0]
ncols = u_data.shape[1]
img = np.zeros( (nrows, ncols, 4), dtype=np.uint8 )
u_diff_val = np.amax(u_data) - np.amin(u_data)
img[:, :, 0] = 255 * ( (u_data-np.amin(u_data)) / u_diff_val )
v_diff_val = np.amax(v_data) - np.amin(v_data)
img[:, :, 1] = 255 * ( (v_data-np.amin(v_data)) / v_diff_val )
img[:, :, 2] = 0
img[:, :, 3] = 255
print('udata: ', np.amin(u_data), np.amax(u_data))
print('vdata: ', np.amin(v_data), np.amax(v_data))
im = Image.fromarray(img)
im.save('../winddata/test_windmap.png')

print('Done!')
#python 3
import numpy as np
import pygrib
from PIL import Image

u_grbs = pygrib.open('utmp.grib')
u_grb = u_grbs.select()[0]
lat,lon = u_grb.latlons()
u_data=u_grb.values
print (u_data.shape)

v_grbs = pygrib.open('vtmp.grib')
v_grb = v_grbs.select()[0]
#lat,lon = grb.latlons()
v_data=v_grb.values
print (v_data.shape)

#min_val = min(np.amin(u_data), np.amin(v_data))
#max_val = max(np.amax(u_data), np.amax(v_data))

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
im.save('../data/windmap.png')

print('Done!')
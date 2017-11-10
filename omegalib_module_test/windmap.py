import windmap
from math import *
from euclid import *
from omega import *
from cyclops import *
from omegaToolkit import *

wm = windmap.initialize()
wm.init('winddata/windmap.ini')

# model
scene = getSceneManager()

# camera
cam = getDefaultCamera()
cam.getController().setSpeed(10000)
cam.setPosition(Vector3(0, 0, 0))
setNearFarZ(2, 400000)

#menu
mm = MenuManager.createAndInitialize()
menu = mm.getMainMenu()
mm.setMainMenu(menu)
campos = [39286.29, 64904.98, 125220.43]
camori = [-0.94, 0.33, 0.09, 0.04]
cmd = 'cam.setPosition(Vector3(' + str(campos[0]) + ',' + str(campos[1]) + ',' + str(campos[2]) + ')),' + \
		'cam.setOrientation(Quaternion(' + str(camori[0]) + ',' + str(camori[1]) + ',' + str(camori[2]) + ',' + str(camori[3]) + '))'
menu.addButton("Go to camera 1", cmd)

queueCommand(":freefly")

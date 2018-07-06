from time import sleep
from picamera import PiCamera
import subprocess
import datetime

refresh = 4
camera = PiCamera()
camera.resolution = (1280,720)
camera.start_preview()

try:
	sleep(refresh)
	camera.capture('initial.bmp')
	while True:
		sleep(refresh)
		camera.capture('update.bmp')
		print('Picture Taken')
except KeyboardInterrupt:
	pass

camera.stop_preview()
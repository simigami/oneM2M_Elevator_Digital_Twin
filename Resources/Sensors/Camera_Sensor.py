import os
import datetime

default_picture_folder_windows = ''
default_picture_folder_linux = '/home/user/Desktop/Camera_Sensor'

def make_folder():
	if not os.path.exists(default_picture_folder_linux):
		os.makedirs(default_picture_folder_linux)

def take_one_picture(picture_output):
	vid_command = "libcamera-jpeg"
	vid_width = "  --width 1080"
	vid_height = " --height 1920"
	vid_time = f" --nopreview -t1"
	vid_output = fr" -o {picture_output}"

	command = vid_command + vid_width + vid_height + vid_time + vid_output
	#print(command)
	os.system(command)

def get_data():
	make_folder()
	timestamp = datetime.datetime.now()
	timestamp_str = timestamp.strftime("%Y_%m%d_%H%M%S")

	picture_output = rf"{default_picture_folder_linux}/{timestamp_str}.jpg"

	take_one_picture(picture_output)

	return picture_output

if __name__ == '__main__':
	get_data()


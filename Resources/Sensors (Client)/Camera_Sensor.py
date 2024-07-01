import os
import datetime

default_picture_folder_windows = ''
default_picture_folder_linux = '/home/user/Desktop/Camera_Sensor'

second = 1
minute = 60 * second
hour = 60 * minute

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

def take_picture_for_N(location, time):
	for i in range(time):
		take_one_picture(location)

def test_video(location):
	vid_command = "libcamera-vid"
	vid_width = " --width 1080"
	vid_height = " --height 1920"
	vid_time = f" -t 60"
	vid_output = fr" -o {location}"

	command = vid_command + vid_width + vid_height + vid_time + vid_output
	os.system(command)

def get_data():
	make_folder()
	timestamp = datetime.datetime.now()
	timestamp_str = timestamp.strftime("%Y_%m%d_%H%M%S")

	picture_output = rf"{default_picture_folder_linux}/{timestamp_str}.jpg"
	video_output = rf"{default_picture_folder_linux}/{timestamp_str}.mp4"

	take_one_picture(picture_output)

	return picture_output

if __name__ == '__main__':
	get_data()


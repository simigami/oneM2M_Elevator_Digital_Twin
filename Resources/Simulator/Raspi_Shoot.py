import os
import time
import datetime
from threading import Thread
from config import TEST_PATH

second = 1
minute = 60 * second
hour = 60 * minute
day = 24 * hour

def make_folder(folder_path):
    if not os.path.exists(folder_path):
        os.makedirs(folder_path)


def take_one_picture(folder_path, timestamp_str):
    vid_command = "libcamera-jpeg"
    vid_width = "  --width 1080"
    vid_height = " --height 1920"
    vid_time = f" --nopreview -t1"

    vid_output = fr" -o {folder_path}/{TEST_PATH.This_Elevator_Number_str}_{timestamp_str}.jpg"

    command = vid_command + vid_width + vid_height + vid_time + vid_output
    os.system(command)

def take_n_picture(folder_path, N):
    make_folder(folder_path)
    now = TEST_PATH.Default_Timestamp
    
    for i in range(N):
        now_str = now.strftime("%Y_%m%d_%H%M%S")
        take_one_picture(folder_path, now_str)
        now = now + datetime.timedelta(seconds=1)

if __name__ == '__main__':
    N = 5
    take_n_picture(N)


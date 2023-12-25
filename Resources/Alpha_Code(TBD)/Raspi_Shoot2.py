import os
import datetime
from threading import Thread
from config import TEST_PATH

second = 1000
minute = 60 * second
hour = 60 * minute
day = 24 * hour


def create_log(message):
    log_file_name = "Video_Record_Log.txt"
    now = datetime.datetime.now().strftime("%Y_%m%d_%H%M%S")


    if TEST_PATH.os_name == "Linux":
        os.chdir(TEST_PATH.Videos_Folder_Location_Linux)
    else:
        os.chdir(TEST_PATH.Videos_Folder_Location_windows)
    
    log_message = f"[Time :{now}] : {message}\n"

    with open(log_file_name, "a") as log_file:
        log_file.write(log_message)

def run_Windows(timestamp_str, shoot_time):
    vid_command = "libcamera-vid"
    vid_width = " --width 1080"
    vid_height = " --height 1920"
    vid_time = f" --framerate 30 -t {shoot_time} -o "
    TEST_PATH.Default_Timestamp = timestamp_str

    #vid_output = fr"{TEST_PATH.Videos_Folder_Location_windows}\{timestamp_str}\{TEST_PATH.This_Elevator_Number_str}_{timestamp_str}.h264"
    vid_output = fr"{TEST_PATH.Videos_Folder_Location_windows}\{TEST_PATH.This_Elevator_Number_str}_{timestamp_str}.h264"
    command = vid_command + vid_width + vid_height + vid_time + vid_output

    message = "Video Start"
    create_log(message)

    os.system(command)

    message = "Video End"
    create_log(message)

    return 1
    
def run_Linux(timestamp, shoot_time):
    
    vid_command = "libcamera-vid"
    vid_width = " --width 1080"
    vid_height = " --height 1920"
    vid_time = f" --framerate 30 -t {shoot_time} -o "
    timestamp_str = timestamp.strftime("%Y_%m%d_%H%M%S")
    TEST_PATH.Default_Timestamp = timestamp_str

    if TEST_PATH.os_name == "Linux":
        vid_output = fr"{TEST_PATH.Videos_Folder_Location_Linux}/{TEST_PATH.This_Elevator_Number_str}_{timestamp_str}.h264"
    else:
        vid_output = fr"{TEST_PATH.Videos_Folder_Location_windows}\{TEST_PATH.This_Elevator_Number_str}_{timestamp_str}.h264"
        
    command = vid_command + vid_width + vid_height + vid_time + vid_output
    
    message = "Video Start"
    create_log(message)

    os.system(command)

    message = "Video End"
    create_log(message)

    return 1

def make_folder(timestamp_str):
    output_folder = fr"{TEST_PATH.Pictures_Folder_Location_Linux}/{timestamp_str}"

    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    return output_folder

def take_one_picture(folder_path, timestamp_str):
    vid_command = "libcamera-jpeg"
    vid_width = "  --width 1080"
    vid_height = " --height 1920"
    vid_time = f" --nopreview -t1"

    vid_output = fr" -o {folder_path}/{TEST_PATH.This_Elevator_Number_str}_{timestamp_str}.jpg"

    command = vid_command + vid_width + vid_height + vid_time + vid_output
    os.system(command)

def take_n_picture(N):
    timestamp = datetime.datetime.now()
    timestamp_str = timestamp.strftime("%Y_%m%d_%H%M%S")
    folder_path = make_folder(timestamp_str)

    now = timestamp
    for i in range(N):
        now_str = now.strftime("%Y_%m%d_%H%M%S")
        take_one_picture(folder_path, now_str)
        now = now + datetime.timedelta(seconds=1)

if __name__ == '__main__':
    pass


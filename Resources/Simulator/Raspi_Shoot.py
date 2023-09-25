import os
import datetime
from threading import Thread
from config import TEST_PATH

second = 1000
minute = 60 * second
hour = 60 * minute
day = 24 * hour

def run_commad(command):
    print(command)
    os.system(command)

def create_log(message):
    log_file_name = "Video_Record_Log.txt"
    now = datetime.datetime.now().strftime("%Y%m%d%H%M%S")

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

    thr = Thread(target=run_commad, args=(command,))
    thr.start()
    thr.join()

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

    #vid_output = fr"{TEST_PATH.Videos_Folder_Location_windows}\{timestamp_str}\{TEST_PATH.This_Elevator_Number_str}_{timestamp_str}.h264"
    vid_output = fr"{TEST_PATH.Videos_Folder_Location_windows}/{TEST_PATH.This_Elevator_Number_str}_{timestamp_str}.h264"
    command = vid_command + vid_width + vid_height + vid_time + vid_output

    message = "Video Start"
    create_log(message)

    thr = Thread(target=run_commad, args=(command,))
    thr.start()
    thr.join()

    message = "Video End"
    create_log(message)

    return 1

def take_one_picture(picture_path):
    command = f"libcamera-jpeg --width 1080 --height 1920 -o {picture_path}"
    os.system(command)
    
    return picture_path

if __name__ == '__main__':
    pass


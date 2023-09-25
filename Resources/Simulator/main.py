import datetime
import os
import glob
import multiprocessing
import Make_Dirs, Raspi_Shoot, Get_Sensors,Video_To_Image, Elevator_Inside
from config import TEST_PATH

shoot_time = 10000
Linux = 0
Windows = 1

def video_exist():
    path = TEST_PATH.Videos_Remainder_Folder_Location_Windows

    if os.path.exists(path):
        target = os.path.join(path, '**/*.h264')
        video_list = glob.glob(target, recursive=True)

        if video_list is None:
            print("Error in Video_To_Image, video list is None")
            return None
        else:
            return video_list

    return None


def extract_info_from_filename(filename):
    #print(filename)
    parts = filename.split('_')

    identifier = parts[0]
    timestamp = parts[1] + parts[2] + parts[3].split('.')[0]
    # print(identifier)
    # print(timestamp)

    TEST_PATH.This_Elevator_Number_str = identifier
    TEST_PATH.Default_Timestamp = timestamp

    timestamp = datetime.datetime.strptime(timestamp, '%Y%m%d%H%M%S')
    return identifier, timestamp

def run():
    Make_Dirs.make_dirs_for_program()

    if Linux:
        timestamp = datetime.datetime.now().replace(microsecond=0)
        pool = multiprocessing.Pool(processes=2)

        pool.apply_async(Raspi_Shoot.run_Linux, args=(timestamp, shoot_time))
        pool.apply_async(Get_Sensors.write_average_alt_per_second, args=(timestamp, shoot_time))

        pool.close()
        pool.join()

        video_path = TEST_PATH.Videos_Folder_Location_windows + rf"/{TEST_PATH.This_Elevator_Number_str}_{TEST_PATH.Default_Timestamp}.h264"
        output_folder = TEST_PATH.Pictures_Folder_Location_windows + rf"/{TEST_PATH.Default_Timestamp}"
        Video_To_Image.extract_frames_logic(video_path, output_folder, second=-1)


    elif Windows:
        pre_videos = video_exist()
        if len(pre_videos) >= 1:
            os.chdir(TEST_PATH.Videos_Remainder_Folder_Location_Windows)
            for video in pre_videos:
                video = os.path.basename(video)

                identifier, timestamp = extract_info_from_filename(video)
                Make_Dirs.Reset_Path_Based_On_No(identifier)

                timestamp_str = timestamp.strftime("%Y_%m%d_%H%M%S")

                video_path = TEST_PATH.Videos_Remainder_Folder_Location_Windows + rf"/{identifier}/{identifier}_{timestamp_str}.h264"
                picture_folder = TEST_PATH.Pictures_Folder_Location_windows + rf"/{timestamp_str}"

                Video_To_Image.extract_frames_logic(video_path, picture_folder, second=-1)
                Elevator_Inside.detect_color(picture_folder, 0)

        else:
            pass
            # TEST_PATH.Default_Timestamp = fr"20230912142324"
            # video_path = TEST_PATH.Videos_Folder_Location_windows + rf"/No5_20230912142324.h264"
            # picture_folder = TEST_PATH.Pictures_Folder_Location_windows + rf"/2023_0912_142324"
            #
            # Video_To_Image.extract_frames_logic(video_path, picture_folder, second=500)
            #
            # Elevator_Inside.detect_color(picture_folder, 0)

if __name__ == '__main__':
    run()
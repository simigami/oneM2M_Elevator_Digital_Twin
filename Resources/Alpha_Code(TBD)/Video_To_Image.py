import os
import datetime
import cv2
from config import TEST_PATH

def extract_frames_logic(video_path, output_folder, second=-1):
    timestamp = datetime.datetime.strptime(TEST_PATH.Default_Timestamp, "%Y_%m%d_%H%M%S")
    timestamp = timestamp - datetime.timedelta(seconds=1) # timestamp correction with sensors
    
    cap = cv2.VideoCapture(video_path)
    frame_count = 0

    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    if second == -1:
        ret, frame = cap.read()
        while ret:
            if frame_count % 30 == 0:
                timestamp = timestamp + datetime.timedelta(seconds=1)
                timestamp_str = timestamp.strftime("%Y_%m%d_%H%M%S")

                # Save the frame as an image
                frame_filename = os.path.join(output_folder, f'{timestamp_str}.jpg')

                print(f"Frames extracted and saved to {output_folder} as {timestamp_str}.jpg")
                cv2.imwrite(frame_filename, frame)

            frame_count += 1
            ret, frame = cap.read()
        cap.release()
    else:
        count = 0
        while count != second:
            # Read a frame from the video
            ret, frame = cap.read()

            if not ret:
                break
            timestamp = timestamp + datetime.timedelta(seconds=frame_count/30.0)
            count = count + 1

            if frame_count % 30 == 0:
                timestamp_str = timestamp.strftime("%Y_%m%d_%H%M%S")

                # Save the frame as an image
                frame_filename = os.path.join(output_folder, f'{timestamp_str}.jpg')

                print(f"Frames extracted and saved to {output_folder} as {timestamp_str}.jpg")
                cv2.imwrite(frame_filename, frame)

            frame_count += 1
        cap.release()

if __name__ == '__main__':
    # vid = '/home/user/Videos/libcamera_vid/Result/No1_20230726235205.h264'
    # vid_name = 'No1_20230726235205.h264'
    # output = f'/home/user/Videos/Pictures/{vid_name}'
    # time = datetime.datetime.now()
    # extract_frames(vid, output, time)
    pass

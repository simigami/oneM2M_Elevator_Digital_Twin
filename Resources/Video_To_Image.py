import os
import time
import datetime
import cv2

def extract_frames(video_path, output_folder, start_timestamp):
    # Create the output folder if it doesn't exist
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    # Open the video file
    cap = cv2.VideoCapture(video_path)
    frame_count = 0

    total_frame = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    frame_rate = int(cap.get(cv2.CAP_PROP_FPS))
    total_length_seconds = total_frame / frame_rate

    log_start = time.time()
    while True:
        # Read a frame from the video
        ret, frame = cap.read()

        if not ret:
            break
        timestamp = start_timestamp + datetime.timedelta(seconds=frame_count / 30.0)

        if frame_count % 30 == 0:
            timestamp_str = timestamp.strftime("%Y%m%d_%H%M%S")

            # Save the frame as an image
            frame_filename = os.path.join(output_folder, f'frame_{timestamp_str}.jpg')

            print(f"Frames extracted and saved to {output_folder} as frame_{timestamp_str}.jpg")
            cv2.imwrite(frame_filename, frame)

        frame_count += 1

    cap.release()

    log_end = time.time()
    print("Time Spent is {} seconds".format(log_end-log_start))


if __name__ == '__main__':
    vid = '/home/user/Videos/libcamera_vid/Result/No1_20230726235205.h264'
    vid_name = 'No1_20230726235205.h264'
    output = f'/home/user/Videos/Pictures/{vid_name}'
    time = datetime.datetime.now()
    extract_frames(vid, output, time)
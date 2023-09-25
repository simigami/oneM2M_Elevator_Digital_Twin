import os
import time
import datetime
import tailer
import cv2
import numpy as np

import Write_Log

try:
    from config import TEST_PATH, TEST_COLOR, TEST_VARIABLES
    from collections import deque

except Exception as e:
    pass

# This py is in charge of detect green dots in given image folder path and write final button list in txt

def read_label_and_draw_rectangle(img, coordinates_of_label, show=0):
    if TEST_PATH.os_name == "Linux":
        label_path = TEST_PATH.label_linux
    else:
        label_path = TEST_PATH.label_windows
        
    height, width, _ = img.shape

    with open(label_path, 'r') as file:
        annotations = file.readlines()

        for annotation in annotations:
            class_id, x_ratio, y_ratio, width_ratio, height_ratio = map(float, annotation.strip().split(' '))

            left_x = int((x_ratio - width_ratio / 2) * width)
            top_y = int((y_ratio - height_ratio / 2) * height)
            right_x = int((x_ratio + width_ratio / 2) * width)
            bottom_y = int((y_ratio + height_ratio / 2) * height)

            coordinates_of_label.append([int(class_id), left_x, right_x, top_y, bottom_y])
            cv2.rectangle(img, (left_x, top_y), (right_x, bottom_y), (0, 255, 0), 2)

        if(show):
            cv2.namedWindow('sample', cv2.WINDOW_NORMAL)  # WINDOW_NORMAL allows for window resizing
            cv2.resizeWindow('sample', width // 2, height // 2)
            cv2.imshow('sample', img)
            cv2.waitKey(0)

def detect_contours(img, lowest_color_to_detect, highest_color_to_detect, show=0):
    detected_img = cv2.inRange(img, lowest_color_to_detect, highest_color_to_detect)
    contours, hierarchy = cv2.findContours(detected_img, cv2.RETR_TREE, cv2.CHAIN_APPROX_NONE)
    #print(contours)
    flattened_contour = []

    for contour in contours:
        area = cv2.contourArea(contour)
        if area > TEST_VARIABLES.min_circle_area:
            flattened_contour.extend(contour.reshape(-1, 2))

    flattened_contour = np.array(flattened_contour)

    if(show):
        height, width, _ = img.shape
        cv2.namedWindow('sample', cv2.WINDOW_NORMAL)  # WINDOW_NORMAL allows for window resizing
        cv2.resizeWindow('sample', width // 2, height // 2)
        cv2.imshow('sample', detected_img)
        cv2.waitKey(0)

    return flattened_contour, hierarchy

def check_contour_dots_with_id(flattened_contour, coordinates_of_label):
    id_in_dots = [0 for i in range(coordinates_of_label[-1][0])]
    id_in_dots.append(len(flattened_contour)) # len(flatten_contour) == total dot count
    _, panel_left_x, panel_right_x, panel_top_y, panel_bottom_y = coordinates_of_label[-1]

    #print(flattened_contour)
    #print(coordinates_of_label)
    for dot in flattened_contour:
        x, y = dot
        if (panel_left_x <= x <= panel_right_x) and (panel_top_y <= y <= panel_bottom_y):
            for coordinates_of_each_id in coordinates_of_label:
                class_id, left_x, right_x, top_y, bottom_y = coordinates_of_each_id

                if (left_x <= x <= right_x) and (top_y <= y <= bottom_y):
                    id_in_dots[class_id-1] += 1
                    break

    return id_in_dots

def return_pressed_buttons(id_in_dots):
    print(id_in_dots)
    
    total = id_in_dots[-1]
    result = []
    if total >= TEST_VARIABLES.min_total_dot_when_green:
        for i in range(len(id_in_dots)-1):
            if id_in_dots[i] >= total * TEST_VARIABLES.min_confidence_rate:
                result.append(i)

    return result

def write_to_txt(final_id_list, log_folder, debug=0):
    os.chdir(log_folder)
    txt_name = rf"Button_List_{TEST_PATH.Default_Timestamp}.txt"
    #txt_name = rf"Button_List_20230908125554.txt"

    print("Writing Button List on image {}...".format(final_id_list[0]))
    with open(txt_name, 'a') as file:
        for elem in final_id_list:
            if elem == final_id_list[-1]:
                file.write(f"{elem}\n")
            else:
                file.write(f"{elem} ")

    if(debug):
        with open(txt_name, 'r') as file:
            last_line = []
            for _ in range(2):
                last_line.append(next(file).strip())
            previous = last_line[0]
            previous_arr = list(map(str, previous.split(' ')))
            previous_arr[0] = previous_arr[0][:-4]  # remove .jpg string

            current = last_line[1]
            current_arr = list(map(str, current.split(' ')))

            if previous_arr[1:] != current_arr[1:]:
                Write_Log.gather_logs_and_make_final(previous_arr, current_arr)

    else:
        last_line = tailer.tail(open(txt_name), 2)
        print(last_line)
        if len(last_line) == 2: ## If previous log already exists
            previous = last_line[0]
            previous_arr = list(map(str, previous.split(' ')))

            current = last_line[1]
            current_arr = list(map(str, current.split(' ')))

            if previous_arr[1:] != current_arr[1:]:
                Write_Log.gather_logs_and_make_final(previous_arr, current_arr)

            # print(previous_arr)
            # print(current_arr)

def detect_color(folder_path, show):
    coordinates_of_label = []

    os.chdir(folder_path)
    images = [file for file in os.listdir() if file.lower().endswith('.jpg')]
    images.sort()
    start = time.time()

    #print(images)
    for image in images:
        # if image == "2023_0908_125334.jpg":
        #     show = 1

        img = cv2.imread(image)
        read_label_and_draw_rectangle(img, coordinates_of_label, show)

        lowest_color_to_detect = np.array(TEST_COLOR.lowest_RGB, dtype="uint8")
        highest_color_to_detect = np.array(TEST_COLOR.highest_RGB, dtype="uint8")

        flattened_contour, hierarchy = detect_contours(img, lowest_color_to_detect, highest_color_to_detect, show)

        id_in_dots = check_contour_dots_with_id(flattened_contour, coordinates_of_label)

        final_id_list = return_pressed_buttons(id_in_dots)
        if len(final_id_list) != 0:
            final_id_list.insert(0, image)

            if len(flattened_contour) >= TEST_VARIABLES.min_total_dot_when_green:
                if TEST_PATH.os_name == "Linux":
                    write_to_txt(final_id_list, TEST_PATH.Logs_Folder_Location_Linux)
                    os.chdir(folder_path)
                else:
                    write_to_txt(final_id_list, TEST_PATH.Logs_Folder_Location_windows)
                    os.chdir(folder_path)
        else:
            continue

        if (show):
            height, width, _ = img.shape
            cv2.namedWindow('sample', cv2.WINDOW_NORMAL)  # WINDOW_NORMAL allows for window resizing
            cv2.resizeWindow('sample', width // 2, height // 2)
            cv2.imshow('sample', img)
            cv2.waitKey(0)

    end = time.time()
    print("Detecting Green Button on {} is Complete\nTime Spent is : {} sec".format(folder_path, round((end-start),3)))


if __name__ == '__main__':
    # folder_path = TEST_PATH.sample_image_windows
    # detect_color(folder_path, 0)
    test = [0]
    test2 = rf"E:\ML\Elevator Git\Elevator_Results\No5\Logs"
    write_to_txt(test, test2)


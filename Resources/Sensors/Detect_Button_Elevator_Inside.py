import os
import platform
import time
import cv2
import numpy as np

current_dir_path = os.getcwd()
os.chdir('../config/')
config_dir_path = os.getcwd()
os.chdir(current_dir_path)
os_version = platform.system()

detection_list = ['Open', 'Close', 'B5', 'B4', 'B3', 'B2', 'B1', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', 'Panel']
min_circle_area = 1.0
min_total_dot_when_green = 20
min_confidence_rate = 0.01
lowest_RGB = [22, 71, 57]
highest_RGB = [38, 179, 144]

# This py is in charge of detect green dots in given image folder path and write final button list in txt

def read_label_and_draw_rectangle(img, coordinates_of_label, debug=0):

    if os_version == "Windows":
        label_path = rf"{config_dir_path}\label_No1.txt"
    else:
        label_path = rf"{config_dir_path}/label_No1.txt"

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

        if(debug):
            cv2.namedWindow('sample', cv2.WINDOW_NORMAL)  # WINDOW_NORMAL allows for window resizing
            cv2.resizeWindow('sample', width // 2, height // 2)
            cv2.imshow('sample', img)
            cv2.waitKey(0)

def detect_contours(img, lowest_color_to_detect, highest_color_to_detect, debug=0):
    detected_img = cv2.inRange(img, lowest_color_to_detect, highest_color_to_detect)
    contours, hierarchy = cv2.findContours(detected_img, cv2.RETR_TREE, cv2.CHAIN_APPROX_NONE)

    flattened_contour = []

    for contour in contours:
        area = cv2.contourArea(contour)
        if area > min_circle_area:
            flattened_contour.extend(contour.reshape(-1, 2))

    flattened_contour = np.array(flattened_contour)

    if(debug):
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

    for dot in flattened_contour:
        x, y = dot
        if (panel_left_x <= x <= panel_right_x) and (panel_top_y <= y <= panel_bottom_y):
            for coordinates_of_each_id in coordinates_of_label:
                class_id, left_x, right_x, top_y, bottom_y = coordinates_of_each_id

                if (left_x <= x <= right_x) and (top_y <= y <= bottom_y):
                    id_in_dots[class_id] += 1
                    break

    return id_in_dots

def return_pressed_buttons(id_in_dots):
    #print(id_in_dots)

    total = id_in_dots[-1]
    result = []
    if total >= min_total_dot_when_green:
        for i in range(len(id_in_dots)-1):
            if id_in_dots[i] >= total * min_confidence_rate:
                result.append(i)

    return result

def get_data(image_path, debug=0):
    coordinates_of_label = []
    detected = []

    start = time.time()

    # if(debug):
    #     image_path = "E:\ML\Elevator Git\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\Images_Sample\Sample_Image_2.jpg"
    image_path = "E:\ML\Elevator Git\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\Images_Sample\Sample_Image_2.jpg"

    img = cv2.imread(image_path)
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    height, width, _ = img.shape

    read_label_and_draw_rectangle(img, coordinates_of_label, debug)

    lowest_color_to_detect = np.array(lowest_RGB, dtype="uint8")
    highest_color_to_detect = np.array(highest_RGB, dtype="uint8")

    flattened_contour, hierarchy = detect_contours(img, lowest_color_to_detect, highest_color_to_detect, debug)

    id_in_dots = check_contour_dots_with_id(flattened_contour, coordinates_of_label)

    final_id_list = return_pressed_buttons(id_in_dots)

    for id in final_id_list:
        detected.append(detection_list[id])

    return detected

if __name__ == '__main__':
    test = 1
    if(test):
        image_path = "E:\ML\Elevator Git\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\Images_Sample\Sample_Image_2.jpg"
        detected = get_data(image_path, debug=1)
        print(detected)
    else:
        pass

import cv2
import yaml

import Detect_Color
import config_default
def get_label_data(label_text_path):
    label_data = []

    with open(label_text_path, 'r') as f:
        labels = f.readlines()

    for label_info in labels:
        label_info = label_info.strip().split(' ')

        class_id = int(label_info[0])
        x = float(label_info[1])
        y = float(label_info[2])
        width = float(label_info[3])
        height = float(label_info[4])

        label_data.append((class_id, x, y, width, height))

    return label_data
    
def display_label_info(image, label_text_path):
    label_config = config_default.Config_Label()
    color_config = config_default.Config_Color()

    label_data = get_label_data(label_text_path)

    text_offset = label_config.Text['margin']
    font_scale = label_config.Text['font_scale']
    font_color_white = color_config.Color_Base['white']
    font_color_black = color_config.Color_Base['black']

    for label in label_data:
        class_id, x, y, width, height = label
        print(label)

        image_height, image_width, _ = image.shape
        left = int((x-width/2) * image_width)
        top = int((y-height/2) * image_height)
        right = int((x+width/2) * image_width)
        bottom = int((y+height/2) * image_height)

        #print("left " + str(left) + ", top " + str(top), ", right " + str(right), ", bottom ", str(bottom))
        cv2.rectangle(image, (left, top), (right, bottom), (0, 255, 0), 2)
        # label_text = f"Class ID: {class_id}"
        #
        # with open(yaml_path, 'r') as yaml_file:
        #     yaml_data = yaml.safe_load(yaml_file)
        #     class_lables = yaml_data['names']
        #
        # if class_id < len(class_lables):
        #     label_text = f"Class ID: {class_id} - {class_lables[class_id]}"
        # else:
        #     label_text = f"Class ID: {class_id}"
        #
        # if class_lables[class_id] == "Green":
        #     cv2.putText(image, label_text, (left, top - 10), cv2.FONT_HERSHEY_SIMPLEX, font_scale, font_color_black,
        #                 label_config.Text['font_size'])
        # else:
        #     cv2.putText(image, label_text, (label_config.Text['margin'], text_offset), cv2.FONT_HERSHEY_SIMPLEX, font_scale,
        #                 font_color_black, label_config.Text['font_size'])
        #     text_offset += label_config.Text['height']

    cv2.namedWindow('sample', cv2.WINDOW_NORMAL)
    cv2.imshow('sample', image)

    #image_rs = cv2.resize(image, (270,480))
    #cv2.imshow('sample', image_rs)
    key = cv2.waitKey(0)

if __name__== '__main__':
    img = cv2.imread(rf"E:\ML\Elevator Git\Elevator_Results\Pictures\No5\20230912_142324\20230912_142324.jpg")
    path = rf"E:\ML\Elevator Git\Elevator_Results\Configs\label_No5.txt"
    display_label_info(img, path)

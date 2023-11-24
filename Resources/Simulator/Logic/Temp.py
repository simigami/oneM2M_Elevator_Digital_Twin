import numpy as np
import cv2

picture_path = rf"E:\ML\Elevator Git\Elevator_Results\No5\Pictures\2023_0908_125553\2023_0908_130411.jpg"

img = cv2.imread(picture_path)

lowest_color_to_detect = np.array([0, 0, 0], dtype="uint8")
highest_color_to_detect = np.array([225, 245, 220], dtype="uint8")

detected_img = cv2.inRange(img, lowest_color_to_detect, highest_color_to_detect)

contours, hierarchy = cv2.findContours(detected_img, cv2.RETR_TREE, cv2.CHAIN_APPROX_NONE)

height, width, _ = img.shape
cv2.namedWindow('detect', cv2.WINDOW_NORMAL)  # WINDOW_NORMAL allows for window resizing
cv2.resizeWindow('detect', width // 2, height // 4)
cv2.imshow('detect', detected_img)

cv2.imwrite('test.jpg', detected_img)
cv2.waitKey(0)
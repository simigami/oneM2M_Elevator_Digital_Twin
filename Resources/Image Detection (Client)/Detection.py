import os
import cv2
from ultralytics import YOLO
os.environ["WANDB_MODE"] = "dryrun"

def detect_images():
    new_model = YOLO(r'best.pt')
    test_folder_dir = r'E:\ML\Elevator Git\Elevator_Results\No4\Pictures\2023_0912_133342'

    test_images = os.listdir(test_folder_dir)
    for test_image in test_images:
        test_image_dir = os.path.join(test_folder_dir, test_image)
        result = new_model(test_image_dir)

        plots = result[0].plot()

        # Change Images Size
        plots = cv2.resize(plots, (540, 960))

        # Show Image
        cv2.imshow('result', plots)
        cv2.waitKey(0)

def detect_one_image(img_path):
    new_model = YOLO(r'best.pt')
    result = new_model(img_path)
    class_str = ''

    for i in range(len(result[0].pred[0])):
        class_str += f'{result[0].pred[0][i][0]} '

    return class_str

if __name__ == '__main__':
    # model = YOLO('yolov8x.pt')
    # model.train(
    #     data='./train.yaml',
    #     batch=16,
    #     epochs=100
    # )
    detect_images()
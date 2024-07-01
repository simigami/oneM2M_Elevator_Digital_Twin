import os
import cv2
from ultralytics import YOLO
os.environ["WANDB_MODE"] = "dryrun"

def test():
    new_model = YOLO(r'best.pt')
    test_folder_dir = r'E:\ML\Elevator Git\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\Resources\Image Detection (Client)\Test Images'

    test_images = os.listdir(test_folder_dir)
    for test_image in test_images:
        test_image_dir = os.path.join(test_folder_dir, test_image)
        result = new_model(test_image_dir)

        plots = result[0].plot()

        # Change Images Size
        plots = cv2.resize(plots, (620, 620))
        # Save Image
        cv2.imwrite(f'{test_image_dir}_result.jpg', plots)

if __name__ == '__main__':
    # model = YOLO('yolov8x.pt')
    # model.train(
    #     data='./train.yaml',
    #     batch=16,
    #     epochs=100
    # )
    test()


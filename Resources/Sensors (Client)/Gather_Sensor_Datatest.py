import os
import tqdm
import time
import init
#import Altimeter_Sensor
import Camera_Sensor
from ultralytics import YOLO

new_model = YOLO(r'best.pt')
def get_sensor_datas():
    data = {}

    altimeter = None
    temperature = None

    #altimeter, temperature = Altimeter_Sensor.get_data()

    if altimeter is not None:
        data['altimeter'] = altimeter

    if temperature is not None:
        data['temperature'] = temperature

    picture_output_path = Camera_Sensor.get_data()

    if picture_output_path is not None:
        result = new_model(picture_output_path)
        names = new_model.names
        detected = []

        for r in result:
            for c in r.boxes.cls:
                detected.append(names[int(c)])

        data['button_inside'] = detected

    return data

def get_sensor_datas_debug(ev_name, picture_dir_path, result_write_path, mode=0):
    entries = os.scandir(picture_dir_path)
    e = [entry.path for entry in entries if entry.is_file()]
    record = []

    pbar = tqdm.tqdm(e,
        total=len(e),
        desc="Processing",
        ncols=100,
        leave=True,
    )
    i = 1

    # Clear a txt content if mode is 1
    if mode == 1:
        with open(result_write_path, 'w') as f:
            f.write('')

    try:
        for entry in pbar:
            picture_output_path = entry

            result = new_model(picture_output_path, verbose=False)
            names = new_model.names
            detected = []

            for r in result:
                for c in r.boxes.cls:
                    detected.append(names[int(c)].split(' ')[-1])

            detected = list(set(detected))
            detected.sort()
            detected.remove('Human') if 'Human' in detected else detected

            record.append([i, detected])

            if i % 3600 == 0:
                with open(result_write_path, 'a') as f:
                    for each_res in record:
                        f.write(str(each_res) + '\n')

                record = []

            i += 1
            time.sleep(0.01)

        pbar.close()

        # for i, entry in enumerate(entries):
        #     if entry.is_file():
        #         picture_output_path = entry.path
        #
        #         result = new_model(picture_output_path, verbose=False)
        #         names = new_model.names
        #         detected = []
        #
        #         for r in result:
        #             for c in r.boxes.cls:
        #                 detected.append(names[int(c)].split(' ')[-1])
        #
        #         detected = list(set(detected))
        #         detected.sort()
        #         detected.remove('Human') if 'Human' in detected else detected
        #
        #         record.append([i+1, detected])
        #
        #         if (i+1) % 3600 == 0:
        #             with open(result_write_path, 'a') as f:
        #                 for each_res in record:
        #                     f.write(str(each_res) + '\n')
        #
        #             record = []

        with open(result_write_path, 'a') as f:
            for each_res in record:
                f.write(str(each_res) + '\n')

        return True

    except Exception as e:
        print(f"Error in Debugging: {e}")
        return False

if __name__ == '__main__':
    picture_dir_path = r"E:\ML\Elevator Git\Elevator_Results\No4\Pictures\2023_0912_133342"
    get_sensor_datas_debug(picture_dir_path)
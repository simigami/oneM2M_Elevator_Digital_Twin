import os
import yaml
from config import TEST_PATH, TEST_VARIABLES
# This function is in charge of gathering each small logs from other and make a final log for Customers

def write_to_txt(log_folder, message):
    os.chdir(log_folder)
    txt_name = rf"FinalLog_{TEST_PATH.Default_Timestamp}.txt"

    print("Writing Final Log...")
    with open(txt_name, 'a') as file:
            file.write(f"{message}")

def gather_logs_and_make_final(previous_button_list, current_button_list):
    InOut = "In"

    if TEST_PATH.os_name == "Linux":
        log_from_elevator_inside = TEST_PATH.Logs_Folder_Location_Linux
        yaml_path = TEST_PATH.yaml_linux

        pbl = previous_button_list[1:]
        cbl = current_button_list[1:]

        os.chdir(TEST_PATH.Logs_Folder_Location_Linux)

        timestamp_str = TEST_PATH.Default_Timestamp
        sensor_txt_name = rf"Sensor_{timestamp_str}.txt"

        with open(yaml_path, 'r') as file:
            text = file.read()
            config = yaml.safe_load(text)

            names = config['names']


        with open(sensor_txt_name, 'r') as file:
            line = next(file).strip()
            arr = list(map(str, line.split(' ')))

            while arr[0] != current_button_list[0]:
                line = next(file).strip()
                arr = list(map(str, line.split(' ')))

        timestamp_str, alt, temp = arr
        EA = TEST_VARIABLES.Elevator_Alt

        lower_floor_cid = 0
        higher_floor_cid = 0
        for i in range(len(EA)-1):
            lower_floor_alt = EA[i]
            higher_floor_alt = EA[i+1]

            if lower_floor_alt <= alt <= higher_floor_alt:
                lower_floor_cid = i+2
                higher_floor_cid = i+3

        for i in range(len(pbl)):
            pbl[i] = names[pbl[i]]
        for i in range(len(cbl)):
            cbl[i] = names[cbl[i]]

        lower_floor_name = names[lower_floor_cid]
        higher_floor_name = names[higher_floor_cid]


        condition_message = "TEST"
        message = rf"Inout: {InOut}\nTimestamp: {current_button_list[0]}\n{condition_message}\nPrevious Button Panel: {pbl}\nCurrent Button Panel: {cbl}\nCurrent Floor: {lower_floor_name} between {higher_floor_name}"

        write_to_txt(TEST_PATH.Logs_Folder_Location_Linux, message)

    else:
        log_from_elevator_inside = TEST_PATH.Logs_Folder_Location_windows



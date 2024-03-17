import datetime
import time

import requests
import socket
import json
import ast
import os

#import Altimeter_Sensor
import Camera_Sensor
import Detect_Button_Elevator_Inside

#server_ip = "192.168.0.134"
#server_ip = "127.0.0.1"
server_ip = "192.168.0.134"
server_port = 10050
#server_port = 10053


def dev_sensor_data(timestamp, file_path):
    #SejongAI EV1 -5 12 0 10 -55 21 [] [[3, Up]]
    ret = []
    with open(file_path, 'r') as file:
        lines = file.readlines()

        for line in lines:
            data = line.strip().split()

            building_name = data[0]
            device_name = data[1]

            if device_name == "OUT":
                delta = int(data[2])

                button_outside = []
                if data[3] != '[]':
                    sub = data[3][1:-1]
                    values = sub.split(".")
                    for value in values:
                        if value == "":
                            continue
                        else:
                            subsub = value[1:-1]
                            button_values = subsub.split(",")
                            floor = int(button_values[0])
                            if str(button_values[1]) == "True":
                                direction = True
                            else:
                                direction = False

                            button_outside.append([floor, direction])

                sensor_data = {
                    "building_name": building_name,
                    "device_name": device_name,
                    "timestamp": (timestamp + datetime.timedelta(seconds=delta)).strftime("%Y_%m%d_%H%M%S"),
                    "button_outside": button_outside
                }

                ret.append(sensor_data)

            else:
                underground_floor = int(data[2])
                ground_floor = int(data[3])
                delta = int(data[4])
                velocity = float(data[5])
                altimeter = float(data[6])
                temperature = float(data[7])

                button_inside = []
                if data[8] != "[]":
                    button_inside = ast.literal_eval(data[8])

                button_outside = []
                if data[9] != "[]":
                    sub = data[9][1:-1]
                    values = sub.split(".")
                    for value in values:
                        if value == "":
                            continue
                        else:
                            subsub = value[1:-1]
                            button_values = subsub.split(",")
                            floor = int(button_values[0])
                            if str(button_values[1]) == "True":
                                direction = True
                            else:
                                direction = False

                            button_outside.append([floor, direction])


                sensor_data = {
                    "building_name": building_name,
                    "device_name": device_name,
                    "underground_floor": underground_floor,
                    "ground_floor": ground_floor,
                    "timestamp": (timestamp + datetime.timedelta(seconds=delta)).strftime("%Y_%m%d_%H%M%S"),
                    "velocity": velocity,
                    "altimeter": altimeter,
                    "temperature": temperature,
                    "button_inside": button_inside,
                    "button_outside": button_outside
                }

                ret.append(sensor_data)

    return ret

def dev_send_to_server(file_path, server_ip, server_port):
    address = (server_ip, server_port)
    device_name = "EV1"
    header = {
        rf"{device_name}": []
    }

    start = datetime.datetime.now()
    ret = dev_sensor_data(start, file_path)

    for i in range(len(ret)):
        send_time = datetime.datetime.strptime(ret[i]['timestamp'], "%Y_%m%d_%H%M%S")
        now = datetime.datetime.now()

        while(now < send_time):
            time.sleep(1)
            now = datetime.datetime.now()

        if(now > send_time):
            print(ret[i])

            json_file_path = os.path.join(os.getcwd(), 'result.json')

            if not os.path.exists(json_file_path):
                with open('result.json', 'w') as json_file:
                    json.dump(header, json_file, indent=2)

            with open('result.json', 'r+') as json_file:
                file_content = json.load(json_file)

                # If device_name is different
                if device_name not in file_content:
                    file_content[device_name] = []

                file_content[device_name].append(ret[i])
                json_file.seek(0)
                json.dump(file_content, json_file, indent=2)

            try:
                with open(json_file_path, 'r') as json_file:
                    json_data = json.load(json_file)

                last_dict = json_data.get(device_name, [])[-1]

                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                    s.connect(address)
                    s.sendall(json.dumps(last_dict).encode() + b'\n')

                    print("JSON data sent successfully.")

            except (socket.error, json.JSONDecodeError, socket.timeout) as e:
                print(f"Error sending JSON data: {e}")

    return

def get_sensor_datas():
    building_name = "SejongAI"
    device_name = "EV1"
    altimeter = None
    temperature = None
    velocity = -10
    button_detected_elevator_inside = ['4','5']

    underground_floor = 5
    ground_floor = 12

    # [0] = Outside Floor, [1] Moving Direction -> True = Up, False = Down
    button_detected_elevator_outside = [[1, False], [3, True]] #[[8, True]]

    header = {
        rf"{device_name}": []
    }

    #altimeter, temperature = Altimeter_Sensor.get_data()
    
    picture_output_path = ""
    #picture_output_path = Camera_Sensor.get_data()

    if picture_output_path is None:
        return None
    else:
        #button_detected_elevator_inside = Detect_Button_Elevator_Inside.get_data(picture_output_path, debug=1)

        sensor_data = {
            "building_name": building_name,
            "device_name": device_name,
            "underground_floor": underground_floor,
            "ground_floor": ground_floor,
            "timestamp": datetime.datetime.now().strftime("%Y_%m%d_%H%M%S"),
            "velocity": velocity,
            "altimeter": -51,
            "temperature": 22,
            "button_inside": button_detected_elevator_inside,
            "button_outside": button_detected_elevator_outside
        }

        json_file_path = os.path.join(os.getcwd(),'result.json')

        if not os.path.exists(json_file_path):
            with open('result.json', 'w') as json_file:
                json.dump(header, json_file, indent=2)

        with open('result.json', 'r+') as json_file:
            file_content = json.load(json_file)

            # If device_name is different
            if device_name not in file_content:
                file_content[device_name] = []

            file_content[device_name].append(sensor_data)
            json_file.seek(0)
            json.dump(file_content, json_file, indent=2)

        return json_file_path, device_name

def send_json_to_server(server_ip, server_port, json_file_path, device_name):
    address = (server_ip, server_port)

    try:
        with open(json_file_path, 'r') as json_file:
            json_data = json.load(json_file)
           
        last_dict = json_data.get(device_name, [])[-1]

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(address)
            s.sendall(json.dumps(last_dict).encode() + b'\n')

            print("JSON data sent successfully.")
            return

    except (socket.error, json.JSONDecodeError, socket.timeout) as e:
        print(f"Error sending JSON data: {e}")


if __name__ == '__main__':
    dev_file_path = r"E:\ML\Elevator Git\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\Resources\Sensors\testlog2.txt"
    # json_file_path, device_name = get_sensor_datas()
    # if json_file_path is not None:
    #     send_json_to_server(server_ip, server_port, json_file_path, device_name)

    dev_send_to_server(dev_file_path, server_ip, server_port)
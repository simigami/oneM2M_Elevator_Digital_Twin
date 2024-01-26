import datetime
import requests
import socket
import json
import os

#import Altimeter_Sensor
import Camera_Sensor
import Detect_Button_Elevator_Inside

#server_ip = "192.168.0.134"
server_ip = "127.0.0.1"
server_port = 10050

def get_sensor_datas():
    building_name = "SejongAI"
    device_name = "EV1"
    altimeter = None
    temperature = None
    velocity = -10
    button_detected_elevator_inside = ['B4']

    underground_floor = 5
    ground_floor = 12

    # [0] = Outside Floor, [1] Moving Direction -> True = Up, False = Down
    button_detected_elevator_outside = [] #[[8, True]]

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
    json_file_path, device_name = get_sensor_datas()
    if json_file_path is not None:
        send_json_to_server(server_ip, server_port, json_file_path, device_name)
import datetime
import socket
import json
import os

# import Altimeter_Sensor
import Camera_Sensor
import Detect_Button_Elevator_Inside

server_ip = "127.0.0.1"
server_port = 10050

def get_sensor_datas():
    device_name = "EV1"
    header = {
        rf"{device_name}": []
    }

    # altimeter, temperature = Altimeter_Sensor.get_data()
    picture_output_path = Camera_Sensor.get_data()

    if picture_output_path is None:
        return None
    else:
        button_detected_elevator_inside = Detect_Button_Elevator_Inside.get_data(picture_output_path, debug=0)

        sensor_data = {
            "timestamp": datetime.datetime.now().strftime("%Y_%m%d_%H%M%S"),
            # "altimeter": altimeter,
            # "temperature": temperature,
            "button_inside": button_detected_elevator_inside
        }

        json_file_path = os.path.join(os.getcwd(),'result.json')

        if not os.path.exists(json_file_path):
            with open('result.json', 'w') as json_file:
                json.dump(header, json_file, indent=2)

        with open('result.json', 'r+') as json_file:
            file_content = json.load(json_file)
            file_content[device_name].append(sensor_data)
            json_file.seek(0)
            json.dump(file_content, json_file, indent=2)

        return json_file_path

def send_json_to_server(json_file_path, server_ip, server_port):
    address = (server_ip, server_port)

    try:
        with open(json_file_path, 'r') as json_file:
            json_data = json.load(json_file)

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(address)
            s.sendall(json.dumps(json_data).encode() + b'\n')

            print("JSON data sent successfully.")
            return

    except (socket.error, json.JSONDecodeError) as e:
        print(f"Error sending JSON data: {e}")


if __name__ == '__main__':
    json_file_path = get_sensor_datas()
    if json_file_path is not None:
        send_json_to_server(json_file_path, server_ip, server_port)
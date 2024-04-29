# This file is responsilble for sending data to DT Server
# First This Will gather initial data from init.py
# Then, it will gather RTS information by using Gather_Sensor_Data.py
# It will concat init and sensor data, then send all information to DT Server
import datetime
import time
import socket
import json
import init
#import Gather_Sensor_Data
import ast

server_ip = "192.168.0.134"
server_port = 10050

init_data = init.set_init_data()

def test_mode():
    logs = []
    address = (server_ip, server_port)

    with open('testlog.txt', 'r') as file:
        # Read the file line by line
        for line in file:
            logs.append(line.strip().split(' '))

            if logs[0][0] == "OUT":
                outside_logs = logs[-1][-1]
                to_list = ast.literal_eval(outside_logs)

                logs[-1][-1] = to_list

            else:
                inside_logs = logs[-1][-1]
                to_list = ast.literal_eval(inside_logs)

                logs[-1][-1] = to_list

    count = 0

    for elem in logs:
        start = datetime.datetime.now()

        sensor_data = {
            "building_name": "",
            "device_name": "",
            "underground_floor": None,
            "ground_floor": None,
            "each_floor_altimeter": None,
            "timestamp": None,
            "velocity": None,
            "altimeter": None,
            "temperature": None,
            "button_inside": None,
            "button_outside": None,
            "E_Idle": None,
            "E_Standby": None,
            "E_Ref": None
        }

        if elem[0] == init_data.outname:
            sleep_time = int(elem[1]) - count
            sensor_data['building_name'] = init_data.building_name
            sensor_data['timestamp'] = datetime.datetime.now().isoformat()
            sensor_data['device_name'] = init_data.outname
            sensor_data['button_outside'] = elem[-1]

        else:
            sleep_time = int(elem[1]) - count
            thisEV = None

            for each_ev in init_data.evlist:
                if each_ev.ev_name == elem[0]:
                    thisEV = each_ev
                    break

            if thisEV is not None:
                sensor_data['building_name'] = init_data.building_name
                sensor_data['device_name'] = elem[0]
                sensor_data['underground_floor'] = thisEV.underground_floor
                sensor_data['ground_floor'] = thisEV.ground_floor
                sensor_data['each_floor_altimeter'] = thisEV.altimeter_list
                sensor_data['timestamp'] = datetime.datetime.now().isoformat()
                sensor_data['velocity'] = elem[2]
                sensor_data['altimeter'] = elem[3]
                sensor_data['temperature'] = elem[4]
                sensor_data['button_inside'] = elem[-1]

                if thisEV.energy_flag is True:
                    sensor_data['E_Idle'] = thisEV.E_idle
                    sensor_data['E_Standby'] = thisEV.E_standby
                    sensor_data['E_Ref'] = thisEV.E_ref

        end = datetime.datetime.now()
        diff = (end - start).total_seconds()

        if sleep_time - diff > 0:
            time.sleep(sleep_time - diff)

        count = int(elem[1])

        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect(address)
                s.sendall(json.dumps(sensor_data).encode() + b'\n')

                print(f"{sensor_data} sent successfully.")
                s.close()

        except (socket.error, json.JSONDecodeError, socket.timeout) as e:
            print(f"Error sending JSON data: {e}")

def inside_mode(data):
    address = (server_ip, server_port)

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(address)
            s.sendall(json.dumps(data).encode() + b'\n')

            print("JSON data sent successfully.")
            return

    except (socket.error, json.JSONDecodeError, socket.timeout) as e:
        print(f"Error sending JSON data: {e}")

if __name__ == '__main__':
    test_mode()
# This file is responsilble for sending data to DT Server
# First This Will gather initial data from init.py
# Then, it will gather RTS information by using Gather_Sensor_Data.py
# It will concat init and sensor data, then send all information to DT Server
import datetime
import time
import socket
import json
import init
import requests
#import Gather_Sensor_Data
import ast
import os

# Name of the directory inside the current directory
directory_name = "Simulated Data Directory"
directory_path = os.path.join(os.getcwd(), directory_name)

server_ip = "192.168.0.134"
server_port = 10050

headers = {
    "Content-Type": "application/json"
    # ?ty=0 // Init Elevator List to Digital Twin Server
    # ?ty=1 // Signal From Outside of Elevator
    # ?ty=2 // Signal From Inside of Elevator that does not holds physical values
    # ?ty=3 // Inside of Elevator that holds physical values

    # en option only works when ty=2
    # ?en=1 // Use Energy Consumption
    # ?en=0 // Do Not Use Energy Consumption
}

def get_custom_physical_datas(data, sensor_data):
    sensor_data['velocity'] = float(data[-4])
    sensor_data['altimeter'] = float(data[-3])
    sensor_data['temperature'] = float(data[-2])

    return sensor_data

def set_http_header(elem, thisEV, init_data, init_dt=0):
    headers = {
        "Content-Type": "application/json"
    }

    en_Flag = None
    ty_Flag = None

    if elem[0] == init_data.outname:
        headers["Content-Type"] = "application/json?ty=1"
        return headers

    if thisEV is not None:
        if thisEV.energy_flag is True:
            en_Flag = 1

        else:
            en_Flag = 0

        if init_dt == 1:
            ty_Flag = 0

        elif thisEV.physical_flag is True:
            ty_Flag = 3

        else:
            ty_Flag = 2

        headers["Content-Type"] = f"application/json?ty={ty_Flag}&en={en_Flag}"
        return headers

def insert_default_payloads(init_data, thisEV, sensor_data):
    sensor_data['building_name'] = init_data.building_name
    sensor_data['device_name'] = thisEV.ev_name

    sensor_data['timestamp'] = datetime.datetime.now().isoformat()

    sensor_data['underground_floor'] = thisEV.underground_floor
    sensor_data['ground_floor'] = thisEV.ground_floor
    sensor_data['each_floor_altimeter'] = thisEV.altimeter_list

    sensor_data['acceleration'] = thisEV.acceleration
    sensor_data['max_velocity'] = thisEV.max_velocity

    if thisEV.energy_flag is True:
        sensor_data['E_Idle'] = thisEV.E_idle
        sensor_data['E_Standby'] = thisEV.E_standby
        sensor_data['E_Ref'] = thisEV.E_ref

    return sensor_data


def read_file_datas(rts_path):
    ret = []
    file = open(rts_path, 'r')
    for line in file:
        ret.append(line)

    file.close()
    return ret

def run_rts(rts_path):
    file_datas = read_file_datas(rts_path)

    for line in file_datas:
        header, sensor_data = rts_logic(line)

        if sensor_data is not None:
            send(header, sensor_data)

def rts_logic(line):
    logs = []

    sensor_data = {
        "building_name": "",
        "device_name": "",
        "underground_floor": None,
        "ground_floor": None,
        "each_floor_altimeter": None,
        "timestamp": None,
        "acceleration": None,
        "velocity": None,
        "max_velocity": None,
        "altimeter": None,
        "temperature": None,
        "button_inside": None,
        "button_outside": None,
        "E_Idle": None,
        "E_Standby": None,
        "E_Ref": None
    }

    logs.append(line.strip().split(' '))

    if logs[0][0] == "OUT":
        outside_logs = logs[-1][-1]
        to_list = ast.literal_eval(outside_logs)

        logs[-1][-1] = to_list

    else:
        inside_logs = logs[-1][-1]
        to_list = ast.literal_eval(inside_logs)

        logs[-1][-1] = to_list

    elem = logs[0]
    thisEV = None

    if len(elem) == 0:
        return None

    elif elem[0] == init_data.outname:
        sensor_data['building_name'] = init_data.building_name
        sensor_data['timestamp'] = datetime.datetime.now().isoformat()
        sensor_data['device_name'] = init_data.outname
        sensor_data['button_outside'] = elem[-1]

    else:
        for each_ev in init_data.evlist:
            if each_ev.ev_name == elem[1]:
                thisEV = each_ev
                break

        sensor_data = insert_default_payloads(init_data, thisEV, sensor_data)

        if thisEV.physical_flag is True:
            sensor_data = get_custom_physical_datas(elem, sensor_data)

        sensor_data['button_inside'] = elem[-1]

    headers = set_http_header(elem, thisEV, init_data)

    return headers, sensor_data

def send(http_header, sensor_data):
    try:
        url = "http://" + server_ip + ":" + str(server_port)
        response = requests.post(url, headers=http_header, json=sensor_data)

        print(response.status_code)
        time.sleep(0.1)

    except (socket.error, json.JSONDecodeError, socket.timeout) as e:
        print(f"Error sending JSON data: {e}")

def run_test():
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

        thisEV = None

        if elem[0] == init_data.outname:
            sleep_time = int(elem[1]) - count
            sensor_data['building_name'] = init_data.building_name
            sensor_data['timestamp'] = datetime.datetime.now().isoformat()
            sensor_data['device_name'] = init_data.outname
            sensor_data['button_outside'] = elem[-1]

        else:
            sleep_time = int(elem[1]) - count

            for each_ev in init_data.evlist:
                if each_ev.ev_name == elem[0]:
                    thisEV = each_ev
                    break

            sensor_data = insert_default_payloads(init_data, thisEV, sensor_data)

            if thisEV.physical_flag is True:
                sensor_data = get_custom_physical_datas(elem, sensor_data)

        headers = set_http_header(elem, thisEV, init_data)

        end = datetime.datetime.now()
        diff = (end - start).total_seconds()

        if sleep_time - diff > 0:
            time.sleep(sleep_time - diff)

        count = int(elem[1])

        try:
            url = "http://" + server_ip + ":" + str(server_port)
            response = requests.post(url, headers=headers, json=sensor_data)

            print(response.status_code)
            print(response.json())

        except (socket.error, json.JSONDecodeError, socket.timeout) as e:
            print(f"Error sending JSON data: {e}")

def run_inside(data):
    address = (server_ip, server_port)
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(address)
            s.sendall(json.dumps(data).encode() + b'\n')

            print("JSON data sent successfully.")
            return

    except (socket.error, json.JSONDecodeError, socket.timeout) as e:
        print(f"Error sending JSON data: {e}")

def init_dt_server_by_ev_list(init_data):
    ev_list = init_data.evlist

    for thisEV in ev_list:
        sensor_data = {
            "building_name": "",
            "device_name": "",
            "underground_floor": None,
            "ground_floor": None,
            "each_floor_altimeter": None,
            "timestamp": None,
            "acceleration": None,
            "velocity": None,
            "max_velocity": None,
            "altimeter": None,
            "temperature": None,
            "button_inside": None,
            "button_outside": None,
            "E_Idle": None,
            "E_Standby": None,
            "E_Ref": None
        }

        http_header = set_http_header([""], thisEV, init_data, init_dt=1)

        sensor_data['building_name'] = init_data.building_name
        sensor_data['device_name'] = thisEV.ev_name
        sensor_data['timestamp'] = datetime.datetime.now().isoformat()
        sensor_data['underground_floor'] = thisEV.underground_floor
        sensor_data['ground_floor'] = thisEV.ground_floor
        sensor_data['each_floor_altimeter'] = thisEV.altimeter_list
        sensor_data['acceleration'] = thisEV.acceleration
        sensor_data['max_velocity'] = thisEV.max_velocity

        if thisEV.energy_flag is True:
            sensor_data['E_Idle'] = thisEV.E_idle
            sensor_data['E_Standby'] = thisEV.E_standby
            sensor_data['E_Ref'] = thisEV.E_ref

        send(http_header, sensor_data)

    return

if __name__ == '__main__':
    init_data = init.set_init_data()
    init_dt_server_by_ev_list(init_data)

    if init_data.rts_path is not None:
        run_rts(init_data.rts_path)

    else:
        run_test()
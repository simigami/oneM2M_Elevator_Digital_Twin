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
import socket

# Name of the directory inside the current directory
directory_name = "Simulated Data Directory"
directory_path = os.path.join(os.getcwd(), directory_name)

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

global server_ip
global server_port

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
    sensor_data['building name'] = init_data.building_name
    sensor_data['elevator name'] = thisEV.ev_name
    sensor_data['timestamp'] = datetime.datetime.now().isoformat()
    sensor_data['underground floor'] = thisEV.underground_floor
    sensor_data['ground floor'] = thisEV.ground_floor
    sensor_data['each floor altimeter'] = thisEV.altimeter_list
    sensor_data['acceleration'] = thisEV.acceleration
    sensor_data['max velocity'] = thisEV.max_velocity

    if thisEV.energy_flag is True:
        sensor_data['use energy calculation flag'] = True
        sensor_data['idle energy'] = thisEV.E_idle
        sensor_data['standby energy'] = thisEV.E_standby
        sensor_data['ref energy'] = thisEV.E_ref

    return sensor_data

def read_file_datas(rts_path):
    ret = []
    file = open(rts_path, 'r')
    for line in file:
        ret.append(line)

    file.close()
    return ret

def run_rts(this_building):
    file = read_file_datas(this_building.rts_path)

    for line in file:
        header, sensor_data = rts_logic(this_building, line)

        if sensor_data is not None:
            send(header, sensor_data)

def rts_logic(this_building, line):
    global server_ip
    global server_port

    logs = []

    sensor_data = {
        "building name": "",
        "elevator name": "",
        "underground floor": None,
        "ground floor": None,
        "each floor altimeter": None,
        "timestamp": None,
        "acceleration": None,
        "velocity": None,
        "max velocity": None,
        "altimeter": None,
        "temperature": None,
        "button inside": None,
        "button outside": None,
        "idle energy": None,
        "standby energy": None,
        "ref energy": None
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

    elif elem[0] == this_building.outname:
        sensor_data['building name'] = this_building.building_name
        sensor_data['timestamp'] = datetime.datetime.now().isoformat()
        sensor_data['elevator name'] = this_building.outname
        sensor_data['button outside'] = elem[-1]

    else:
        for each_ev in this_building.evlist:
            if each_ev.ev_name == elem[1]:
                thisEV = each_ev
                break

        sensor_data = insert_default_payloads(this_building, thisEV, sensor_data)

        if thisEV.physical_flag is True:
            sensor_data = get_custom_physical_datas(elem, sensor_data)

        sensor_data['button inside'] = elem[-1]

    headers = set_http_header(elem, thisEV, this_building)

    return headers, sensor_data

def send(http_header, sensor_data):
    try:
        url = "http://" + server_ip + ":" + str(server_port)
        response = requests.post(url, headers=http_header, json=sensor_data)

        print(response.status_code)
        time.sleep(0.1)

    except (socket.error, json.JSONDecodeError, socket.timeout) as e:
        print(f"Error sending JSON data: {e}")

def run_test(this_building):
    address = (server_ip, server_port)
    logs = []

    with open('testlog.txt', 'r') as file:
        # Read the file line by line
        for line in file:
            if line == '\n':
                break

            log = line.strip().split(' ')

            if log[0] == "OUT":
                outside_logs = log[-1]
                to_list = ast.literal_eval(outside_logs)

                log[-1] = to_list

            else:
                inside_logs = log[-1]
                to_list = ast.literal_eval(inside_logs)

                log[-1] = to_list

            logs.append(log)

    count = 0
    for elem in logs:
        start = datetime.datetime.now()

        sensor_data = {
            "building name": "",
            "elevator name": "",
            "underground floor": None,
            "ground floor": None,
            "each floor altimeter": None,
            "timestamp": None,
            "acceleration": None,
            "velocity": None,
            "max velocity": None,
            "altimeter": None,
            "temperature": None,
            "button inside": None,
            "button outside": None,
            "idle energy": None,
            "standby energy": None,
            "ref energy": None,
            "use energy calculation flag": False
        }

        thisEV = None

        if elem[0] == this_building.outname:
            sleep_time = int(elem[1]) - count
            sensor_data['building name'] = this_building.building_name
            sensor_data['elevator name'] = this_building.outname
            sensor_data['timestamp'] = datetime.datetime.now().isoformat()
            sensor_data['button outside'] = elem[-1]

        else:
            sleep_time = int(elem[2]) - count

            for each_ev, _ in this_building.evlist.items():
                if each_ev == elem[1]:
                    thisEV = this_building.evlist[each_ev]
                    break

            sensor_data = insert_default_payloads(this_building, thisEV, sensor_data)
            sensor_data = get_custom_physical_datas(elem, sensor_data) if thisEV.physical_flag is True else sensor_data

            sensor_data['button inside'] = elem[-1]

        headers = set_http_header(elem, thisEV, this_building)

        end = datetime.datetime.now()
        diff = (end - start).total_seconds()

        if sleep_time - diff > 0:
            time.sleep(sleep_time - diff)

        count = int(elem[2]) if elem[0] != this_building.outname else int(elem[1])

        try:
            url = "http://" + server_ip + ":" + str(server_port)
            response = requests.post(url, headers=headers, json=sensor_data)
            print(response.status_code)

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

    for _, thisEV in ev_list.items():
        sensor_data = {
            "building name": "",
            "elevator name": "",
            "underground floor": None,
            "ground floor": None,
            "each floor altimeter": None,
            "timestamp": None,
            "acceleration": None,
            "velocity": None,
            "max velocity": None,
            "altimeter": None,
            "temperature": None,
            "button inside": None,
            "button outside": None,
            "idle energy": None,
            "standby energy": None,
            "ref energy": None,
            "use energy calculation flag": False
        }

        http_header = set_http_header([""], thisEV, init_data, init_dt=1)

        sensor_data['building name'] = init_data.building_name
        sensor_data['elevator name'] = thisEV.ev_name
        sensor_data['timestamp'] = datetime.datetime.now().isoformat()
        sensor_data['underground floor'] = thisEV.underground_floor
        sensor_data['ground floor'] = thisEV.ground_floor
        sensor_data['each floor altimeter'] = thisEV.altimeter_list
        sensor_data['acceleration'] = thisEV.acceleration
        sensor_data['max velocity'] = thisEV.max_velocity

        if thisEV.energy_flag is True:
            sensor_data['use energy calculation flag'] = True
            sensor_data['idle energy'] = thisEV.E_idle
            sensor_data['standby energy'] = thisEV.E_standby
            sensor_data['ref energy'] = thisEV.E_ref

        send(http_header, sensor_data)

    return

if __name__ == '__main__':
    this_server, this_building = init.set_init_data()

    server_ip = this_server.ipaddr
    server_port = this_server.port

    init_dt_server_by_ev_list(this_building)

    if this_building.rts_path is not None:
        run_rts(this_building)

    else:
        run_test(this_building)
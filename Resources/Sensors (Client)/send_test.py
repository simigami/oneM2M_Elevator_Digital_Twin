# This file is responsilble for sending data to DT Server
# First This Will gather initial data from init.py
# Then, it will gather RTS information by using Gather_Sensor_Data.py
# It will concat init and sensor data, then send all information to DT Server
import datetime
import time
import socket
import json
import init_test as init
import requests
#import Gather_Sensor_Data
import ast
import shutil
import os
import socket
import Gather_Sensor_Datatest as GSD

# Name of the directory inside the current directory
directory_name = "Simulated Data Directory"
directory_path = os.path.join(os.getcwd(), directory_name)

class DataFormat:
    def __init__(self):
        self.data = {}
    def add(self, key, value):
        if key in self.data:
            print("cannot add existing key")
            return

        self.data[key] = value
        return

    def modify(self, key, new_value):
        if key not in self.data:
            print("cannot modify non-existing key")
            return

        self.data[key] = new_value

    def delete(self, key):
        if key not in self.data:
            print("cannot delete non-existing key")
            return

        self.data.pop(key, None)

    def get_value(self, key):
        if key not in self.data:
            print("cannot get non-existing key's value")
            return None

        return self.data[key]

def send(server, http_header, DataFormat):
    try:
        url = "http://" + server.ipaddr + ":" + str(server.port)
        response = requests.post(url, headers=http_header, json=json.dumps(DataFormat.data))
        return True

    except (socket.error, json.JSONDecodeError, socket.timeout) as e:
        print(f"Error sending JSON data: {e}")
        return False

def init_dt_server(server, building, each_elevator):
    sensor_data = DataFormat()

    http_header = {'Content-Type': 'application/json?ty=0'}

    sensor_data.add('building name', building.building_database.get_value('building name'))
    sensor_data.add('elevator name', each_elevator.default_info.get_value('elevator name'))
    sensor_data.add('timestamp', datetime.datetime.now().isoformat())
    sensor_data.add('underground floor', each_elevator.default_info.get_value('underground floor'))
    sensor_data.add('ground floor', each_elevator.default_info.get_value('ground floor'))
    sensor_data.add('each floor altimeter', each_elevator.default_info.get_value('each floor altimeter'))
    sensor_data.add('acceleration', each_elevator.default_info.get_value('acceleration'))
    sensor_data.add('max velocity', each_elevator.default_info.get_value('max velocity'))

    if each_elevator.default_info.get_value('energy consumption check') is True:
        sensor_data.add('idle energy', each_elevator.default_info.get_value('idle energy'))
        sensor_data.add('standby energy', each_elevator.default_info.get_value('standby energy'))
        sensor_data.add('ref energy', each_elevator.default_info.get_value('ref energy'))

    altimeter_dir_path = each_elevator.default_info.get_value('altimeter dir path')
    prev_data_save_dir_path = each_elevator.default_info.get_value('previous data save dir path')

    # Check dir exists, if not make one
    if altimeter_dir_path is not None and not os.path.exists(altimeter_dir_path):
        os.makedirs(altimeter_dir_path)

    if altimeter_dir_path is not None and not os.path.exists(prev_data_save_dir_path):
        os.makedirs(prev_data_save_dir_path)

    return send(server, http_header, sensor_data)

def debug(server, building, flag=True):
    min_opt = 0
    max_opt = 4
    path = None

    print("This section is for debugging")
    print("Please Enter a number to select the debugging option")
    print("1. Gather All Elevator Sensor Data using config.ini\n"
          "2. Send Building Sensor Data to DT Server (Generated)\n"
          "3. Send Building Sensor Data to DT Server (Real-Time)\n"
          "4. Init Test to DT Server\n"
          "0. Exit\n"
    )

    select = int(input())
    while select != 0:
        while select < min_opt or select > max_opt:
            print("Please select the number between 1 and 3")
            print("Select the number: ")
            select = int(input())

        if select == 1:
            path = debug_gather_sensor_data(building)

        elif select == 2:
            debug_send_sensor_data_g(server, building, path)

        elif select == 3:
            pass

        print("Job Done\n Please Enter a number to select the debugging option or exit to press 0")
        select = int(input())

def debug_gather_sensor_data(building) -> str | bytes | None:
    elevator_result_list = {}
    prev_check_mapper = {}
    list_of_timelines = {}
    src_path = os.getcwd()

    try:
        for each_elevator in building.elevator_list:
            ev = building.elevator_list[each_elevator]
            print("Debugging Elevator : ", ev.default_info.get_value('elevator name'))

            if ev.default_info.get_value('use camera sensor') is True:
                ev_name = ev.default_info.get_value('elevator name')
                picture_dir_path = ev.default_info.get_value('picture dir path')
                picture_dir_name = picture_dir_path.split('\\')[-1]

                result_write_dir = ev.default_info.get_value('previous data save dir path')
                result_write_path = os.path.join(result_write_dir, building.building_database.get_value(
                    'building name') + '_' + ev.default_info.get_value(
                    'elevator name') + '_' + picture_dir_name + ".txt")

                elevator_result_list[ev_name] = result_write_path
                prev_check_mapper[ev_name] = None
                # Check if result_write_path exists, if not make one
                if not os.path.exists(result_write_path):
                    res = GSD.get_sensor_datas_debug(ev_name, picture_dir_path, result_write_path)

                else:
                    print("Result Write Path Already Exists. Do you want to overwrite it? (y/n)")

                    res = input()
                    while res.lower() != 'y' and res.lower() != 'n':
                        print("Please enter y or n")
                        res = input()

                    if res.lower() == 'y':
                        res = GSD.get_sensor_datas_debug(ev_name, picture_dir_path, result_write_path, mode=1)
                    else:
                        continue

            else:
                continue

        for elem in elevator_result_list:
            with open(elevator_result_list[elem], 'r') as f:
                lines = f.readlines()

            # Convert Payload to List
            for i in range(len(lines)):
                lines[i] = ast.literal_eval(lines[i])

            for timeline in lines:
                t = timeline[0]
                c_detected = debug_convert_payload(timeline)

                if c_detected is None:
                    continue

                if t not in list_of_timelines:
                    list_of_timelines[t] = [[elem, "IN", c_detected]]

                else:
                    list_of_timelines[t].append([elem, "IN", c_detected])

        if list_of_timelines:
            building_prev_data_save_dir = building.building_database.get_value('previous data save dir path')
            now = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
            building_prev_data_save_name = building.building_database.get_value(
                'building name') + "_" + now + "_prev_data.txt"
            building_prev_data_save_path = os.path.join(building_prev_data_save_dir, building_prev_data_save_name)
            src_path = building_prev_data_save_path

            if not os.path.exists(building_prev_data_save_dir):
                os.makedirs(building_prev_data_save_dir)

            with open(building_prev_data_save_path, 'a') as f:
                for key, value in sorted(list_of_timelines.items()):
                    timedelta = key
                    for each_elevator_payload in value:
                        ev_name = each_elevator_payload[0]
                        ev_inside = each_elevator_payload[1]
                        ev_payload = each_elevator_payload[2]

                        if ev_payload == prev_check_mapper[ev_name]:
                            continue

                        f.write(str(ev_name) + " " + str(timedelta) + " " + str(ev_inside) + " " + str(ev_payload).replace(" ", "") + '\n')
                        prev_check_mapper[ev_name] = ev_payload

        else:
            Exception("list_of_timelines is Empty")

        # Ask If user want to overwrite this content into previous data save temp file
        print("Do you want to overwrite this content into previous data save temp file? (y/n)")
        res = input()
        while res.lower() != 'y' and res.lower() != 'n':
            print("Please enter y or n")
            res = input()

        if res.lower() == 'y':
            prev_dir_path = building.building_database.get_value('previous data save dir path')
            temp_name = building.building_database.get_value('previous data save temp file name')

            prev_path = os.path.join(prev_dir_path, temp_name + ".txt" if temp_name[-4:] != ".txt" else temp_name)

            #   Copy the content of building_prev_data_save_path to prev_path
            shutil.copyfile(src_path, prev_path)

        return src_path

    except Exception as e:
        print(f"Error in Debug Gather Sensor Data: {e}")
        return None

def debug_convert_payload(payload) -> list | None:
    new_payload = []
    contents = payload[1]

    if not contents:
        return []

    try:
        for elem in contents:
            if elem.isdigit():
                new_payload.append(int(elem))

            else:
                first_letter = elem[0]
                leftover = elem[1:]

                if first_letter == 'B' and leftover.isdigit():
                    new_payload.append(int(-1 * int(leftover)))

                else:
                    new_payload.append(elem)

        return new_payload

    except Exception as e:
        print(f"Error in Debug Convert Payload: {e}")
        return None


def debug_send_sensor_data_g(server, building, path_debug):
    try:
        path = None

        if path_debug is None:
            print("Debug path is None\nCheck config.ini path")

            prev_dir_path = building.building_database.get_value('previous data save dir path')
            temp_name = building.building_database.get_value('previous data save temp file name')

            prev_path = os.path.join(prev_dir_path, temp_name+".txt" if temp_name[-4:] != ".txt" else temp_name)

            if not os.path.exists(prev_path):
                Exception("Previous Data Save Dir Path Does Not Exist")

            path = prev_path

        else:
            path = path_debug

        with open(path, 'r') as f:
            lines = f.readlines()

        timedelta = 0
        for line in lines:
            data = line.strip().split(' ')

            ev_name = str(data[0])
            ev_timeline = int(data[1])

            header = set_http_header(building.elevator_list[ev_name], data)
            body = set_http_body(building.elevator_list[ev_name], data)

            if body is None:
                continue

            body.add('building name', building.building_database.get_value('building name'))

            time.sleep(ev_timeline - timedelta if ev_timeline != timedelta > 0 else 0)
            timedelta = ev_timeline if ev_timeline != timedelta else timedelta

            if not send(server, header, body):
                Exception("Error in Sending Data")

        return True

    except (socket.error, json.JSONDecodeError, socket.timeout) as e:
        print(f"Error sending JSON data: {e}")
        return False

def debug_send_sensor_data_r(server, building, path_debug):
    pass

def set_http_header(ev, data) -> dict:
    headers = {
        "Content-Type": "application/json?"
        # ?ty=0 // Init Elevator List to Digital Twin Server
        # ?ty=1 // Signal From Outside of Elevator
        # ?ty=2 // Signal From Inside of Elevator that does not holds physical values
        # ?ty=3 // Inside of Elevator that holds physical values

        # en option only works when ty=2
        # ?en=1 // Use Energy Consumption
        # ?en=0 // Do Not Use Energy Consumption
    }

    energy_flag = ev.default_info.get_value('energy consumption check') if ev.default_info.get_value('energy consumption check') is not None else False
    physical_flag = ev.default_info.get_value('use physical system information check') if ev.default_info.get_value('use physical system information check') is not None else False

    if data[2] == "OUT":
        headers["Content-Type"] += "ty=1"

    elif physical_flag is False:
        headers["Content-Type"] += "ty=2"

    elif physical_flag is True:
        headers["Content-Type"] += "ty=3"

    if energy_flag is True:
        headers["Content-Type"] += "&en=1"

    else:
        headers["Content-Type"] += "&en=0"

    return headers

def set_http_body(ev, data) -> DataFormat | None:
    body = DataFormat()
    ev_type = str(data[2])
    ev_payload = ast.literal_eval(data[3])

    if ev_type.lower() == "out":
        direction = str(data[-1]).lower()
        body.add('button outside', ev_payload)
        body.add('button outside direction', direction)

    elif ev_type.lower() == "in":
        body.add('button inside', ev_payload)

    else:
        print("Unknown Elevator Type\nSkipping this data")
        return None

    for key in ev.default_info.db:
        body.add(key, ev.default_info.db[key])

    return body

if __name__ == '__main__':
    # Get Initial Data
    server, building = init.set_init_data()
    RTS_BAN_FLAG = False

    # Init DT Server
    for each_elevator in building.elevator_list:
        if not init_dt_server(server, building, building.elevator_list[each_elevator]):
            print("DT Server Not Initial")
            RTS_BAN_FLAG = True

    # Client Decides to use RTS, Previous Data, Debugging.
    print("Client Decides to use RTS, Previous Data, Debugging.")
    print("1. Use RTS\n2. Use Previous Data\n3. Debugging\n")
    print("Select the number: ")
    select = int(input())

    while select < 1 or select > 3:
        print("Please select the number between 1 and 3")
        print("Select the number: ")
        select = int(input())

    if not RTS_BAN_FLAG and select == 1:
        # Use RTS
        pass
    elif not RTS_BAN_FLAG and select == 2:
        # Use Previous Data
        pass
    elif select == 3:
        # Debugging
        debug(server, building, flag=RTS_BAN_FLAG)
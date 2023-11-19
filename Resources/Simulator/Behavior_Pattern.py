import copy
import datetime
import math
import sys
import json

import Calculation
from config import TEST_VARIABLES, TEST_ELEVATOR

second = 1
minute = 60 * second
hour = 60 * minute

class LinkedList:
    def __init__(self):
        self.head = None
        self.last = None
        self.count = 0

    def add_node(self, node):
        if self.head is not None and self.last is not None:
            self.last.next = node
            node.prev = self.last
            self.last = node
        else:
            self.head = node
            self.last = node
        self.count += 1

    def get_last(self):
        return self.last

    def get_head(self):
        return self.head

    def print_all_node(self):
        pointer = self.head
        while pointer is not None:
            pointer.pattern.print_var()
            if pointer.energy is not None:
                pointer.energy.print_energy()
            self.parse_node_to_json(pointer)
            pointer = pointer.next

    def parse_node_to_json(self, pointer):
        json_list_data = {
            'default_info' : pointer.pattern.parse_data_instance.data,
            'trip_path' : pointer.pattern.trip_path,
            'floor_to_floor_metrix' : pointer.pattern.floor_dict,
            'trip_count' : pointer.pattern.trip_count,
            'trip_distance' : pointer.pattern.trip_distance,
            'total_distance' : pointer.pattern.total_distance
        }
        json_list = json.dumps(json_list_data)

        path = rf"E:\ML\Elevator Git\Elevator_Results\temp.json"
        with open(path, "w") as json_file:
            json.dump(json_list_data, json_file, indent=4)

class Node:
    def __init__(self):
        self.pattern = None
        self.energy = None
        self.timestamp = None
        self.next = None
        self.prev = None

    def set_pattern(self, pattern, timestamp):
        self.pattern = pattern
        self.timestamp = timestamp

    def set_energy(self, energy):
        self.energy = energy

class patt:
    def __init__(self):
        self.parse_data_instance = parse_data()
        self.on_time = 0
        self.trip_path = []
        self.trip_count = 0
        self.trip_direction = None  # True = Up, False = Down
        self.trip_distance = None
        self.total_distance = 0
        self.floor = None
        self.floor_int = None
        self.floor_dict = build_floor_dict()
        self.what_is_done = None
        self.what_is_new = None
        self.latest_stop = None

    def set_on_time(self, value):
        self.on_time = value

    def reset_trip_path(self):
        self.trip_path = []

    def add_trip_path(self, floor):
        self.trip_path.append(floor)

    def pop_trip_path(self, floor):
        self.trip_path.remove(floor)

    def remove_trip_path(self, trip_path, floor):
        if floor in trip_path:
            trip_path.remove(floor)
        else:
            print("Error in trip_path, no such floor in list")

    def print_floor_dict_all(self):
        print(self.floor_dict)

    def print_floor_dict_specific(self, floor):
        print(self.floor_dict[floor])

    def print_trip_count(self):
        print(self.trip_count)

    def print_trip_distance(self):
        print(self.trip_distance)

    def print_total_distance(self):
        print(self.total_distance)

    def print_var(self):
        print("default info : {}\ntrip path : {}\nfloor to floor metrix : {}\ntrip count : {}\ntrip distance : {}\ntotal distance : {}\n".format(
        self.parse_data_instance.data,
        self.trip_path,
        self.floor_dict,
        self.trip_count,
        self.trip_distance,
        self.total_distance))

class parse_data:
    def __init__(self):
        self.data = {
            'Inout': '',
            'Timestamp': '',
            'Operation': '',
            'Previous Button Panel': '',
            'Current Button Panel' : '',
            'Current Floor' : [0, 0],
            'Elevator Number' : 0,
            'Altimeter' : 0
        }

def build_floor_dict():
    if(TEST_VARIABLES.underground_floors == None or TEST_VARIABLES.ground_floors == None):
        print("Error on building floor data, floor data is None")

        return None
    else:
        dict_dict = {}
        for i in range(1, TEST_VARIABLES.underground_floors + 1):
            dict = {}
            for j in range(1, TEST_VARIABLES.underground_floors+1):
                dict[f'B{j}'] = 0

            for j in range(1, TEST_VARIABLES.ground_floors+1):
                dict[f'{j}'] = 0
            dict['Alt'] = TEST_VARIABLES.underground_alts[-i]

            dict_dict[f'B{i}'] = dict

        for i in range(1, TEST_VARIABLES.ground_floors + 1):
            dict = {}
            for j in range(1, TEST_VARIABLES.underground_floors+1):
                dict[f'B{j}'] = 0

            for j in range(1, TEST_VARIABLES.ground_floors+1):
                dict[f'{j}'] = 0
            dict['Alt'] = TEST_VARIABLES.ground_alts[i-1]

            dict_dict[f'{i}'] = dict

    return dict_dict

def parse_button_list(pattern):
    pbl = pattern.parse_data_instance.data['Previous Button Panel']
    pbl_int = []
    cbl = pattern.parse_data_instance.data['Current Button Panel']
    cbl_int = []
    cf = pattern.parse_data_instance.data['Current Floor']

    for elem in pbl:
        if elem[0] == 'B':
            button_int = int(elem[1]) * -1
        elif elem[0] == 'C' or elem[0] == 'O':
            button_int = 0
        else:
            button_int = int(elem)
        pbl_int.append(button_int)

    for elem in cbl:
        if elem[0] == 'B':
            button_int = int(elem[1]) * -1
        elif elem[0] == 'C' or elem[0] == 'O':
            button_int = 0
        else:
            button_int = int(elem)
        cbl_int.append(button_int)

    floor_int = 0
    for elem in cf:
        if elem[0] == 'B':
            floor_int += int(elem[1]) * -1
        elif elem[0] == 'C' or elem[0] == 'O':
            pass
        else:
            floor_int += int(elem)

    floor_int = round(floor_int / 2, 1)
    what_is_done = list(set(pbl_int) - set(cbl_int))
    what_is_new = list(set(cbl_int) - set(pbl_int))

    # print(what_is_done)
    # print(what_is_new)

    pattern.floor_int = floor_int
    pattern.what_is_done = what_is_done
    pattern.what_is_new = what_is_new

    return pattern

def refresh_trip_path(pattern):

    if len(pattern.trip_path) == 0: # If trip_path is empty, it means Elevator is now operating with a single-side direction
        if len(pattern.what_is_new) == 0:
            print("Error, what_is_new should have a value")
        else:
            if pattern.floor_int < pattern.what_is_new[0]:  # If Lowest Detected Button is Upper than current detected Altimeter, It moves Up
                pattern.trip_direction = True
            else:
                pattern.trip_direction = False
            what_is_new = sorted(pattern.what_is_new, reverse=(not pattern.trip_direction))

            floor_int = math.floor(pattern.floor_int)
            pattern.latest_stop = floor_int

            for elem in what_is_new:
                pattern.add_trip_path(elem)

    else:
        if pattern.trip_direction is None:
            print("Error, Trip_direction is None")
        else:
            what_is_done = sorted(pattern.what_is_done, reverse=(not pattern.trip_direction))
            what_is_new = sorted(pattern.what_is_new, reverse=(not pattern.trip_direction))

            if len(what_is_done) != 0:
                for elem in what_is_done:
                    if elem != 0:   # if floor is not Closed or Open = 0
                        #print("Stop at {}, Start From {}\n".format(elem, pattern.latest_stop))
                        pattern = calculate_trip_distance(pattern, pattern.latest_stop, elem)

                        pattern.pop_trip_path(elem)
                        pattern.trip_count += 1
                        pattern.latest_stop = elem

            if len(what_is_new) != 0:
                for elem in what_is_new:
                    if elem != 0:  # if floor is not Closed or Open = 0
                        pattern.add_trip_path(elem)

            if len(pattern.trip_path) == 0:
                pattern.trip_direction = None

    return pattern


def calculate_trip_distance(pattern, start, end):
    if TEST_VARIABLES.total_floors != (len(TEST_VARIABLES.underground_alts) + len(TEST_VARIABLES.ground_alts)):
        print("Error in Calculation, total floor and Altimeter length is mismatched")
    else:
        if start >= 0:
            start_str = str(start)
        else:
            start_str = 'B'+str(-1*start)

        if end >= 0:
            end_str = str(end)
        else:
            end_str = 'B'+str(-1*end)

        pattern.trip_distance = abs(pattern.floor_dict[start_str]['Alt']-pattern.floor_dict[end_str]['Alt'])
        pattern.total_distance += pattern.trip_distance

        return pattern

def add_where_to_where(pattern):
    pbl = pattern.parse_data_instance.data['Previous Button Panel']
    cbl = pattern.parse_data_instance.data['Current Button Panel']

    what_is_new = list(set(cbl) - set(pbl))

    if len(pattern.what_is_new) != 0:
        current_floor = str(math.floor(pattern.floor_int))
        for elem in what_is_new:
            if elem != 'Close' and elem != 'Open':
                pattern.floor_dict[current_floor][elem] += 1

    return pattern

def check_interval_passed(LL, interval):
    v_hour = interval // hour
    v_minute = (interval % hour) // minute
    v_second = (interval % hour) % minute

    head_time = LL.head.timestamp
    end_time = LL.last.timestamp
    on_time = end_time - head_time

    if head_time != end_time:
        day_bar_interval = round(datetime.timedelta(days=1) / (end_time-head_time))

        #print(end_time-head_time)
        #if (end_time-head_time) >= datetime.timedelta(hours=v_hour, minutes=v_minute, seconds=v_second):
        temp = Calculation.Config_Elevator_Specification()

        temp2 = Calculation.values()
        temp2.set_cb(0.5)
        temp2.set_h(1)
        temp2.set_max_load(TEST_ELEVATOR.Luxen_capacity)
        temp2.set_trip(LL.last.pattern.trip_count * day_bar_interval)

        temp2.set_ref_cycle_energy(TEST_ELEVATOR.Luxen_ref_cycle_energy)
        temp2.set_ref_cycle_distance(TEST_VARIABLES.total_height)

        temp2.set_short_cycle_energy(TEST_ELEVATOR.Luxen_short_cycle_energy)
        temp2.set_short_cycle_distance(TEST_ELEVATOR.Luxen_short_cycle_distance)

        temp.set_values(temp2)
        temp.get_energy()   # Calculate Energy Consumption

        node = Calculation.energy_node()
        node.set_vaule(temp)

        LL.last.energy = node
        LL.last.pattern.set_on_time(on_time)

        #temp.print_energy()
        #print(end_time - head_time)

def parse_log_to_log_array(file_name):
    with open(file_name, 'r') as f:
        log = f.read()

    paragraphs = log.strip().split('\n\n')
    pattern = patt()

    arr = []
    for paragraph in paragraphs:
        arr.append(paragraph)

    return arr

def parse_log_array_to_node(elem, LL, pattern):
    #pattern = patt()
    lines = elem.strip().split('\n')

    if len(lines) == 7:
        for i in range(len(lines)):
            #print(lines[i])
            if i != 2:
                value = lines[i].split(': ')
                pattern.parse_data_instance.data[value[0]] = value[1]
            else:
                key, value = 'Operation', lines[i]
                pattern.parse_data_instance.data[key] = value

        pattern.parse_data_instance.data['Previous Button Panel'] = eval(
            pattern.parse_data_instance.data['Previous Button Panel'])
        pattern.parse_data_instance.data['Current Button Panel'] = eval(
            pattern.parse_data_instance.data['Current Button Panel'])
        floor_list = pattern.parse_data_instance.data['Current Floor'].split(' between ')
        pattern.parse_data_instance.data['Current Floor'] = [floor_list[0], floor_list[1]]

        pattern = parse_button_list(pattern)
        pattern = add_where_to_where(pattern)
        pattern = refresh_trip_path(pattern)

        pattern_copy = copy.deepcopy(pattern)

        temp = Node()
        temp.set_pattern(pattern_copy, datetime.datetime.strptime(pattern_copy.parse_data_instance.data['Timestamp'],
                                                                  '%Y_%m%d_%H%M%S'))
        #temp.pattern.print_var()

        LL.add_node(temp)
        check_interval_passed(LL, 1 * minute)

        return LL

    else:
        print("Error in lines, log attribute does not match")

        return None

def init():
    TEST_VARIABLES.total_height = abs(TEST_VARIABLES.ground_alts[-1] - TEST_VARIABLES.underground_alts[0])
    TEST_VARIABLES.short_cycle_height = round(TEST_VARIABLES.total_height/4, 1)
    LL = LinkedList()

    return LL

if __name__ == '__main__':
    LL = init()

    path = rf"E:\ML\Elevator Git\Elevator_Results\temp.txt"
    LL = parse_log_to_log_array(path)
    LL.print_all_node()
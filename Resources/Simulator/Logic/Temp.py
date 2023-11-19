import copy
import datetime
import gc
import math
import queue
import sys
import json

import Calculation, Simulation
from config import TEST_VARIABLES, TEST_ELEVATOR

one_tenth_second = 1
second = 10
minute = 60 * second
hour = 60 * minute

# OPCODE
# 0 = Initial Value
# 1 = Elevator response from OUT Action and move to destination floor
# 2 = Elevator Detects New Button and re-coordinate its location
# 3 = Elevator Detects Button Disappear and re-coordinate its location
# 10 = Elevator finishes its trip and turns to IDLE + STANDBY
# 11 = Elevator turns to OPERATION mode and start moving or while moving
# 21 = Elevator decelerate to stop at floor due to OUT Action
# 22 = Elevator decelerate to stop at floor due to IN Action
# 100 = Elevator Emergency Halt

class System:
    def __init__(self):
        self.underground_floors = TEST_VARIABLES.underground_floors
        self.ground_floors = TEST_VARIABLES.ground_floors
        self.alts = TEST_VARIABLES.alts
        self.alts_dict = {}
        self.elevators = []
        self.add_elevators()
        self.make_altimeter_dictionary(self.underground_floors, self.ground_floors, self.alts)
        self.latest_log_timestamp = []
        self.latest_job_done_timestamp = []

    def make_altimeter_dictionary(self, underground_floors, ground_floors, alts):
        index = 0
        for i in range(underground_floors, 0, -1):
            str_i = str(i)
            str_floor = "B"+str_i
            self.alts_dict[str_floor] = alts[index]
            index += 1

        index = 0
        for i in range(1, ground_floors+1, 1):
            str_floor = str(i)
            self.alts_dict[str_floor] = alts[index+underground_floors]
            index += 1

    def set_latest_log_timestamp(self, timestamp):
        self.latest_log_timestamp.append(timestamp)

    def get_latest_log_timestamp(self, default=-1):
        return self.latest_log_timestamp[default]

    def set_job_done_timestamp(self, timestamp):
        self.latest_job_done_timestamp.append(timestamp)

    def get_job_done_timestamp(self, default=-1):
        return self.latest_job_done_timestamp[default]

    def add_elevators(self):
        temp = Elevator()
        self.elevators.append(temp)

    def get_number_of_elevators(self):
        return len(self.elevators)

    def get_elevators(self):
        return self.elevators

    def get_elevator(self, index):
        return self.elevators[index]

    def get_elevator_current_stopped_floor(self, index):
        elevator = self.get_elevator(index)
        elevator_floor = elevator.get_current_stopped_floor()

        return elevator_floor

    def get_delta_altimeter_altimeter_and_floor(self, start_altimeter, destination_floor):
        if destination_floor < 0:
            delta_index = abs(self.underground_floors) - abs(destination_floor)
            floor_altimeter = self.alts[delta_index]

        else:
            floor_altimeter = self.alts[self.underground_floors + destination_floor - 1]

        if start_altimeter < floor_altimeter:
            direction = True # True = Up, False = Down
        else:
            direction = False

        delta_altimeter = abs(start_altimeter - floor_altimeter)

        if direction:
            return [start_altimeter, floor_altimeter, delta_altimeter, direction]

        else:
            return [floor_altimeter, start_altimeter, delta_altimeter, direction]



    def get_delta_altimeter_floor_and_floor(self, start, dst):
        if start < dst:
            direction = True # True = Up, False = Down
        else:
            direction = False

        higher_floor = max(start, dst)
        lower_floor = start if higher_floor == dst else dst

        if higher_floor < 0:
            delta_index = abs(self.underground_floors) - abs(higher_floor)
            higher_floor_alts = self.alts[delta_index]

        else:
            higher_floor_alts = self.alts[self.underground_floors + higher_floor - 1]

        if lower_floor < 0:
            delta_index = abs(self.underground_floors) - abs(lower_floor)
            lower_floor_alts = self.alts[delta_index]

        else:
            lower_floor_alts = self.alts[self.underground_floors + lower_floor - 1]

        delta_altimeter = abs(higher_floor_alts - lower_floor_alts)

        if direction:
            return [lower_floor_alts, higher_floor_alts, delta_altimeter, direction]

        else:
            return [higher_floor_alts, lower_floor_alts, delta_altimeter, direction]


class Elevator:
    def __init__(self):
        self.opcode = 0
        self.entire_trip_list = []
        self.entire_trip_list_pointer = None
        self.first_log_timestamp = None
        self.temp_log_timestamp = 0

        self.current_trip_list = TripList()
        self.current_trip_node = None
        self.last_frame_node = None

        self.reserved_trip_queue = queue.Queue()

        self.current_stopped_floor = -5
        self.current_stopped_altimeter = -55

        self.velocity = 0
        self.maximum_velocity = 2.5
        self.time_to_reach_maximum_velocity = 2.0
        self.time_to_reach_maximum_velocity_trip = 4.0
        self.acceleration = 1.25
        self.direction = True

        self.closest_floor = {}
        self.current_altimeter = None

        self.current_trip_start_time = None
        self.current_trip_end_time = None

        self.trip_count = 0

    def init_current_trip_list(self):
        self.current_trip_list = TripList()

    def set_current_trip_start_time(self, timestamp):
        self.current_trip_start_time = timestamp
    def get_current_trip_start_time(self):
        return self.current_trip_start_time
    def set_current_trip_end_time(self, timestamp):
        self.current_trip_end_time = timestamp
    def get_current_trip_end_time(self):
        return self.current_trip_end_time

    def add_trip_count(self, value):
        self.trip_count += value

    def get_trip_count(self):
        return self.trip_count

    def get_direction(self):
        return self.direction

    def set_direction(self, direction):
        self.direction = direction

    def set_opcode(self, op):
        self.opcode = op

    def get_opcode(self):
        return self.opcode

    def get_current_stopped_floor(self):
        return self.current_stopped_floor

    def set_current_stopped_floor(self, value):
        self.current_stopped_floor = value

    def get_acceleration(self):
        return self.acceleration

    def get_velocity(self):
        return self.velocity

    def add_velocity(self, value):
        self.velocity += value

    def set_velocity(self, value):
        self.velocity = value

    def set_closest_floor(self, dict):
        self.closest_floor = dict.copy()

    def set_current_trip_list(self, trip):
        self.current_trip_list = trip
    def get_current_trip_list(self):
        return self.current_trip_list
class TripList:
    def __init__(self):
        self.head = None
        self.log_time = None
        self.TTR = None
        self.end_time = None

        self.start_floor = None
        self.destination_floor = None
        self.altimeter_array = None

        self.current_node_pointer = None
        self.direction = None

    def append(self, new_node):
        if self.head is None:
            self.head = new_node
        else:
            temp = self.head
            while temp.next:
                temp = temp.next

            new_node.prev = temp
            temp.next = new_node

    def move_pointer_to_next(self, value):
        if self.current_node_pointer is not None:
            for i in range(value):
                if self.current_node_pointer is None:
                    return None

                else:
                    self.current_node_pointer = self.current_node_pointer.next

        return self.current_node_pointer

    def remove(self, current):
        head = self.head

        if current is head:
            next = current.next
            if next is not None:
                current.next = None
                next.prev = None
                self.head = next
            else:
                self.head = None

    def display(self):
        temp = self.head
        while temp:
            if temp.start_time is not None:
                temp.display()

            temp = temp.next

    def set_end_time(self):
        self.end_time = self.log_time + datetime.timedelta(seconds=(self.TTR))

    def get_end_time(self):
        return self.end_time

    def set_start_floor(self, value):
        self.start_floor = value

    def set_destination_floor(self, value):
        self.destination_floor = value

    def get_start_floor(self):
        return self.start_floor

    def get_destination_floor(self):
        return self.destination_floor

    def set_log_time(self, start_time):
        self.start_time = start_time

    def set_TTR(self, TTR):
        self.TTR = TTR

    def get_TTR(self):
        return self.TTR

    def set_direction(self):
        if self.start_floor is not None and self.destination_floor is not None:
            if self.start_floor < self.destination_floor:
                self.direction = True
            else:
                self.direction = False

        else:
            self.direction = None

    def display(self):
        print(f"Opertaion Start Time : {self.log_time}\nTTR : {self.TTR}\nStart Floor : {self.start_floor}\nDestination Floor : {self.destination_floor}\n")

        node = self.head
        while node is not None:
            print(f"This Node Time : {node.current_time}\nTime Elapsed: {node.elapsed_time/1000} seconds\nAltimeter: {node.altimeter} meters\nClosest Floor: {node.closest_floor}\n")
            node = node.next

class TripNode:
    def __init__(self):
        self.current_time = None
        self.elapsed_time = None

        self.altimeter = None

        self.velocity = None
        self.status = None  # 0 = At maximum speed, 1 = is accelerate, 2 = is decelerate
        self.closest_floor = {}

        self.next = None
        self.prev = None

    def set_current_time(self, value):
        self.current_time = value

    def get_current_time(self):
        return self.current_time

    def set_elapsed_time(self, elapsed_time):
        self.elapsed_time = elapsed_time
    def set_velocity(self, velocity):
        self.velocity = velocity

    def get_velocity(self):
        return self.velocity
    def set_altimeter(self, altimeter):
        self.altimeter = altimeter

    def get_altimeter(self):
        return self.altimeter

    def set_closest_floor(self, closest_floor):
        self.closest_floor = closest_floor.copy()

    def get_status(self):
        return self.status

    def get_closet_upper_floor(self):
        str_floor =  self.closest_floor['upper_floor']
        if str_floor[0] == 'B':
            floor = int(str_floor[1:]) * -1
        else:
            floor = int(str_floor)

        return floor

def time_to_reach(elevator, altimeter_difference):
    a = elevator.acceleration
    t = elevator.time_to_reach_maximum_velocity
    at = a * t
    minimum_acceleration_distance = 2 * at

    if altimeter_difference <= minimum_acceleration_distance:
        TTR = math.sqrt(altimeter_difference * 4 /a)

        return [TTR, False]
    else:
        TTR = (altimeter_difference/at) + t

        return [TTR, True]

def find_closest_floors(alts_dict, current_altimeter):
    closest_floors = {
        'upper_floor': None,
        'lower_floor': None
    }
    closest_upper_diff = float('inf')
    closest_lower_diff = float('inf')

    for floor, altimeter in alts_dict.items():
        diff = current_altimeter - altimeter

        if diff < 0 and abs(diff) < closest_upper_diff:
            closest_upper_diff = abs(diff)
            closest_floors['upper_floor'] = floor

        elif diff > 0 and abs(diff) < closest_lower_diff:
            closest_lower_diff = abs(diff)
            closest_floors['lower_floor'] = floor

        elif diff == 0: ## if diff is 0
            closest_floors['upper_floor'] = floor
            closest_floors['lower_floor'] = floor

            break

    return closest_floors

def analyze_out_log(latest_out_log):
    #latest_out_log.display()

    inout = latest_out_log.get_inout()
    floor = latest_out_log.get_out_floor()
    timestamp = latest_out_log.get_timestamp()
    direction = latest_out_log.get_direction()

    return [inout, floor, timestamp, direction]

def time_to_reach(elevator, altimeter_difference):
    a = elevator.acceleration
    t = elevator.time_to_reach_maximum_velocity
    at = a * t
    minimum_acceleration_distance = 2 * at

    if altimeter_difference <= minimum_acceleration_distance:
        TTR = math.sqrt(altimeter_difference * 4 /a)

        return TTR
    else:
        TTR = (altimeter_difference/at) + t

        return TTR

def time_to_reach_during_moving(elevator, altimeter_difference):
    elevator_current_velocity = elevator.get_velocity()
    elevator_maximum_velocity = elevator.maximum_velocity

    a = elevator.acceleration
    t = elevator.time_to_reach_maximum_velocity

    if elevator_current_velocity == elevator_maximum_velocity:
        altimeter_to_stay_maximum = altimeter_difference - (0.5 * a * t * t)
        time_to_stay_maximum = round(altimeter_to_stay_maximum / elevator_maximum_velocity, 2)

        TTR = time_to_stay_maximum + t

        return [TTR, 0, time_to_stay_maximum, t]

    else:
        velocity_to_reach_maximum = elevator_maximum_velocity - elevator_current_velocity
        time_to_reach_maximum_velocity = round(velocity_to_reach_maximum / a, 2)

        altimeter_to_reach_maximum_velocity = (elevator_current_velocity + elevator_maximum_velocity) * time_to_reach_maximum_velocity * 0.5
        altimeter_to_stay_maximum = round(altimeter_difference - altimeter_to_reach_maximum_velocity - (0.5 * a * t * t), 2)

        time_to_stay_maximum = altimeter_to_stay_maximum / elevator_maximum_velocity

        TTR = time_to_reach_maximum_velocity + time_to_stay_maximum + t

        return [TTR, time_to_reach_maximum_velocity, time_to_stay_maximum, t]

def make_trip_list(elevator, information_array):
    # altimeter_array = [start_floor_alts , destination_floor_alts, delta_altimeter, direction]
    # information_array = [elevator_current_floor, destination_floor, altimeter_array, TTR]
    trip_list = TripList()

    trip_list.start_floor = information_array[0]
    trip_list.destination_floor = information_array[1]
    trip_list.altimeter_array = information_array[2]
    trip_list.TTR = information_array[3]

    if trip_list.start_floor < trip_list.destination_floor:
        trip_list.direction = True
    else:
        trip_list.direction = False

    return trip_list

def put_trip_node_data_into_trip_list(elevator, TTR_array, f, direction=True, TTS=3):
    #
    # TTR_array = [TTR, time_to_reach_maximum_velocity, time_to_stay_maximum, t]
    millisecond = 1000
    trip_list = elevator.trip_list

    acceleration = elevator.get_acceleration()
    elevator_velocity = elevator.get_velocity()
    t = elevator.time_to_reach_maximum_velocity

    TTR = TTR_array[0]
    time_to_reach_maximum_velocity = TTR_array[1] * millisecond
    time_to_stay_maximum = TTR_array[2] * millisecond
    time_to_decelerate_to_zero = TTR_array[3] * millisecond

    TTR_to_microsecond = round(int(TTR * millisecond), -2)
    TTS_to_microsecond = round(int(TTS * millisecond), -2)

    current_velocity = elevator_velocity
    current_altimeter = altimeter_array[0]

    if elevator.temp_log_timestamp == 0:
        elevator.temp_log_timestamp = elevator.first_log_timestamp

    if direction is True:
        if TTR >= elevator.time_to_reach_maximum_velocity_trip:

            for elapsed_time in range(0, TTR_to_microsecond+100, 100):  # Loop through each 0.1 second
                if elapsed_time < time_to_reach_maximum_velocity:  # Acceleration phase
                    before = current_velocity
                    current_velocity += acceleration * 0.1
                    after = current_velocity

                    current_altimeter += ((before + after) * 0.1 * 0.5)
                    #print(f"E time : {elapsed_time}, before : {before}, after : {after}")

                elif time_to_reach_maximum_velocity <= elapsed_time < time_to_reach_maximum_velocity + time_to_stay_maximum:  # Constant speed phase
                    current_altimeter += (current_velocity * 0.1)
                    #print(f"E time : {elapsed_time}")

                elif time_to_reach_maximum_velocity + time_to_stay_maximum <= elapsed_time < time_to_reach_maximum_velocity + time_to_stay_maximum + time_to_decelerate_to_zero:  # Deceleration phase
                    before = current_velocity
                    current_velocity += (acceleration * -0.1)
                    after = current_velocity

                    current_altimeter += ((before + after) * 0.1 * 0.5)
                    #print(f"E time : {elapsed_time}, before : {before}, after : {after}")

                elif elapsed_time == TTR_to_microsecond:
                    current_altimeter = altimeter_array[1]

                closest_floors = find_closest_floors(System.alts_dict, current_altimeter)
                #print(f"E time : {elapsed_time}, velocity : {current_velocity}, altimeter : {current_altimeter}")

                temp_node = TripNode()

                temp_node.set_elapsed_time(elapsed_time)

                temp_node.set_current_time(elevator.temp_log_timestamp+datetime.timedelta(seconds=elapsed_time/millisecond))

                temp_node.set_velocity(current_velocity)
                temp_node.set_altimeter(current_altimeter)
                temp_node.set_closest_floor(closest_floors)

                trip_list.append(temp_node)

                if elapsed_time == TTR_to_microsecond:
                    elevator.temp_log_timestamp = temp_node.get_current_time()

            for elapsed_time in range(0, TTS_to_microsecond+100, 100):  # Loop through each 0.1 second
                closest_floors = find_closest_floors(System.alts_dict, current_altimeter)

                temp_node = TripNode()

                temp_node.set_elapsed_time(elapsed_time)
                temp_node.set_current_time(elevator.temp_log_timestamp+datetime.timedelta(seconds=elapsed_time/millisecond))

                temp_node.set_velocity(current_velocity)
                temp_node.set_altimeter(current_altimeter)
                temp_node.set_closest_floor(closest_floors)

                trip_list.append(temp_node)

                if elapsed_time == TTS_to_microsecond:
                    elevator.temp_log_timestamp = temp_node.get_current_time()

            return elevator

        else:
            for elapsed_time in range(0, TTR_to_microsecond, 100):  # Loop through each 0.1 second
                if elapsed_time == 0:
                    continue

                elif elapsed_time <= TTR_to_microsecond/2:  # Acceleration phase
                    before = current_velocity
                    current_velocity += acceleration * 0.1
                    after = current_velocity

                    current_altimeter += ((before + after) * 0.1 * 0.5)

                elif elapsed_time > TTR_to_microsecond/2:
                    before = current_velocity
                    current_velocity += (acceleration * -0.1)
                    after = current_velocity

                    current_altimeter += ((before + after) * 0.1 * 0.5)

                if elapsed_time == (TTR_to_microsecond-100):
                    current_altimeter = trip_list.altimeter_array[0] + trip_list.altimeter_array[2]

                closest_floors = find_closest_floors(System.alts_dict, current_altimeter)

                temp_node = TripNode()

                temp_node.set_elapsed_time(elapsed_time)
                temp_node.set_current_time(elevator.temp_log_timestamp+datetime.timedelta(seconds=elapsed_time/millisecond))

                temp_node.set_velocity(elevator.get_velocity())
                temp_node.set_altimeter(current_altimeter)
                temp_node.set_closest_floor(closest_floors)

                trip_list.append(temp_node)

                if elapsed_time == TTR_to_microsecond:
                    elevator.temp_log_timestamp = temp_node.get_current_time()

            for elapsed_time in range(0, TTS_to_microsecond+100, 100):  # Loop through each 0.1 second
                closest_floors = find_closest_floors(System.alts_dict, current_altimeter)

                temp_node = TripNode()

                temp_node.set_elapsed_time(elapsed_time)
                temp_node.set_current_time(elevator.temp_log_timestamp+datetime.timedelta(seconds=elapsed_time/millisecond))

                temp_node.set_velocity(current_velocity)
                temp_node.set_altimeter(current_altimeter)
                temp_node.set_closest_floor(closest_floors)

                trip_list.append(temp_node)

                if elapsed_time == TTS_to_microsecond:
                    elevator.temp_log_timestamp = temp_node.get_current_time()

            return elevator

    else:
        pass
        #System.set_job_done_timestamp(temp_node.get_end_time())
    return elevator

def set_trip_start_and_end_time(elevator, log):
    temp = 3
    current_trip_end_time = elevator.current_trip_end_time

    if log is not None:
        log_timestamp = log.data.timestamp
        if current_trip_end_time is not None and current_trip_end_time < log_timestamp:
            log_timestamp = current_trip_end_time + datetime.timedelta(seconds=temp)
        else:
            pass

    else:
        log_timestamp = elevator.current_trip_end_time + datetime.timedelta(seconds=temp)

    current_trip_list = elevator.current_trip_list
    TTR = current_trip_list.TTR

    current_trip_list.log_time = log_timestamp
    current_trip_list.end_time = log_timestamp + datetime.timedelta(seconds=TTR)

    # Now Update Elevator's Start and End Time
    elevator.set_current_trip_start_time(current_trip_list.log_time)
    elevator.set_current_trip_end_time(current_trip_list.end_time)

    return elevator

def can_elevator_reach_destination(elevator, altimeter_array):
    # return [start_altimeter, floor_altimeter, delta_altimeter, direction]
    direction = altimeter_array[-1]
    current_velocity = elevator.get_velocity()

    a = elevator.get_acceleration()
    t = elevator.time_to_reach_maximum_velocity

    if direction:
        delta_altimeter = altimeter_array[-2]

        time_to_decelerate_to_zero = current_velocity / a

        distance_to_decelerate_to_zero = time_to_decelerate_to_zero * a * 0.5
        if distance_to_decelerate_to_zero <= delta_altimeter: # Elevator Can Stop To this floor
            return True

        else: # Elevator Cannot Stop To This Floor
            return False

    else:
        pass

def erase_trip_node_after_now(elevator):
    current_node = elevator.current_trip_node
    next_node = current_node.next

    current_node.next = None
    next_node.prev = None

    del next_node
    gc.collect()

    return elevator

def modify_trip_list(elevator, altimeter_array ,TTR_array, destination_floor):
    millisecond = 1000

    current_trip_list = elevator.get_current_trip_list()
    current_trip_node = elevator.current_trip_node

    start_time = elevator.current_trip_start_time
    elapsed_time = current_trip_node.elapsed_time / millisecond

    start_altimeter = current_trip_list.altimeter_array[0]
    destination_altimeter = altimeter_array[1]

    # Modify current Trip List
    current_trip_list.set_TTR(TTR_array[0] + elapsed_time)
    current_trip_list.altimeter_array = [start_altimeter, destination_altimeter, abs(destination_altimeter - start_altimeter), altimeter_array[-1]]
    current_trip_list.set_destination_floor(destination_floor)
    current_trip_list.set_end_time()

    elevator.set_current_trip_end_time(current_trip_list.end_time)
    elevator.current_altimeter = elevator.current_trip_node.altimeter

    return elevator

def move_single_elevator(elevator, new_timestamp=None):
    entire_trip_list = elevator.entire_trip_list
    print(entire_trip_list)

    elevator.trip_list = TripList()
    elevator.temp_log_timestamp = 0

    for i in range(len(entire_trip_list)-1):
        if new_timestamp is not None:
            index = elevator.entire_trip_list.index(elevator.entire_trip_list_pointer)
            if i == index:
                elevator.temp_log_timestamp = 0
                elevator.first_log_timestamp = new_timestamp

        start_floor = entire_trip_list[i]
        destination_floor = entire_trip_list[i+1]

        altimeter_array = System.get_delta_altimeter_floor_and_floor(start_floor, destination_floor)

        altimeter_difference = altimeter_array[2]

        # STEP 2 : Find out TTR time
        TTR_array = time_to_reach_during_moving(elevator, altimeter_difference)
        TTR = TTR_array[0]

        # STEP 3 : Calculate Each 0.1 second Elevator Altimeter and Floor information
        temp = [start_floor, destination_floor, altimeter_array, TTR]
        elevator.set_current_trip_list(make_trip_list(elevator, temp))

        #print(TTR_array)
        #print(altimeter_array)

        elevator = put_trip_node_data_into_trip_list(elevator, TTR_array, altimeter_array, TTS=3)

    #elevator.trip_list.display()

    return elevator

def check_any_reserved_operation(elevator):
    elevator.set_opcode(10)

    # elevator.entire_trip_list.append(elevator.get_current_trip_list())
    # Find out that any reserved Queue has to be operate
    flag = elevator.reserved_trip_queue.empty()

    return flag

def exit_simulator(System):
    Elevators = System.elevators
    for elevator in Elevators:
        print(elevator)


    TEST_VARIABLES.BP_Loop_Flag = False

def renew_entire_trip_list(elevator, log):
    entire_trip_list = elevator.entire_trip_list
    log_destination_floor = log.data.out_floor

    if len(entire_trip_list) == 0:
        current_stopped_floor_floor = elevator.get_current_stopped_floor()

        entire_trip_list.append(current_stopped_floor_floor)
        entire_trip_list.append(log_destination_floor)

        elevator.entire_trip_list_pointer = entire_trip_list[-1]

        return elevator
    else:
        index = entire_trip_list.index(elevator.entire_trip_list_pointer)
        current_destination_floor = entire_trip_list[index]

        if current_destination_floor < log_destination_floor:
            #print(elevator.current_stopped_altimeter)
            #print(elevator.current_trip_node.altimeter)
            #print(elevator.entire_trip_list_pointer)

            entire_trip_list.append(log_destination_floor)
            #print(elevator.entire_trip_list)

            return elevator

        else:
            start_floor = entire_trip_list[index-1]
            destination_floor = log_destination_floor

            altimeter_array = System.get_delta_altimeter_floor_and_floor(start_floor, destination_floor)
            #print(altimeter_array)

            reachable = can_elevator_reach_destination(elevator, altimeter_array)
            #print(reachable)

            if reachable:
                entire_trip_list.insert(index, destination_floor)
                elevator.entire_trip_list_pointer = entire_trip_list[index]

                prev_start_timestamp = log.prev.data.timestamp
                elevator.first_log_timestamp = prev_start_timestamp

                return elevator
            else:
                insert_element(entire_trip_list, destination_floor, False)
                return elevator

def insert_element(arr, element, flag):
    if flag:  # 내림차순 정렬
        for i in range(len(arr)):
            if arr[i] <= element:
                arr.insert(i, element)
                break
        else:
            arr.append(element)
    else:  # 오름차순 정렬
        for i in range(len(arr)):
            if arr[i] >= element:
                arr.insert(i, element)
                break
        else:
            arr.append(element)


def run(System, a_log):
    # Do we have to Analyze Real Format?
    # If yes then Analyze
    # If no then simulation start based on log
    start_timestamp = a_log.get_first_log_timestamp()

    for elevator in System.elevators:
        elevator.first_log_timestamp = start_timestamp

    log = a_log.get_all_log()
    log_timestamp = log.data.timestamp
    log_is_finished = False

    elevators = System.get_elevators()

    number_of_elevator_finished_operation = 0

    while(TEST_VARIABLES.BP_Loop_Flag):
        if len(elevators) == 0:
            print("No Elevator is Registered")
            TEST_VARIABLES.BP_Loop_Flag = False

        else:
            ### Elevator Status Change Phase ###
            for elevator in elevators:
                opcode = elevator.get_opcode()

                if opcode == 11:
                    if elevator.current_trip_node is None:
                        print("Current Trip Node is None, Check Is there any logs left")
                        if log_is_finished is True:
                            print("All Logs are Finished and Elevator is done Operation")
                            elevator.trip_list.display()

                            elevator.set_opcode(0)
                            number_of_elevator_finished_operation += 1

                        else:
                            print("Elevator is done Operation But Still Log Remains")
                            elevator.set_opcode(10)

                    elif elevator.current_trip_node.next is None:
                        print("Last Frame Before Elevator Stops Operating. Remember This Status")
                        elevator.last_frame_node = elevator.current_trip_node
                        elevator.current_trip_node = elevator.current_trip_node.next

                    else:
                        elevator.current_trip_node = elevator.current_trip_node.next
                        prev = elevator.current_trip_node.prev
                        if prev.altimeter == elevator.current_trip_node.altimeter and elevator.current_stopped_altimeter != elevator.current_trip_node.altimeter:

                            elevator.current_stopped_altimeter = elevator.current_trip_node.altimeter
                            index = elevator.entire_trip_list.index(elevator.entire_trip_list_pointer)
                            print(f"CHANGE INDEX : {index} TO {index+1}")
                            print(elevator.entire_trip_list)

                            if index+1 == len(elevator.entire_trip_list):
                                print("Elevator Has Reached Last Trip Destination Floor")

                            else:
                                elevator.entire_trip_list_pointer = elevator.entire_trip_list[index+1]
                                elevator.set_current_stopped_floor(elevator.entire_trip_list_pointer)

                        pass

            ### Log Check Phase ###
            if start_timestamp == log_timestamp:
                print(f"Time {start_timestamp} log detected")
                # Find Out that log is IN log(TRUE) or OUT log(FALSE)
                inout = log.get_inout()
                if inout: # if log is IN log
                    pass

                else: # if log is OUT log
                    num_of_elevators = len(elevators)

                    if num_of_elevators >= 2:
                        pass

                    elif num_of_elevators == 0:
                        print("No Elevator is Registered")
                        TEST_VARIABLES.BP_Loop_Flag = False

                    else:
                        elevator = elevators[0]
                        elevator = renew_entire_trip_list(elevator, log)

                        new_timestamp = log.data.timestamp if elevator.last_frame_node is not None else None
                        elevator = move_single_elevator(elevator, new_timestamp)

                        if elevator.last_frame_node is not None:
                            print("Found Last Frame Node, elevator re-operating")
                            current_trip_node = elevator.last_frame_node
                            node = elevator.trip_list.head
                            while node is not None:
                                if current_trip_node.current_time == node.current_time:
                                    elevator.current_trip_node = current_trip_node
                                    elevator.last_frame_node = None

                                    break
                                else:
                                    node = node.next

                        else:
                            current_trip_node = elevator.current_trip_node

                        if current_trip_node is None:
                            elevator.set_opcode(10)

                            elevator.current_trip_node = elevator.trip_list.head

                        else:
                            elevator.set_opcode(11)

                            current_trip_node_timestamp = current_trip_node.current_time
                            node = elevator.trip_list.head

                            while node is not None:
                                if current_trip_node_timestamp == node.current_time:
                                    elevator.current_trip_node = node
                                    break

                                else:
                                    node = node.next

                            if elevator.current_trip_node is None:
                                print("FATAL ERROR : CURRENT TRIP NODE IS NONE")

                # Move log to next one
                log = a_log.get_all_log()
                if log is None:
                    log_is_finished = True

                else:
                    log_timestamp = log.data.timestamp

            ### End Phase ###
            if number_of_elevator_finished_operation == len(System.elevators):
                TEST_VARIABLES.BP_Loop_Flag = False

        start_timestamp += datetime.timedelta(seconds=0.1)


if __name__ == '__main__':
    System = System()

    log_path =  r'E:\ML\Elevator Git\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\Resources\Simulator\testlog.txt'
    a_log , i_log, o_log = Simulation.run(log_path)

    run(System, a_log)
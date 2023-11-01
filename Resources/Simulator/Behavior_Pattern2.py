import copy
import datetime
import math
import sys
import json

import Calculation, Simulation
from config import TEST_VARIABLES, TEST_ELEVATOR

second = 1
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
        self.make_altitude_dictionary(self.underground_floors, self.ground_floors, self.alts)
        self.latest_log_timestamp = []
        self.latest_job_done_timestamp = []

    def make_altitude_dictionary(self, underground_floors, ground_floors, alts):
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

    def get_elevator(self, index):
        return self.elevators[index]

    def get_elevator_current_stopped_floor(self, index):
        elevator = self.get_elevator(index)
        elevator_floor = elevator.get_current_stopped_floor()

        return elevator_floor


    def get_delta_altimeter_altimeter_and_floor(self, altimeter, floor):
        if floor < 0:
            delta_index = abs(self.underground_floors) - abs(floor)
            floor_alts = self.alts[delta_index]

        else:
            floor_alts = self.alts[self.underground_floors + floor - 1]

        delta_altimeter = abs(altimeter - floor_alts)

        higher_alts = max(altimeter, floor_alts)
        lower_alts = min(altimeter, floor_alts)

        return [higher_alts, lower_alts, delta_altimeter]

    def get_delta_altimeter_floor_and_floor(self, floor1, floor2):
        higher_floor = max(floor1, floor2)
        lower_floor = floor1 if higher_floor == floor2 else floor2

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

        return [higher_floor_alts, lower_floor_alts, delta_altimeter]


class Elevator:
    def __init__(self):
        self.action = Action()
        self.trip_list = []

        self.latest_trip = Trip()

        self.current_stopped_floor = -5
        self.velocity = 0
        self.maximum_velocity = 2.5
        self.time_to_reach_maximum_velocity = 2.0
        self.acceleration = 1.25
        self.direction = True

        self.closest_floor = {}
        self.current_altitude = None

        self.trip_count = 0

    def add_trip_count(self, value):
        self.trip_count += value

    def get_trip_count(self):
        return self.trip_count

    def get_direction(self):
        return self.direction

    def set_opcode(self, op):
        self.action.opcode = op

    def get_opcode(self):
        return self.action.opcode

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

    def set_trip(self, trip):
        self.latest_trip = trip

class Trip:
    def __init__(self):
        self.head = None

    def append(self, new_node):
        if self.head is None:
            self.head = new_node
        else:
            temp = self.head
            while temp.next:
                temp = temp.next
            temp.next = new_node

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

class Node:
    def __init__(self):
        self.start_time = None
        self.TTR = None
        self.end_time = None

        self.elapsed_time = None

        self.altitude = None

        self.velocity = None
        self.status = None  # 0 = At maximum speed, 1 = is accelerate, 2 = is decelerate
        self.closest_floor = {}

        self.next = None
        self.prev = None

        self.start_floor = None
        self.destination_floor = None

    def set_end_time(self):
        self.end_time = self.start_time + datetime.timedelta(seconds=(self.TTR))

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

    def set_start_time(self, start_time):
        self.start_time = start_time
    def set_TTR(self, TTR):
        self.TTR = TTR
    def set_elapsed_time(self, elapsed_time):
        self.elapsed_time = elapsed_time
    def set_velocity(self, velocity):
        self.velocity = velocity
    def set_altitude(self, altitude):
        self.altitude = altitude
    def set_closest_floor(self, closest_floor):
        self.closest_floor = closest_floor.copy()

    def get_altitude(self):
        return self.altitude

    def get_velocity(self):
        return self.velocity

    def get_status(self):
        return self.status

    def get_closet_upper_floor(self):
        str_floor =  self.closest_floor['upper_floor']
        if str_floor[0] == 'B':
            floor = int(str_floor[1:]) * -1
        else:
            floor = int(str_floor)

        return floor

    def display(self):
        print(f"Opertaion Start Time : {self.start_time}\nTTR : {self.TTR}\nTime Elasped: {self.elapsed_time/1000} seconds\nStart Floor : {self.start_floor}\nDestination Floor : {self.destination_floor}\nAltitude: {self.altitude} meters\nClosest Floor: {self.closest_floor}\n")

class Action:
    def __init__(self):
        self.opcode = 0

def code10(a_log, System, data, floor):
    if System.get_number_of_elevators() >= 2:
        pass
    else:
        elevator = System.get_elevator(0)
        if elevator.get_opcode() == 0 or elevator.get_opcode() == 10:

            elevator_floor = System.get_elevator_current_stopped_floor(0)
            altimeter_array = System.get_delta_altimeter_floor_and_floor(elevator_floor, floor)
            move_elevator(elevator, altimeter_array, elevator_floor, floor)

        elif elevator.get_opcode() == 11:
            erase_trip_node(elevator, System.get_latest_log_timestamp())
            head = elevator.trip_list[-1].head
            closest_upper = head.get_closet_upper_floor()
            last_destination_floor = head.get_destination_floor()
            # print(closest_upper)
            # print(last_destination_floor)
            # print(floor)

            if last_destination_floor < floor:  # Make New Trip After this Trip finishes
                current_out_log = copy.deepcopy(a_log.pointer_out)
                current_out_log = current_out_log.next

                end_time = head.get_end_time()
                #print("Analyze Logs")
                while current_out_log is not None:
                    data = current_out_log.data
                    current_out_log_timestamp = data.get_timestamp()
                    if current_out_log_timestamp <= end_time:
                        pass
                        # data.display()
                    # print(current_out_log.data.display())
                    # print(end_time)
                    # print(current_out_log_timestamp)

                    current_out_log = current_out_log.next

                waiting_time = datetime.timedelta(seconds=(3))
                new_start_time = end_time + waiting_time
                System.set_job_done_timestamp(new_start_time)
                elevator.set_opcode(10)

                System.set_latest_log_timestamp(System.get_job_done_timestamp())
                elevator_floor = System.get_elevator_current_stopped_floor(0)
                altimeter_array = System.get_delta_altimeter_floor_and_floor(elevator_floor, floor)
                move_elevator(elevator, altimeter_array, elevator_floor, floor)
                elevator.set_current_stopped_floor(floor)

                pass

            elif floor < last_destination_floor:
                #print(System.latest_log_timestamp)
                System.latest_log_timestamp.pop()

                elevator_floor = System.get_elevator_current_stopped_floor(0)
                altimeter_array = System.get_delta_altimeter_floor_and_floor(elevator_floor, floor)
                move_elevator(elevator, altimeter_array, elevator_floor, floor)

                elevator.set_current_stopped_floor(floor)
                #print(System.latest_log_timestamp)

                System.set_latest_log_timestamp(System.get_job_done_timestamp())
                #System.latest_log_timestamp = elevator.trip_list[-1].head.end_time
                elevator_floor = System.get_elevator_current_stopped_floor(0)
                altimeter_array = System.get_delta_altimeter_floor_and_floor(elevator_floor, last_destination_floor)
                move_elevator(elevator, altimeter_array, elevator_floor, last_destination_floor)

                elevator.set_current_stopped_floor(last_destination_floor)
                #print(System.latest_log_timestamp)

            pass

def logic(elevator, elevator_floor, floor, altimeter_array):
    trip_temp = Trip()

    elevator.set_opcode(11)

    higher_altitude = altimeter_array[0]
    lower_altitude = altimeter_array[1]
    altitude_difference = altimeter_array[2]
    current_altitude = lower_altitude

    #print(altimeter_array)

    acceleration = elevator.get_acceleration()
    t = elevator.time_to_reach_maximum_velocity
    TTR, flag = time_to_reach(elevator, altitude_difference)  # Time formula from physics

    TTR_to_microsecond = round(int(TTR * 1000),-2)

    current_velocity = 0

    if flag is True:
        for elapsed_time in range(0, TTR_to_microsecond+100, 100):  # Loop through each 0.1 second
            if elapsed_time == 0:
                continue

            if current_altitude >= higher_altitude:
                print(elapsed_time)
                print(current_velocity)

            elif elapsed_time <= t * 1000:  # Acceleration phase
                before = current_velocity
                current_velocity += acceleration * 0.1
                after = current_velocity

                current_altitude += ((before + after) * 0.1 * 0.5)

            elif elapsed_time >= t * 1000 and elapsed_time <= (TTR_to_microsecond - t * 1000):  # Constant speed phase
                elevator.set_velocity((elevator.maximum_velocity))  # Constant speed of 2.5 m/s

                current_altitude += (elevator.get_velocity() * 0.1)


            elif elapsed_time > (TTR_to_microsecond - t * 1000):  # Deceleration phase
                before = current_velocity
                current_velocity += (acceleration * -0.1)
                after = current_velocity

                current_altitude += ((before + after) * 0.1 * 0.5)

            closest_floors = find_closest_floors(System.alts_dict, current_altitude)

            temp_node = Node()

            temp_node.set_TTR(TTR)
            temp_node.set_start_time(System.get_latest_log_timestamp())
            temp_node.set_end_time()
            temp_node.set_elapsed_time(elapsed_time)

            temp_node.set_start_floor(elevator_floor)
            temp_node.set_destination_floor(floor)

            temp_node.set_velocity(elevator.get_velocity())
            temp_node.set_altitude(current_altitude)
            temp_node.set_closest_floor(closest_floors)

            trip_temp.append(temp_node)

        System.set_job_done_timestamp(temp_node.get_end_time())
        return trip_temp

    else:
        for elapsed_time in range(0, TTR_to_microsecond, 100):  # Loop through each 0.1 second
            if elapsed_time == 0:
                continue

            elif elapsed_time <= TTR_to_microsecond/2:  # Acceleration phase
                before = current_velocity
                current_velocity += acceleration * 0.1
                after = current_velocity

                current_altitude += ((before + after) * 0.1 * 0.5)

            elif elapsed_time > TTR_to_microsecond/2:
                before = current_velocity
                current_velocity += (acceleration * -0.1)
                after = current_velocity

                current_altitude += ((before + after) * 0.1 * 0.5)

            if elapsed_time == (TTR_to_microsecond-100):
                current_altitude = lower_altitude + altitude_difference

            closest_floors = find_closest_floors(System.alts_dict, current_altitude)

            temp_node = Node()
            temp_node.set_TTR(TTR)
            temp_node.set_start_time(System.get_latest_log_timestamp())
            temp_node.set_end_time()
            temp_node.set_elapsed_time(elapsed_time)

            temp_node.set_start_floor(elevator_floor)
            temp_node.set_destination_floor(floor)

            temp_node.set_velocity(elevator.get_velocity())
            temp_node.set_altitude(current_altitude)
            temp_node.set_closest_floor(closest_floors)

            trip_temp.append(temp_node)

        System.set_job_done_timestamp(temp_node.get_end_time())
        return trip_temp


def move_elevator(elevator, altimeter_array, elevator_floor, floor):
    if elevator.get_direction() is True:
        if elevator.get_opcode() == 0 or elevator.get_opcode() == 10:
            trip_temp = logic(elevator, elevator_floor, floor, altimeter_array)
            elevator.set_trip(copy.deepcopy(trip_temp))
            elevator.trip_list.append(elevator.latest_trip)

        elif elevator.get_opcode() == 11:
            trip_temp = logic(elevator, elevator_floor, floor, altimeter_array)
            elevator.set_trip(copy.deepcopy(trip_temp))
            elevator.trip_list.append(elevator.latest_trip)

    elevator.add_trip_count(1)

def erase_trip_node(elevator, System_latest_timestamp):
    current_node = elevator.trip_list[-1].head

    while current_node is not None:
        elapsed_time = current_node.elapsed_time
        current_node_time = current_node.start_time + datetime.timedelta(seconds=(elapsed_time/1000))

        if current_node.start_time <= current_node_time <= System_latest_timestamp:
            elevator.latest_trip.remove(current_node)

        elif System_latest_timestamp < current_node_time:
            break

        current_node = elevator.latest_trip.head

def time_to_reach(elevator, altitude_difference):
    a = elevator.acceleration
    t = elevator.time_to_reach_maximum_velocity
    at = a * t
    minimum_acceleration_distance = 2 * at

    if altitude_difference <= minimum_acceleration_distance:
        TTR = math.sqrt(altitude_difference * 4 /a)

        return [TTR, False]
    else:
        TTR = (altitude_difference/at) + t

        return [TTR, True]

def find_closest_floors(alts_dict, current_altitude):
    closest_floors = {
        'upper_floor': None,
        'lower_floor': None
    }
    closest_upper_diff = float('inf')
    closest_lower_diff = float('inf')

    for floor, altitude in alts_dict.items():
        diff = current_altitude - altitude

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

def run(System, a_log):
    latest_out_log = a_log.get_out_log()

    while latest_out_log is not None:
        data = analyze_out_log(latest_out_log)
        System.latest_log_timestamp.append(data[2])

        code10(a_log, System, data, data[1])

        elevator = System.elevators[0]

        #elevator.latest_trip.display()
        elevator.latest_trip = Trip()

        latest_out_log = a_log.get_out_log()

    for trip in elevator.trip_list:
        head = trip.head
        while head is not None:
            head.display()
            head = head.next

if __name__ == '__main__':
    System = System()

    log_path =  r'E:\ML\Elevator Git\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\Resources\Simulator\testlog.txt'
    a_log , i_log, o_log = Simulation.run(log_path)

    run(System, a_log)
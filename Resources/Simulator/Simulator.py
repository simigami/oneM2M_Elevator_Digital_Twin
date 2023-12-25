import datetime
import os.path
import csv

import queue
import sys
import json

import Logic.Single_Elevator as Single_Elevator
import Calculation, Simulation
from config import TEST_VARIABLES, TEST_ELEVATOR


one_tenth_second = 1
second = 10
minute = 60 * second
hour = 60 * minute
global csv_number
global simulation_timestamp

# OPCODE
# 0 = Initial Value
# 10 = Elevator finishes its trip and turns to IDLE + STANDBY
# 20 = Elevator Stops on Floor but not IDLE
# 21 = Elevator on Acceleration
# 22 = Elevator on Max Speed
# 23 = Elevator on Deceleration
# 24 = Elevator Move on Temp Floor due to No button detected in elevator inside
# 100 = Elevator Emergency Halt

class System:
    def __init__(self):
        self.elevators = [] # List of Elevator Class
        self.now = None
        self.simulation_start_time = None

        self.underground_floors = TEST_VARIABLES.underground_floors
        self.ground_floors = TEST_VARIABLES.ground_floors

        self.building_alts = TEST_VARIABLES.alts
        self.alts_dict = {}

        self.TTS = 3

        self.make_altimeter_dictionary(self.underground_floors, self.ground_floors, self.building_alts)

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

    def add_elevator(self, elevator):
        self.elevators.append(elevator)

    def get_elevators(self):
        return self.elevators

    def print_all_elevator_trip(self):
        for elevator in self.elevators:
            print(f"Trip for Elevator {elevator}")
            for each_trip in elevator.done_trip_list:
                print(f"Start Floor = {each_trip.start_floor} -> End Floor = {each_trip.destination_floor}\n")

    def get_closest_floor_from_this_altimeter(self, altimeter, direction):
        closest_floor = None
        closest_altitude_diff = float('inf')

        for floor, floor_altimeter in self.alts_dict.items():
            altitude_diff = abs(floor_altimeter - altimeter)
            if direction and altimeter > floor_altimeter:
                # Going up, so skip floors below the altimeter
                continue

            if not direction and altimeter < floor_altimeter:
                # Going down, so skip floors above the altimeter
                continue

            if altitude_diff < closest_altitude_diff:
                closest_floor = floor
                break

        return closest_floor

    def get_delta_altimeter_floor_and_floor(self, start, dst):
        if start < dst:
            direction = True # True = Up, False = Down
        else:
            direction = False

        higher_floor = max(start, dst)
        lower_floor = start if higher_floor == dst else dst

        if higher_floor < 0:
            delta_index = abs(self.underground_floors) - abs(higher_floor)
            higher_floor_alts = self.building_alts[delta_index]

        else:
            higher_floor_alts = self.building_alts[self.underground_floors + higher_floor - 1]

        if lower_floor < 0:
            delta_index = abs(self.underground_floors) - abs(lower_floor)
            lower_floor_alts = self.building_alts[delta_index]

        else:
            lower_floor_alts = self.building_alts[self.underground_floors + lower_floor - 1]

        delta_altimeter = abs(higher_floor_alts - lower_floor_alts)

        if direction:
            return [lower_floor_alts, higher_floor_alts, delta_altimeter, direction]

        else:
            return [higher_floor_alts, lower_floor_alts, delta_altimeter, direction]

    def display(self):
        text = f"Time is {self.now}\n"

        return text

class Elevator:
    def __init__(self):
        self.opcode = 0

        self.full_trip_list = None
        self.current_trip_list = None
        self.done_trip_list = []

        self.current_trip_node = None
        self.trip_node_right_before_IDLE = None

        self.current_stopped_floor = -5
        self.current_stopped_altimeter = -55

        # self.current_stopped_floor = 12
        # self.current_stopped_altimeter = 1

        self.current_velocity = 0
        self.maximum_velocity = 2.5
        self.time_to_reach_maximum_velocity = 2.0
        self.acceleration = 1.25
        self.direction = True

        self.current_trip_start_time = None
        self.current_trip_end_time = None

        self.trip_count = 0

    def set_opcode(self, opcode):
        self.opcode = opcode

    def get_opcode(self):
        return self.opcode

    def set_direction(self, direction):
        self.direction = direction

    def get_direction(self):
        return self.direction

    def set_current_velocity(self, velocity):
        self.current_velocity = velocity

    def get_current_velocity(self):
        return self.current_velocity

    def get_time_to_reach_maximum_velocity(self):
        return self.time_to_reach_maximum_velocity

    def get_acceleration(self):
        return self.acceleration

    def get_maximum_velocity(self):
        return self.maximum_velocity

    def set_current_trip_node(self, node):
        self.current_trip_node = node

    def get_current_trip_node(self):
        return self.current_trip_node

    def set_trip_node_right_before_IDLE(self, node):
        self.trip_node_right_before_IDLE = node

    def get_trip_node_right_before_IDLE(self):
        return self.trip_node_right_before_IDLE

    def set_current_stopped_floor(self, floor):
        self.current_stopped_floor = floor

    def get_current_stopped_floor(self):
        return self.current_stopped_floor

    def set_current_stopped_altimeter(self, altimeter):
        self.current_stopped_altimeter = altimeter

    def get_current_stopped_altimeter(self):
        return self.current_stopped_altimeter

    def display(self):
        node = self.current_trip_node
        text = f"This Node Time : {node.current_time}\nTime Elapsed: {node.elapsed_time / 1000} seconds\nVelocity : {node.velocity}m/s\nAltimeter: {node.altimeter} meters\nClosest Floor: {node.closest_floor}\n\n"
        return text

def write_to_csv(Elevator, file_path):
    global simulation_timestamp
    global csv_number

    file_name = 'result.csv'
    mode = 'a' if os.path.exists(file_path) else 'w'
    current_trip_node = Elevator.current_trip_node
    clf = current_trip_node.closest_floor['lower_floor']
    cuf = current_trip_node.closest_floor['upper_floor']

    file_path += '/' + file_name

    if not os.path.isfile(file_path):
        with open(file_path, 'w', newline='') as csv_file:
            fwriter = csv.writer(csv_file)

            csv_header = ['Number', 'Current Time', 'Trip Time', 'Velocity', 'Altimeter', 'Closest Upper Floor', 'Closest Lower Floor']
            #csv_header = ['Current Time', 'Trip Time', 'Velocity', 'Altimeter', 'Closest Upper Floor', 'Closest Lower Floor']
            fwriter.writerow(csv_header)

            csv_payload = [csv_number, simulation_timestamp, current_trip_node.current_time,
                           current_trip_node.velocity, current_trip_node.altimeter, cuf, clf]
            fwriter.writerow(csv_payload)

    else:
        with open(file_path, 'a', newline='') as csv_file:
            fwriter = csv.writer(csv_file)

            csv_payload = [csv_number, simulation_timestamp, current_trip_node.current_time,
                           current_trip_node.velocity, current_trip_node.altimeter, cuf, clf]
            # csv_payload = [current_trip_node.current_time, current_trip_node.elapsed_time / 1000,
            #                current_trip_node.velocity, current_trip_node.altimeter, cuf, clf]

            fwriter.writerow(csv_payload)

    csv_number += 1

def write_to_file(Elevator_System, Elevator, file_path):
    file_name = 'result.txt'
    mode = 'a' if os.path.exists(file_path) else 'w'

    file_path += '/' + file_name

    f = open(file_path, mode)

    current_time_text = Elevator_System.display()
    node_info_text = Elevator.display()

    f.write(current_time_text)
    f.write(node_info_text)

def write_text_to_file(text, file_path):
    file_name = 'result.txt'
    mode = 'a' if os.path.exists(file_path) else 'w'

    file_path += '/' + file_name

    f = open(file_path, mode)

def ending(Elevator_System):
    # END STEP 1. CHECK EVERY CURRENT TRIP LIST TO DONE TRIP LIST
    for elevator in Elevator_System.elevators:
        if elevator.full_trip_list.reachable_head is not None:
            elevator.done_trip_list.append(elevator.full_trip_list.reachable_head)
            elevator.full_trip_list.reachable_head = None

    # END STEP 2. PRINT EVERY DONE TRIP LIST OF ELEVATORS
    Elevator_System.print_all_elevator_trip()
def run():
    # PHASE 0 Get Actual Log from real log + Set Flags, Elevators, Variables
    Elevator_System = System()

    result_path = r'E:\ML\Elevator Git\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\Resources\Simulator'
    log_path = r'E:\ML\Elevator Git\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\Resources\Simulator\testlog.txt'
    
    # result_path = r'/Users/yummyshrimp/Desktop/Elevator/Resources/Simulator'
    # log_path = r'/Users/yummyshrimp/Desktop/Elevator/Resources/Simulator/testlog.txt'
    
    a_log, i_log, o_log = Simulation.run(log_path)
    log_instance = None # Pointer For Log Detect

    elevator_test = Elevator()
    Elevator_System.add_elevator(elevator_test)
    number_of_elevator_code10 = 0

    elevators = Elevator_System.get_elevators()

    global simulation_timestamp
    simulation_timestamp = a_log.head.data.timestamp

    # Set Flags
    flag_force_end_time = simulation_timestamp + datetime.timedelta(minutes=30)
    flag_elevator_IDLE = [False * len(elevators)]
    flag_elevator_time_to_get_out_IDLE = [0 * len(elevators)]
    flag_end_loop = True # False = Loop End
    flag_analyze_log = False # True = Analyze Log Start
    flag_log_analysis_complete = True
    flag_end_log = False # True = Start End Phase
    flag_record_start_time = True

    global csv_number
    csv_number = 1

    # Init Settings
    for elevator in elevators:
        elevator.full_trip_list = Single_Elevator.Full_Trip_List()

    # While Loop Begin
    while(flag_end_loop):
        if flag_record_start_time:
            Elevator_System.simulation_start_time = simulation_timestamp
            flag_record_start_time = False

        Elevator_System.now = simulation_timestamp
    # PHASE 1 : Change Elevator Status Per 0.1 Second
        for index, elevator in enumerate(elevators):
            current_trip_node = elevator.get_current_trip_node()

            if current_trip_node is not None:
                next = current_trip_node.next
                if next is not None:
                    # Compare Velocity and Set opcode
                    current_velocity = current_trip_node.get_velocity()
                    next_node_velocity = next.get_velocity()

                    current_altimeter = current_trip_node.get_altimeter()
                    next_node_altimeter = next.get_altimeter()

                    if current_velocity < next_node_velocity:
                        elevator.set_opcode(21)
                    elif current_velocity > next_node_velocity:
                        elevator.set_opcode(22)
                    elif current_velocity == next_node_velocity:
                        elevator.set_opcode(23)

                    if current_altimeter < next_node_altimeter:
                        elevator.set_direction(True)
                    else:
                        elevator.set_direction(False)

                    elevator.set_current_velocity(current_velocity)
                    # Change Current Trip Node
                    write_to_file(Elevator_System, elevator, result_path)
                    write_to_csv(elevator, result_path)
                    # Elevator_System.display()
                    # elevator.display()

                    elevator.set_current_trip_node(next)

                # If current Trip Node is Last Before IDLE, velocity = 0
                elif next is None:
                    write_to_file(Elevator_System, elevator, result_path)
                    write_to_csv(elevator, result_path)
                    # Elevator_System.display()
                    # elevator.display()

                    current_velocity = current_trip_node.get_velocity()
                    elevator.set_current_velocity(current_velocity)

                    if current_velocity == 0:
                        elevator.set_opcode(20)

                        current_trip_list = elevator.current_trip_list
                        if current_trip_list.next is not None:
                            full_trip_list = elevator.full_trip_list
                            full_trip_list.remove_trip_list(current_trip_list) # Remove Current Trip List To next One

                            elevator.current_trip_list = None
                            elevator.done_trip_list.append(current_trip_list)

                            elevator.set_trip_node_right_before_IDLE(current_trip_node)
                            elevator.set_current_trip_node(next)

                        else:   # if this trip list is last, and also node is last
                            full_trip_list = elevator.full_trip_list
                            full_trip_list.remove_trip_list(current_trip_list)  # Remove Current Trip List To next One

                            elevator.current_trip_list = None
                            if elevator.full_trip_list.reachable_head is not None:
                                elevator.full_trip_list.reachable_head = elevator.full_trip_list.reachable_head.next

                            elevator.done_trip_list.append(current_trip_list)

                            elevator.set_trip_node_right_before_IDLE(current_trip_node)
                            elevator.set_current_trip_node(next)

                            pass
                    elevator.set_current_trip_node(None)

            # If current Trip Node is None
            else:
                # Get trip_node_right_before_IDLE
                trip_node_right_before_IDLE = elevator.get_trip_node_right_before_IDLE()

                if trip_node_right_before_IDLE is not None:
                    # Set Elevator to IDLE and wait for next log or operation
                    elevator.set_opcode(20)
                    TTS = 3
                    flag_elevator_time_to_get_out_IDLE[index] = Elevator_System.now + datetime.timedelta(seconds=TTS)

                    current_stopped_floor = trip_node_right_before_IDLE.closest_floor['upper_floor']

                    if current_stopped_floor[0] == 'B':
                        int_floor = int(current_stopped_floor[1:]) * -1
                    else:
                        int_floor = int(current_stopped_floor)

                    elevator.set_current_stopped_floor(int_floor)
                    current_stopped_altimeter = Elevator_System.alts_dict[current_stopped_floor]

                    elevator.set_current_stopped_altimeter(current_stopped_altimeter)

                    elevator.set_trip_node_right_before_IDLE(None)

                elif trip_node_right_before_IDLE is None:
                    if flag_elevator_time_to_get_out_IDLE[index] != 0:

                        if simulation_timestamp == flag_elevator_time_to_get_out_IDLE[index]:
                            # Check if there is any Operation Remaining
                            full_trip_list = elevator.full_trip_list
                            reachable_head = full_trip_list.reachable_head

                            if reachable_head is None:
                                # Check if there is any unreachable domains left
                                num_up = full_trip_list.get_number_of_trip_list(1)
                                num_down = full_trip_list.get_number_of_trip_list(2)

                                if num_up == num_down == 0: # There is No more Trip List Remaining
                                    pass
                                elif num_up >= num_down: # Up Direction Should Work First(정의)
                                    unreachable_up = full_trip_list.unreachable_up_head
                                    unreachable_up.start_floor = elevator.get_current_stopped_floor()
                                    unreachable_up.direction = True if unreachable_up.start_floor < unreachable_up.destination_floor else False
                                    full_trip_list.move_unreachable_head_to_reachable_head(1)

                                else: # Down Direction Should Work First
                                    unreachable_down = full_trip_list.unreachable_up_head
                                    unreachable_down.start_floor = elevator.get_current_stopped_floor()
                                    unreachable_down.direction = True if unreachable_down.start_floor < unreachable_down.destination_floor else False
                                    full_trip_list.move_unreachable_head_to_reachable_head(2)

                            elevator.current_trip_list = full_trip_list.reachable_head

                            if elevator.current_trip_list is not None:  # Start to Move Again
                                Single_Elevator.run(Elevator_System, elevator, None)

                            elif elevator.current_trip_list is None:  # There is no operation remaining, so change opcode to 10
                                elevator.set_opcode(10)
                                flag_elevator_IDLE[index] = True

                            flag_elevator_time_to_get_out_IDLE[index] = 0

                        else:
                            full_trip_list = elevator.full_trip_list
                            reachable_head = full_trip_list.reachable_head
                            if reachable_head is None:
                                # Check if there is any unreachable domains left
                                num_up = full_trip_list.get_number_of_trip_list(1)
                                num_down = full_trip_list.get_number_of_trip_list(2)

                                if num_up == num_down == 0: # There is No more Trip List Remaining
                                    pass
                                elif num_up >= num_down: # Up Direction Should Work First(정의)
                                    unreachable_up = full_trip_list.unreachable_up_head
                                    unreachable_up.start_floor = elevator.get_current_stopped_floor()
                                    unreachable_up.direction = True if unreachable_up.start_floor < unreachable_up.destination_floor else False
                                    full_trip_list.move_unreachable_head_to_reachable_head(1)

                                else: # Down Direction Should Work First
                                    unreachable_down = full_trip_list.unreachable_down_head
                                    unreachable_down.start_floor = elevator.get_current_stopped_floor()
                                    unreachable_down.direction = True if unreachable_down.start_floor < unreachable_down.destination_floor else False
                                    full_trip_list.move_unreachable_head_to_reachable_head(2)

                            else: # If reachable head is not None
                                pass
                                # # Check if TTR is None
                                # TTR = reachable_head.TTR
                                # if len(TTR) == 0:
                                #     Single_Elevator.run(Elevator_System, elevator, log_instance)
                                # else:
                                #     pass

                    else:
                        #Search if there is any trip list is newly opened
                        reachable_head = elevator.full_trip_list.reachable_head
                        if reachable_head is not None:
                            elevator.current_trip_list = reachable_head
                            elevator.set_opcode(20)
                            Single_Elevator.run(Elevator_System, elevator, None)

                        else:
                            pass

    # PHASE 2 : LOG DETECT

        # If log is live mode
        if a_log.head is None:
            print("There is no pre-loaded Log, Simulator Turns To Live mode")
            pass

        # If log is already made
        else:
            if log_instance is None:
                log_instance = a_log.head
                flag_log_analysis_complete = False

            else:

                if flag_log_analysis_complete:
                    next_log = log_instance.next

                    if next_log is None: # If log_instance is Last Log
                        flag_end_log = True

                    else:
                        log_instance = next_log
                        flag_log_analysis_complete = False

        log_timestamp = log_instance.data.timestamp
        if log_timestamp == simulation_timestamp:

            flag_analyze_log = True

        else:
            flag_analyze_log = False


        #flag_end_loop = False

    # PHASE 3: LOG Analyze
        if flag_analyze_log:
            log_instance_data = log_instance.data
            inout = log_instance_data.inout

            if log_instance.data.inout is True and len(log_instance.data.in_current_buttons) == 0:
                #print("ASDASDSAD0")
                pass

            if log_instance.data.inout is True and len(log_instance.data.in_current_buttons) == 1:
                #print("ASDASDSAD1")
                pass

            if log_instance.data.inout is True and len(log_instance.data.in_current_buttons) == 2:
                #print("ASDASDSAD2")
                pass

            if log_instance.data.inout is False and log_instance.data.out_floor == 1:
                #print("ASDASDSAD3")
                pass

            if inout: # If Log is IN Log
                resource = log_instance_data.get_resources()

                previous_button_list = resource[0]
                current_button_list = resource[1]
                current_floor = resource[2]
                elevator_number = resource[3]
                timestamp = resource[4]

                number_of_elevators = len(elevators)
                if number_of_elevators >= 2:
                    pass
                elif number_of_elevators == 0:
                    print("Error Occured, number of elevators should be larger than 0")
                else:   # Single Elevator Moving Algorithms
                    # Check if elevator is currently moving?
                    opcode = elevators[0].get_opcode()
                    if opcode == 10 or opcode == 20: # If Elevator is not IDLE but Stopped at floor
                        Single_Elevator.run(Elevator_System, elevator, log_instance)

                    elif opcode == 21 or opcode == 22 or opcode == 23: # If Elevator is IDLE
                        Single_Elevator.run(Elevator_System, elevator, log_instance)

                # Check 
                #print(previous_button_list)
                #print(current_button_list)

                pass

            else: # If Log is OUT Log
                number_of_elevators = len(elevators)
                if number_of_elevators >= 2:
                    pass
                elif number_of_elevators == 0:
                    print("Error Occured, number of elevators should be larger than 0")
                else:   # Single Elevator Moving Algorithms
                    elevator = elevators[0]

                    Single_Elevator.run(Elevator_System, elevator, log_instance)

            flag_analyze_log = False
            flag_log_analysis_complete = True

    # PHASE 4 END PHASE
        if flag_end_log:
            # Check if any other elevator is still operating
            for elevator in elevators:
                opcode = elevator.get_opcode()
                if opcode == 10:
                    number_of_elevator_code10 += 1

            if number_of_elevator_code10 == len(elevators):
                text = "Log Instance Has Meet Last Log\n Every Elevator Finished Operation and No Log Remains. Turning Off Simulator"
                write_text_to_file(text, result_path)
                # print("Log Instance Has Meet Last Log")
                # print("Every Elevator Finished Operation and No Log Remains. Turning Off Simulator")
                flag_end_loop = False
                flag_log_analysis_complete = False

        if simulation_timestamp == flag_force_end_time:
            flag_end_loop = False

    # PHASE 5 DETOR PHASE
        if not flag_end_loop:
            ending(Elevator_System)

        simulation_timestamp += datetime.timedelta(seconds=0.1)

if __name__ == '__main__':
    run()
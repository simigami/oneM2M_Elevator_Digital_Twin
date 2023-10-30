import simpy
import datetime

import Behavior_Pattern

import simpy
import datetime

import Behavior_Pattern

class Building(object):
    def __init__(self, env, arr, number_of_elevators):
        self.env = env
        self.arr = arr
        self.proc = env.process(self.get_behavior_pattern(arr, env))
        self.elevator = simpy.Resource(env, capacity=number_of_elevators)
        self.LL = Behavior_Pattern.init()
        self.pattern = Behavior_Pattern.patt()

    def get_behavior_pattern(self, arr, env):
        for elem in arr:
            now = datetime.datetime.now()

            print("Start Analysing at {}".format(env.now))
            #print(elem)
            self.LL = Behavior_Pattern.parse_log_array_to_node(elem, self.LL, self.pattern)

            end = datetime.datetime.now()

            i_now = int(now.timestamp())
            i_end = int(end.timestamp())
            # print(i_now)
            # print(i_end)
            delta = i_end - i_now

            yield env.timeout(delta)
            print("End Analysing at {}".format(env.now))

        yield env.process(self.print_LL())

    def print_LL(self):
        self.LL.print_all_node()
        yield env.timeout(1)


if __name__ == '__main__':
    noe = 1
    env = simpy.Environment()
    path = rf"E:\ML\Elevator Git\Elevator_Results\temp.txt"
    arr = Behavior_Pattern.parse_log_to_log_array(path)

    cls = Building(env, arr, noe)
    env.run()
    #LL.print_all_node()
#
# class Node:
#     def __init__(self, data):
#         self.data = data
#         self.next = None
#
# class LinkedList:
#     def __init__(self):
#         self.head = None
#
#     def append(self, data):
#         new_node = Node(data)
#         if self.head is None:
#             self.head = new_node
#         else:
#             temp = self.head
#             while temp.next:
#                 temp = temp.next
#             temp.next = new_node
#
#     def display(self):
#         temp = self.head
#         while temp:
#             if temp.data is not None:
#                 temp.data.display()
#
#             temp = temp.next
#
# class ElevatorLog:
#     def __init__(self, inout, timestamp):
#         self.inout = True if inout == "In" else False
#         self.timestamp = datetime.datetime.strptime(timestamp, '%Y_%m%d_%H%M%S')
#
# class OutElevatorLog(ElevatorLog):
#     def __init__(self, inout, timestamp, floor, number, direction):
#         super().__init__(inout, timestamp)
#         self.action = "Somebody Called Elevator"
#         self.out_floor = self.convert_floor_to_number(floor)
#         self.out_number = int(number)
#         self.out_direction = True if direction == "Up" else False
#
#     def convert_floor_to_number(self, floor):
#         if floor.startswith('B'):
#             return -int(floor[1:])
#         else:
#             return int(floor)
#
#     def display(self):
#         print(f"Inout ({type(self.inout)}): {'In' if self.inout else 'Out'}")
#         print(f"Timestamp ({type(self.timestamp)}): {self.timestamp}")
#         print(f"Action ({type(self.action)}): {self.action}")
#         print(f"Out Floor ({type(self.out_floor)}): {self.out_floor}")
#         print(f"Out Number ({type(self.out_number)}): {self.out_number}")
#         print(f"Out Direction ({type(self.out_direction)}): {'Up' if self.out_direction else 'Down'}\n")
#
# class InElevatorLog(ElevatorLog):
#     def __init__(self, inout, timestamp, action, prev_button, curr_button, curr_floor, elevator_number):
#         super().__init__(inout, timestamp)
#         self.action = action
#         self.in_previous_buttons = self.convert_buttons_to_int(prev_button)
#         self.in_current_buttons = self.convert_buttons_to_int(curr_button)
#         self.in_current_floor = curr_floor.split(" between ")
#         self.in_elevator_number = int(elevator_number)
#
#     def convert_floor_to_number(self, floor):
#         if floor.startswith('B'):
#             return -int(floor[1:])
#         else:
#             return int(floor)
#     def convert_buttons_to_int(self, button_str):
#         buttons = button_str.strip("[]'").split("', '")
#         array = []
#
#         for button in buttons:
#             if button != '':
#                 if button.isnumeric():
#                     array.append(int(button))
#                 else:
#                     array.append(self.convert_floor_to_number(button))
#
#         return array
#
#     def display(self):
#         print(f"Inout ({type(self.inout)}): {'In' if self.inout else 'Out'}")
#         print(f"Timestamp ({type(self.timestamp)}): {self.timestamp}")
#         print(f"Action ({type(self.action)}): {self.action}")
#         print(f"Previous Button Panel ({type(self.in_previous_buttons)}): {self.in_previous_buttons}")
#         print(f"Current Button Panel ({type(self.in_current_buttons)}): {self.in_current_buttons}")
#         print(f"Current Floor ({type(self.in_current_floor)}): {' between '.join(self.in_current_floor)}")
#         print(f"Elevator Number ({type(self.in_elevator_number)}): {self.in_elevator_number}\n")
#
# # Usage example:
# with open(r'E:\ML\Elevator Git\Elevator_Results\temp3.txt', 'r') as file:
#     log_lines = file.read().split('\n\n')
#
# in_log_list = LinkedList()
# out_log_list = LinkedList()
# all_log_list = LinkedList()
#
# for log_text in log_lines:
#     #print(log_text)
#
#     log_parts = log_text.split('\n')
#     inout = log_parts[0].split(': ')[1]
#     timestamp = log_parts[1].split(': ')[1]
#     action = log_parts[2]
#
#     if inout == 'In':
#         prev_button = log_parts[3].split(': ')[1]
#         curr_button = log_parts[4].split(': ')[1]
#         curr_floor = log_parts[5].split(': ')[1]
#         elevator_number = log_parts[6].split(': ')[1]
#
#         temp = InElevatorLog(inout, timestamp, action, prev_button, curr_button, curr_floor, elevator_number)
#         in_log_list.append(temp)
#
#     else:
#         floor = log_parts[3].split(': ')[1]
#         number = log_parts[4].split(': ')[1]
#         direction = log_parts[5].split(': ')[1]
#
#         temp = OutElevatorLog(inout, timestamp, floor, number, direction)
#         out_log_list.append(temp)
#
#     all_log_list.append(temp)
#
# # print("All Logs:")
# # all_log_list.display()
#
# # Display InElevatorLogs
# print("\nInElevator Logs:")
# in_log_list.display()
#
# # Display OutElevatorLogs
# print("\nOutElevator Logs:")
# out_log_list.display()
# This Function Will Return a Trip_List(A List of Nodes) of Start Floor and Destination Floor
import datetime

# class TripList:
#     def __init__(self):
#         self.head = None
#         self.log_time = None
#
#         self.TTR = None
#         self.end_time = None
#
#         self.start_floor = None
#         self.destination_floor = None
#         self.altimeter_array = None
#
#         self.current_node_pointer = None
#         self.direction = None
#
#     def append(self, new_node):
#         if self.head is None:
#             self.head = new_node
#         else:
#             temp = self.head
#             while temp.next:
#                 temp = temp.next
#
#             new_node.prev = temp
#             temp.next = new_node
#
#     def move_pointer_to_next(self, value):
#         if self.current_node_pointer is not None:
#             for i in range(value):
#                 if self.current_node_pointer is None:
#                     return None
#
#                 else:
#                     self.current_node_pointer = self.current_node_pointer.next
#
#         return self.current_node_pointer
#
#     def remove(self, current):
#         head = self.head
#
#         if current is head:
#             next = current.next
#             if next is not None:
#                 current.next = None
#                 next.prev = None
#                 self.head = next
#             else:
#                 self.head = None
#
#     def display(self):
#         temp = self.head
#         while temp:
#             if temp.start_time is not None:
#                 temp.display()
#
#             temp = temp.next
#
#     def set_end_time(self):
#         self.end_time = self.log_time + datetime.timedelta(seconds=(self.TTR))
#
#     def get_end_time(self):
#         return self.end_time
#
#     def set_start_floor(self, value):
#         self.start_floor = value
#
#     def set_destination_floor(self, value):
#         self.destination_floor = value
#
#     def get_start_floor(self):
#         return self.start_floor
#
#     def get_destination_floor(self):
#         return self.destination_floor
#
#     def set_log_time(self, start_time):
#         self.start_time = start_time
#
#     def set_TTR(self, TTR):
#         self.TTR = TTR
#
#     def get_TTR(self):
#         return self.TTR
#
#     def set_direction(self):
#         if self.start_floor is not None and self.destination_floor is not None:
#             if self.start_floor < self.destination_floor:
#                 self.direction = True
#             else:
#                 self.direction = False
#
#         else:
#             self.direction = None
#
#     def display(self):
#         print(f"Opertaion Start Time : {self.log_time}\nTTR : {self.TTR}\nStart Floor : {self.start_floor}\nDestination Floor : {self.destination_floor}\n")
#
#         node = self.head
#         while node is not None:
#             print(f"This Node Time : {node.current_time}\nTime Elapsed: {node.elapsed_time/1000} seconds\nAltimeter: {node.altimeter} meters\nClosest Floor: {node.closest_floor}\n")
#             node = node.next

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

def put_trip_node_data_into_trip_list(Elevator_System, elevator, TTR_array, altimeter_array, direction):
    #
    # TTR_array = [TTR, time_to_reach_maximum_velocity, time_to_stay_maximum, t]
    millisecond = 1000
    trip_list = elevator.full_trip_list.head
    trip_list.init_trip_list()

    if direction:
        constant_direction = 1
    else:
        constant_direction = -1

    if trip_list is None:
        print("Error Occured. Trip List Should be Init befor Assignment")
        return SystemError

    else:
        acceleration = elevator.get_acceleration()
        t = elevator.time_to_reach_maximum_velocity

        TTR = TTR_array[0]
        time_to_reach_maximum_velocity = TTR_array[1] * millisecond
        time_to_stay_maximum = TTR_array[2] * millisecond
        time_to_decelerate_to_zero = TTR_array[3] * millisecond

        TTR_to_microsecond = round(int(TTR * millisecond), -2)
        TTS_to_microsecond = round(int(Elevator_System.TTS * millisecond), -2)

        current_velocity = 0
        current_altimeter = altimeter_array[0]

        if TTR >= elevator.time_to_reach_maximum_velocity * 2:

            for elapsed_time in range(100, TTR_to_microsecond+100, 100):  # Loop through each 0.1 second
                if elapsed_time <= time_to_reach_maximum_velocity:  # Acceleration phase
                    before = current_velocity
                    current_velocity += acceleration * 0.1
                    after = current_velocity

                    current_altimeter += ((before + after) * 0.1 * 0.5) * constant_direction
                    #print(f"E time : {elapsed_time}, before : {before}, after : {after}")

                elif time_to_reach_maximum_velocity+100 <= elapsed_time <= time_to_reach_maximum_velocity + time_to_stay_maximum:  # Constant speed phase
                    current_altimeter += (current_velocity * 0.1) * constant_direction
                    #print(f"E time : {elapsed_time}")

                elif time_to_reach_maximum_velocity + time_to_stay_maximum + 100 <= elapsed_time <= time_to_reach_maximum_velocity + time_to_stay_maximum + time_to_decelerate_to_zero:  # Deceleration phase
                    before = current_velocity
                    current_velocity += (acceleration * -0.1)
                    after = current_velocity

                    current_altimeter += ((before + after) * 0.1 * 0.5) * constant_direction
                    #print(f"E time : {elapsed_time}, before : {before}, after : {after}")

                elif elapsed_time == TTR_to_microsecond:
                    current_altimeter = altimeter_array[1]

                current_altimeter = round(current_altimeter, 5)
                closest_floors = find_closest_floors(Elevator_System.alts_dict, current_altimeter)
                #print(f"E time : {elapsed_time}, velocity : {current_velocity}, altimeter : {current_altimeter}")

                temp_node = TripNode()

                temp_node.set_elapsed_time(elapsed_time)

                temp_node.set_current_time(datetime.timedelta(seconds=elapsed_time/millisecond))

                temp_node.set_velocity(current_velocity)
                temp_node.set_altimeter(current_altimeter)
                temp_node.set_closest_floor(closest_floors)

                trip_list.append_node(temp_node)
                trip_list.TTR = TTR_array

                if elapsed_time == TTR_to_microsecond:
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

                    current_altimeter += ((before + after) * 0.1 * 0.5) * constant_direction

                elif elapsed_time > TTR_to_microsecond/2:
                    before = current_velocity
                    current_velocity += (acceleration * -0.1)
                    after = current_velocity

                    current_altimeter += ((before + after) * 0.1 * 0.5) * constant_direction

                if elapsed_time == (TTR_to_microsecond-100):
                    current_altimeter = altimeter_array[0] + altimeter_array[2]

                current_altimeter = round(current_altimeter, 5)
                closest_floors = find_closest_floors(Elevator_System.alts_dict, current_altimeter)

                temp_node = TripNode()

                temp_node.set_elapsed_time(elapsed_time)
                temp_node.set_current_time(datetime.timedelta(seconds=elapsed_time/millisecond))

                temp_node.set_velocity(current_velocity)
                temp_node.set_altimeter(current_altimeter)
                temp_node.set_closest_floor(closest_floors)

                trip_list.append_node(temp_node)
                trip_list.TTR = TTR_array

                if elapsed_time == TTR_to_microsecond:
                    elevator.temp_log_timestamp = temp_node.get_current_time()

            return elevator


def get_TTR(elevator, altimeter_difference):
    elevator_current_velocity = elevator.get_current_velocity()
    elevator_maximum_velocity = elevator.maximum_velocity

    a = elevator.acceleration
    t = elevator.time_to_reach_maximum_velocity

    time_to_reach_maximum_velocity = round(elevator_maximum_velocity / a, 2)

    altimeter_to_reach_maximum_velocity = elevator_maximum_velocity * time_to_reach_maximum_velocity * 0.5
    altimeter_to_stay_maximum = round(altimeter_difference - altimeter_to_reach_maximum_velocity - (0.5 * a * t * t), 2)

    time_to_stay_maximum = altimeter_to_stay_maximum / elevator_maximum_velocity

    TTR = time_to_reach_maximum_velocity + time_to_stay_maximum + t

    return [TTR, time_to_reach_maximum_velocity, time_to_stay_maximum, t]

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

def draw_VT_moving(Elevator_System, elevator, altimeter_array):
    # altimeter_array =  [lower_floor_alts, higher_floor_alts, delta_altimeter, direction]

    # STEP 1 Get TTR
    altimeter_difference = altimeter_array[-2]

    if altimeter_array[0] < altimeter_array[1]:
        direction = True
    else:
        direction = False

    TTR_array = get_TTR(elevator, altimeter_difference)

    # Set Trip LIST
    elevator = put_trip_node_data_into_trip_list(Elevator_System, elevator, TTR_array, altimeter_array, direction)

    #elevator.current_trip_list.display()
    return elevator

def draw_VT_Staying():
    pass
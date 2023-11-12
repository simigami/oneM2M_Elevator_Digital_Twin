import Logic.Draw_VT as Draw_VT
import Logic.Manage_Trip_List as Manage_Trip_List

class Full_Trip_List:
    def __init__(self):
        self.head = None

        self.current_trip_list = None

    def init_head(self):
        self.head = None

    def set_current_trip_list(self, trip_list):
        self.current_trip_list = trip_list

    def get_current_trip_list(self, trip_list):
        return self.current_trip_list

    def remove_trip_list(self, trip_list):
        if trip_list.prev is None and trip_list.next is None:
            self.head = None

        elif trip_list.prev is None:
            next = trip_list.next

            next.prev = None
            trip_list.next = None

            self.head = next

        elif trip_list.next is None:
            trip_list.prev = None

    def append_trip_list(self, trip_list):
        if self.head is None:
            self.head = trip_list

        else:
            temp = self.head
            while temp.next:
                temp = temp.next

            trip_list.prev = temp
            temp.next = trip_list

    def insert_node_before_this_node(self, node, this):
        if this is not None:
            prev = this.prev
            if prev is None: # If This Node is Head Node
                self.head = node

                node.next = this
                this.prev = node

            else:
                prev.next = node
                this.prev = node

                node.next = this
                node.prev = prev

            this.start_floor = node.destination_floor

    def insert_node_after_this_node(self, node, this):
        if this is not None:
            next = this.next
            if next is None:
                self.append_trip_list(node)

            else:
                this.next = node
                next.prev = node

                node.next = next
                node.prev = this

    def get_last_trip_list(self):
        node = self.head
        while node.next is not None:
            node = node.next
        return node

    def display(self):
        head = self.head
        if head is not None:
            while head:
                print(f"start : {head.start_floor}, dst : {head.destination_floor}\n")
                head = head.next

def analyze_out_log(out_log):
    #latest_out_log.display()
    data = out_log.data

    inout = data.get_inout()
    floor = data.get_out_floor()
    timestamp = data.get_timestamp()
    direction = data.get_direction()

    return [inout, floor, timestamp, direction]

def can_elevator_reach_destination(Elevator_System, elevator, new_trip_dst_floor):
    current_altimeter = elevator.current_trip_node.altimeter
    if new_trip_dst_floor > 0:
        str_floor = str(new_trip_dst_floor)
    else:
        str_floor = 'B'+str(abs(new_trip_dst_floor))

    new_trip_dst_altimeter = Elevator_System.alts_dict[str_floor]
    delta_altimeter = abs(current_altimeter-new_trip_dst_altimeter)

    current_velocity = elevator.get_current_velocity()

    a = elevator.get_acceleration()
    t = elevator.time_to_reach_maximum_velocity

    time_to_decelerate_to_zero = current_velocity / a

    distance_to_decelerate_to_zero = time_to_decelerate_to_zero * a * 0.5
    if distance_to_decelerate_to_zero <= delta_altimeter: # Elevator Can Stop To this floor
        return True

    else: # Elevator Cannot Stop To This Floor
        return False

def run(Elevator_System, elevator, log_instance):
    opcode = elevator.get_opcode()
    log_array = None

    if log_instance is not None:
        log_array = analyze_out_log(log_instance)  # [inout, floor, timestamp, direction]

    direction = None

    #print(opcode)
    if opcode == 0:
        start_floor = elevator.get_current_stopped_floor()
        dst_floor = log_array[1]

        if start_floor < dst_floor:
            direction = True
        else:
            direction = False

        trip_list = Manage_Trip_List.Trip_List(start_floor, dst_floor, direction)

        # Make First Head of full trip list
        elevator.full_trip_list.append_trip_list(trip_list)

        # Set Current Trip List to head of full trip list
        elevator.current_trip_list = elevator.full_trip_list.head

        # return [lower_floor_alts, higher_floor_alts, delta_altimeter, direction]
        altimeter_array = Elevator_System.get_delta_altimeter_floor_and_floor(start_floor, dst_floor)

        elevator = Draw_VT.draw_VT_moving(Elevator_System, elevator, altimeter_array)

        #elevator.current_trip_list.display()
        elevator.current_trip_node = elevator.current_trip_list.head


    elif opcode == 10 or opcode == 20:
        if log_instance is None: # Elevator is end of IDLE on fixed floor but has operation
            elevator.current_trip_list = elevator.full_trip_list.head
            current_trip_list = elevator.current_trip_list

            start_floor = current_trip_list.start_floor
            dst_floor = current_trip_list.destination_floor

            if start_floor < dst_floor:
                direction = True
            else:
                direction = False

            altimeter_array = Elevator_System.get_delta_altimeter_floor_and_floor(start_floor, dst_floor)
            elevator = Draw_VT.draw_VT_moving(Elevator_System, elevator, altimeter_array)
            elevator.current_trip_node = current_trip_list.head

        elif log_instance is not None: # Log has been Transferred when Elevator is not IDLE but fixed on floor
            start_floor = elevator.get_current_stopped_floor()
            dst_floor = log_array[1]

            if start_floor < dst_floor:
                direction = True
            else:
                direction = False

            elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full(elevator.full_trip_list, dst_floor, direction)

        pass

    elif opcode == 21 or opcode == 22 or opcode == 23:
        direction = elevator.direction
        current_trip_list = elevator.current_trip_list
        current_trip_node = elevator.current_trip_node

        current_trip_dst_floor = current_trip_list.destination_floor
        new_trip_dst_floor = log_array[1]

        reachable = can_elevator_reach_destination(Elevator_System, elevator, new_trip_dst_floor)
        if direction: # If Elevator is Currently Moving Up
            if reachable: # If new dst floor can be reachable
                if new_trip_dst_floor < current_trip_dst_floor: # -5 -> 5 => -5 -> 3 -> 5
                    elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full(elevator.full_trip_list, new_trip_dst_floor, direction)

                    new_trip_start_floor = elevator.full_trip_list.head.start_floor
                    altimeter_array = Elevator_System.get_delta_altimeter_floor_and_floor(new_trip_start_floor, new_trip_dst_floor)

                    elevator = Draw_VT.draw_VT_moving(Elevator_System, elevator, altimeter_array)
                    elevator.current_trip_list = elevator.full_trip_list.head

                    current_time = current_trip_node.prev.current_time
                    node = elevator.current_trip_list.head
                    while node is not None:
                        if node.current_time != current_time:
                            node = node.next
                        else:
                            break

                    elevator.current_trip_node = node
                    print("Change Current Trip Node\n")
                    # temp = elevator.current_trip_node
                    # while temp.next is not None:
                    #     print(f"This Node Time : {temp.current_time}\nTime Elapsed: {temp.elapsed_time / 1000} seconds\nVelocity : {temp.velocity}m/s\nAltimeter: {temp.altimeter} meters\nClosest Floor: {temp.closest_floor}\n")
                    #     temp = temp.next

                    #elevator.current_trip_list.display()

                else: # -5 -> 5 => -5 -> 5 -> 8
                    current_trip_list = elevator.current_trip_list

                    elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full(elevator.full_trip_list, new_trip_dst_floor, direction)

                    pass

            else: # If new dst floor cannot be reachable
                opposite_direction = False if direction else True
                elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full(elevator.full_trip_list, new_trip_dst_floor, opposite_direction)
                pass

        else: # If Elevator is Currently Moving Down
            if reachable: # If new dst floor can be reachable
                if new_trip_dst_floor > current_trip_dst_floor: # 5 -> -5 => 5 -> 3 -> -5
                    elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full(elevator.full_trip_list, new_trip_dst_floor, direction)

                    new_trip_start_floor = elevator.full_trip_list.head.start_floor
                    altimeter_array = Elevator_System.get_delta_altimeter_floor_and_floor(new_trip_start_floor, new_trip_dst_floor)

                    elevator = Draw_VT.draw_VT_moving(Elevator_System, elevator, altimeter_array)
                    elevator.current_trip_list = elevator.full_trip_list.head

                    current_time = current_trip_node.prev.current_time
                    node = elevator.current_trip_list.head
                    while node is not None:
                        if node.current_time != current_time:
                            node = node.next
                        else:
                            break

                    elevator.current_trip_node = node
                    print("Change Current Trip Node\n")
                    # temp = elevator.current_trip_node
                    # while temp.next is not None:
                    #     print(f"This Node Time : {temp.current_time}\nTime Elapsed: {temp.elapsed_time / 1000} seconds\nVelocity : {temp.velocity}m/s\nAltimeter: {temp.altimeter} meters\nClosest Floor: {temp.closest_floor}\n")
                    #     temp = temp.next

                    #elevator.current_trip_list.display()

                else: # 5 -> -1 => 5 -> -1 -> -5
                    current_trip_list = elevator.current_trip_list
                    elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full(elevator.full_trip_list, new_trip_dst_floor, direction)

                    pass

            else: # If new dst floor cannot be reachable
                opposite_direction = False if direction else True
                elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full(elevator.full_trip_list, new_trip_dst_floor, opposite_direction)
                pass
            pass
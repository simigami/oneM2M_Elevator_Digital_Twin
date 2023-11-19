import Logic.Draw_VT as Draw_VT
import Logic.Manage_Trip_List as Manage_Trip_List

class Full_Trip_List:
    def __init__(self):
        self.reachable_head = None

        self.unreachable_up_head = None
        self.unreachable_down_head = None

        self.current_trip_list = None
        self.current_trip_list_direction = None

    def init_head(self):
        self.head = None

    def move_unreachable_head_to_reachable_head(self, flag):
        if flag == 1:
            self.reachable_head = self.unreachable_up_head
            self.unreachable_up_head = None
        elif flag == 2:
            self.reachable_head = self.unreachable_down_head
            self.unreachable_down_head = None

    def get_number_of_trip_list(self, flag):
        if flag == 0:
            node = self.reachable_head
        elif flag == 1:
            node = self.unreachable_up_head
        elif flag == 2:
            node = self.unreachable_down_head
        else:
            node = None

        count = 0
        while node:
            count += 1
            node = node.next

        return count

    def select_which_head(self, flag): # 0 Normal, 1 Unreachable Up 2 Unreachable Down
        if flag == 0:
            return self.reachable_head

        elif flag == 1:
            return self.unreachable_up_head

        elif flag == 2:
            return self.unreachable_down_head

        else:
            return None

    def set_current_trip_list(self, trip_list):
        self.current_trip_list = trip_list

    def get_current_trip_list(self, trip_list):
        return self.current_trip_list

    def remove_trip_list(self, trip_list):
        if trip_list.prev is None and trip_list.next is None:
            self.reachable_head = None

        elif trip_list.prev is None:
            next = trip_list.next

            next.prev = None
            trip_list.next = None

            self.reachable_head = next

        elif trip_list.next is None:
            trip_list.prev = None

    def append_trip_list(self, trip_list, flag):
        node = self.select_which_head(flag)
        if node is None:
            if flag == 0:
                self.reachable_head = trip_list

            elif flag == 1:
                self.unreachable_up_head = trip_list

            elif flag == 2:
                self.unreachable_down_head = trip_list

        else:
            while node.next:
                node = node.next

            node.prev = trip_list
            trip_list.next = node

    def insert_node_before_this_node(self, node, this, flag):
        if this.prev is None:
            if flag == 0:
                self.reachable_head = node

            elif flag == 1:
                self.unreachable_up_head = node

            elif flag == 2:
                self.unreachable_down_head = node
            node.next = this
            this.prev = node

        else:
            prev = this.prev
            prev.next = node
            node.prev = prev

            node.next = this
            this.prev = node

        return self

    def insert_node_after_this_node(self, node, this):
        this.next = node
        node.prev = this

        return self

    def get_last_trip_list(self, flag):
        node = self.select_which_head(flag)
        while node.next is not None:
            node = node.next
        return node

    def get_latest_node_of_direction(self, direction):
        node = self.head
        if node is not None:
            node_direction = node.direction
            while node_direction != direction:
                next = node.next
                if next is None:
                    return node
                else:
                    node = next
                    node_direction = node.direction

            return node

        else:
            return None

    def trim(self, flag):
        node = self.select_which_head(flag)
        if node is not None:
            node_dst_floor = node.destination_floor
            next = node.next
            while next:
                next_start_floor = next.start_floor
                if next_start_floor != node_dst_floor:
                    next.start_floor = node.destination_floor

                node = next
                node_dst_floor = node.destination_floor
                next = next.next

    def display(self):
        head = self.reachable_head
        head2 = self.unreachable_up_head
        head3 = self.unreachable_down_head

        print("Reachable List")
        if head is not None:
            while head:
                print(f"start : {head.start_floor}, dst : {head.destination_floor}\ndirection : {head.direction}\n")
                head = head.next
        print("Unreachable Up List\n")
        if head2 is not None:
            while head2:
                print(f"start : {head2.start_floor}, dst : {head2.destination_floor}\ndirection : {head2.direction}\n")
                head2 = head2.next
        print("Unreachable Down List\n")
        if head3 is not None:
            while head3:
                print(f"start : {head3.start_floor}, dst : {head3.destination_floor}\ndirection : {head3.direction}\n")
                head3 = head3.next

def str_floor_to_int(str_floor):
    if str_floor.startswith('B'):
        return -int(str_floor[1:])
    else:
        return int(str_floor)

def analyze_in_log(in_log):
    data = in_log.data
    log_array = data.get_resources()
    
    print(log_array)
    return log_array
    

def analyze_out_log(out_log):
    #latest_out_log.display()
    data = out_log.data

    inout = data.get_inout()
    floor = data.get_out_floor()
    timestamp = data.get_timestamp()
    direction = data.get_direction()

    return [inout, floor, timestamp, direction]

def can_elevator_reach_destination(Elevator_System, elevator, new_trip_dst_floor):
    current_trip_node = elevator.current_trip_node

    if current_trip_node is None:
        current_altimeter = elevator.current_stopped_altimeter
    else:
        current_altimeter = elevator.current_trip_node.altimeter

    if new_trip_dst_floor > 0:
        str_floor = str(new_trip_dst_floor)
    else:
        str_floor = 'B'+str(abs(new_trip_dst_floor))

    new_trip_dst_altimeter = Elevator_System.alts_dict[str_floor]

    direction = elevator.direction
    if direction and new_trip_dst_altimeter < current_altimeter:
        return False

    elif not direction and new_trip_dst_altimeter > current_altimeter:
        return False

    else:
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

def run_In(Elevator_System, elevator, log_instance):
    pass

def run(Elevator_System, elevator, log_instance):
    opcode = elevator.get_opcode()
    log_array = None

    if log_instance is not None:
        data = log_instance.data
        if data.inout is True: # In Log
            # [in_previous_buttons, in_current_buttons, in_current_floor, in_elevator_number , timestamp]
            log_array = analyze_in_log(log_instance)    
        else: # Out Log
            log_array = analyze_out_log(log_instance)   # [inout, floor, timestamp, direction]
     
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
        elevator.full_trip_list.append_trip_list(trip_list, 0)

        # Set Current Trip List to head of full trip list
        elevator.current_trip_list = elevator.full_trip_list.reachable_head

        # return [lower_floor_alts, higher_floor_alts, delta_altimeter, direction]
        altimeter_array = Elevator_System.get_delta_altimeter_floor_and_floor(start_floor, dst_floor)

        elevator = Draw_VT.draw_VT_moving(Elevator_System, elevator, altimeter_array)

        #elevator.current_trip_list.display()
        elevator.current_trip_node = elevator.current_trip_list.head


    elif opcode == 10 or opcode == 20:
        if log_instance is None: # Elevator is end of IDLE on fixed floor but has operation
            elevator.current_trip_list = elevator.full_trip_list.reachable_head
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
            
            # Check if there is other reachable trip list remained
            presume_next_trip_list = elevator.full_trip_list.reachable_head
            
            # Check if this log_instance is In or Out
            inout = log_instance.data.inout
            if inout is True: # In Log
                # [in_previous_buttons, in_current_buttons, in_current_floor, in_elevator_number , timestamp]
                # Step 1. Find Difference = Get Button = Get Trip Dst
                delta = list(set(log_array[1]) - set(log_array[0]))
                if len(delta) != 1:
                    print("Error Occurred")
                    return None
                
                else:
                     dst_floor = delta[0]
                
                # Step 2. Find Direction = Get Current Elevator Floor and compare with dst_floor
                str_floor = log_array[2][0]
                int_floor = str_floor_to_int(str_floor)
                
                if int_floor < dst_floor:
                    dst_floor_direction = True
                else:
                    dst_floor_direction = False
                
                
                pass
            else: # Out Log
                dst_floor = log_array[1]  # [inout, floor, timestamp, direction]
                dst_floor_direction = log_array[-1]

            if presume_next_trip_list is None: # There is no other trip remained
                # Check if there is any unreachable header exists
                unreachable_up = elevator.full_trip_list.unreachable_up_head
                unreachable_down = elevator.full_trip_list.unreachable_down_head
                # if unreachable_up is not None or unreachable_down is not None:
                #     reachable = 0
                #     elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full_trip_list(
                #         elevator.full_trip_list,
                #         dst_floor,
                #         dst_floor_direction,
                #         reachable)

                # If there is not unreachable header exists. This means Elevator has no more operation, So It can move to different direction
                #else:
                elevator.direction = dst_floor_direction
                reachable = 1
                elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full_trip_list(elevator.full_trip_list,
                                                                                              dst_floor,
                                                                                              dst_floor_direction,
                                                                                              reachable)

                elevator.full_trip_list.reachable_head.start_floor = elevator.current_stopped_floor

            else: # There is Trip Remained
                current_elevator_direction = True if presume_next_trip_list.destination_floor - presume_next_trip_list.start_floor > 0 else False

                if current_elevator_direction != dst_floor_direction: # Elevator Cannot move to this floor
                    reachable = 0
                    elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full_trip_list(elevator.full_trip_list, dst_floor,
                                                                                        dst_floor_direction, reachable)

                else: # Elevator Can move to this floor but have to check reachability
                    reachable = can_elevator_reach_destination(Elevator_System, elevator, dst_floor)
                    elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full_trip_list(elevator.full_trip_list, dst_floor,
                                                                                        dst_floor_direction, reachable)

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
                elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full_trip_list(elevator.full_trip_list,
                                                                                              new_trip_dst_floor,
                                                                                              direction, reachable)
                if new_trip_dst_floor < current_trip_dst_floor: # -5 -> 5 => -5 -> 3 -> 5
                    new_trip_start_floor = elevator.full_trip_list.reachable_head.start_floor
                    altimeter_array = Elevator_System.get_delta_altimeter_floor_and_floor(new_trip_start_floor, new_trip_dst_floor)

                    elevator = Draw_VT.draw_VT_moving(Elevator_System, elevator, altimeter_array)
                    elevator.current_trip_list = elevator.full_trip_list.reachable_head

                    current_time = current_trip_node.prev.current_time
                    node = elevator.current_trip_list.head
                    while node is not None:
                        if node.current_time != current_time:
                            node = node.next
                        else:
                            break

                    elevator.current_trip_node = node
                    # temp = elevator.current_trip_node
                    # while temp.next is not None:
                    #     print(f"This Node Time : {temp.current_time}\nTime Elapsed: {temp.elapsed_time / 1000} seconds\nVelocity : {temp.velocity}m/s\nAltimeter: {temp.altimeter} meters\nClosest Floor: {temp.closest_floor}\n")
                    #     temp = temp.next

                    #elevator.current_trip_list.display()

                else: # -5 -> 5 => -5 -> 5 -> 8
                    pass

            else: # If new dst floor cannot be reachable
                direction = log_array[-1]
                elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full_trip_list(elevator.full_trip_list, new_trip_dst_floor, direction, reachable)
                pass

        else: # If Elevator is Currently Moving Down
            if reachable: # If new dst floor can be reachable
                elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full_trip_list(elevator.full_trip_list,
                                                                                              new_trip_dst_floor,
                                                                                              direction, reachable)
                if new_trip_dst_floor > current_trip_dst_floor: # 5 -> -5 => 5 -> 3 -> -5
                    new_trip_start_floor = elevator.full_trip_list.reachable_head.start_floor
                    altimeter_array = Elevator_System.get_delta_altimeter_floor_and_floor(new_trip_start_floor, new_trip_dst_floor)

                    elevator = Draw_VT.draw_VT_moving(Elevator_System, elevator, altimeter_array)
                    elevator.current_trip_list = elevator.full_trip_list.reachable_head

                    current_time = current_trip_node.prev.current_time
                    node = elevator.current_trip_list.head
                    while node is not None:
                        if node.current_time != current_time:
                            node = node.next
                        else:
                            break

                    elevator.current_trip_node = node
                    #print("Change Current Trip Node\n")
                    # temp = elevator.current_trip_node
                    # while temp.next is not None:
                    #     print(f"This Node Time : {temp.current_time}\nTime Elapsed: {temp.elapsed_time / 1000} seconds\nVelocity : {temp.velocity}m/s\nAltimeter: {temp.altimeter} meters\nClosest Floor: {temp.closest_floor}\n")
                    #     temp = temp.next

                    #elevator.current_trip_list.display()

                else: # 5 -> -1 => 5 -> -1 -> -5
                    pass

            else: # If new dst floor cannot be reachable
                opposite_direction = False if direction else True
                elevator.full_trip_list = Manage_Trip_List.append_trip_list_to_Full(elevator.full_trip_list, new_trip_dst_floor, opposite_direction)
                pass
            pass
import copy


class Full_Trip_List:
    def __init__(self):
        self.reachable_head = None

        self.unreachable_up_head = None
        self.unreachable_down_head = None

        self.current_trip_list = None

    def init_head(self):
        self.head = None

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
            self.head = None

        elif trip_list.prev is None:
            next = trip_list.next

            next.prev = None
            trip_list.next = None

            self.head = next

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


class Trip_List:
    def __init__(self, start, dst, direction):
        self.head = None    # Node Start
        self.pointer = None # Node Pointer

        self.next = None # Trip List Next
        self.prev = None # Trip List Prev

        self.start_floor = start # This Trip Start Floor
        self.destination_floor = dst # This Trip Dst Floor

        self.direction = direction
        self.TTR = []

        self.operation_start_time = None # This Trip Start Time : Can be same as log
        self.operation_end_time = None # This Trip Estimated End Time

    def append_node(self, node):
        if self.head is None:
            self.head = node

        else:
            temp = self.head
            while temp.next:
                temp = temp.next

            node.prev = temp
            temp.next = node

    def init_trip_list(self):
        self.TTR = []
        self.head = None

    def display(self):
        print(f"Start Floor : {self.start_floor}\nDestination Floor : {self.destination_floor}\nTTR : {self.TTR}\n")

        node = self.head
        while node is not None:
            print(f"This Node Time : {node.current_time}\nTime Elapsed: {node.elapsed_time/1000} seconds\nVelocity : {node.velocity}m/s\nAltimeter: {node.altimeter} meters\nClosest Floor: {node.closest_floor}\n")
            node = node.next

def init_full_trip_list(start_floor, dst_floor, direction, flag):
    full_trip_list = Full_Trip_List()
    trip_list = Trip_List(start_floor, dst_floor, direction)

    full_trip_list.append_trip_list(trip_list, flag)
    return full_trip_list

def make_trip_list(start_floor, dst_floor, direction):
    trip_list = Trip_List(start_floor, dst_floor, direction)

    return trip_list

def sort_trip_list(full_trip_list, trip_list_head, new_trip_list, direction, flag):
    if trip_list_head is None: # If head is Empty
        trip_list_head = new_trip_list

        return trip_list_head

    else:
        new_trip_dst = new_trip_list.destination_floor

        search_start = trip_list_head
        while trip_list_head.next:
            trip_list_head = trip_list_head.next

        while trip_list_head != search_start:
            end_to_start_node_dst = trip_list_head.destination_floor

            if direction and end_to_start_node_dst < new_trip_dst:
                full_trip_list = full_trip_list.insert_node_after_this_node(new_trip_list, trip_list_head)
                full_trip_list.trim(flag)

                return full_trip_list

            elif direction and end_to_start_node_dst > new_trip_dst:
                trip_list_head = trip_list_head.prev

            elif not direction and end_to_start_node_dst > new_trip_dst:
                full_trip_list = full_trip_list.insert_node_after_this_node(new_trip_list, trip_list_head)
                full_trip_list.trim(flag)

                return full_trip_list

            elif not direction and end_to_start_node_dst < new_trip_dst:
                trip_list_head = trip_list_head.prev

        if trip_list_head == search_start:
            current_node_dst = search_start.destination_floor

            if direction and current_node_dst < new_trip_dst:
                full_trip_list = full_trip_list.insert_node_after_this_node(new_trip_list, trip_list_head)

            elif direction and current_node_dst > new_trip_dst:
                new_trip_list.start_floor = search_start.start_floor
                full_trip_list = full_trip_list.insert_node_before_this_node(new_trip_list, trip_list_head, flag)

            if not direction and current_node_dst > new_trip_dst:
                full_trip_list = full_trip_list.insert_node_after_this_node(new_trip_list, trip_list_head)

            elif not direction and current_node_dst < new_trip_dst:
                new_trip_list.start_floor = search_start.start_floor
                full_trip_list = full_trip_list.insert_node_before_this_node(new_trip_list, trip_list_head, flag)

            full_trip_list.trim(flag)

            return full_trip_list

def append_trip_list_to_Full_trip_list(full_trip_list, new_trip_dst, direction, reachable):
    if reachable:
        flag = 0
        trip_list_head = full_trip_list.select_which_head(flag)
        if trip_list_head is None:
            new_trip_list = make_trip_list(0, new_trip_dst, direction)
            if flag == 0:
                full_trip_list.reachable_head = new_trip_list
            elif flag == 1:
                full_trip_list.unreachable_up_head = new_trip_list
            elif flag == 2:
                full_trip_list.unreachable_down_head = new_trip_list

            return full_trip_list

        else:
            new_trip_list = make_trip_list(0, new_trip_dst, direction)

            full_trip_list = sort_trip_list(full_trip_list, trip_list_head, new_trip_list, direction, flag)

            return full_trip_list

    else:
        if direction:
            flag = 1
        else:
            flag = 2

        trip_list_head = full_trip_list.select_which_head(flag)
        if trip_list_head is None:
            if flag == 1:
                trip_list = Trip_List(0, new_trip_dst, True)
                full_trip_list.append_trip_list(trip_list, flag)

            elif flag == 2:
                trip_list = Trip_List(0, new_trip_dst, False)
                full_trip_list.append_trip_list(trip_list, flag)

        else:
            new_trip_list = make_trip_list(0, new_trip_dst, direction)

            full_trip_list = sort_trip_list(full_trip_list, trip_list_head, new_trip_list, direction, flag)

        return full_trip_list


if __name__ == '__main__':
    full_trip_list = init_full_trip_list(12, 5, False, 0)
    full_trip_list = append_trip_list_to_Full_trip_list(full_trip_list, 8, False, True)
    full_trip_list = append_trip_list_to_Full_trip_list(full_trip_list, 1, False, True)
    full_trip_list = append_trip_list_to_Full_trip_list(full_trip_list, 4, True, False)
    full_trip_list = append_trip_list_to_Full_trip_list(full_trip_list, 7, False, False)
    full_trip_list = append_trip_list_to_Full_trip_list(full_trip_list, -2, False, True)
    full_trip_list = append_trip_list_to_Full_trip_list(full_trip_list, -5, False, False)
    full_trip_list.display()
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

def check_trip_list(Full_Trip_List, new_trip_dst, direction): # Direction True = Up, False = Down
    node = Full_Trip_List.head
    last_trip_dst = None

    if node is not None:
        while node is not None:
            next = node.next
            trip_list_start = node.start_floor
            trip_list_dst = node.destination_floor
            if direction and new_trip_dst < trip_list_dst:
                new_trip_start = trip_list_start

                new_trip_list = Trip_List(new_trip_start, new_trip_dst, direction)
                Full_Trip_List.insert_node_before_this_node(new_trip_list, node)

                break
            elif not direction and new_trip_dst > trip_list_dst:
                new_trip_start = trip_list_start

                new_trip_list = Trip_List(new_trip_start, new_trip_dst, direction)
                Full_Trip_List.insert_node_before_this_node(new_trip_list, node)

                break

            elif next is None: # If This node is Last Trip List
                last_trip_dst = node.destination_floor # This dst will be a starting floor of next trip

            elif next.direction != node.direction:
                new_trip_start = trip_list_dst
                new_trip_list = Trip_List(new_trip_start, new_trip_dst, direction)
                Full_Trip_List.insert_node_before_this_node(new_trip_list, next)

            node = next

        if node is None and last_trip_dst is not None:
            new_trip_list = Trip_List(last_trip_dst, new_trip_dst, direction)
            Full_Trip_List.append_trip_list(new_trip_list)

    return Full_Trip_List

def append_to_latest_trip_list(Full_Trip_List, new_trip_dst, direction):
    last_trip_list = Full_Trip_List.get_last_trip_list()

    last_trip_dst = last_trip_list.destination_floor # This will be next trip starting floor

    trip_list = Trip_List(last_trip_dst, new_trip_dst, direction)
    Full_Trip_List.append_trip_list(trip_list)

    return Full_Trip_List

def append_trip_list_to_Full(Full_Trip_List, new_trip_dst, direction):
    node = Full_Trip_List.head
    while node.direction != direction:
        next = node.next
        if next is None: # If there is not opposite direction trip list
            Full_Trip_List = append_to_latest_trip_list(Full_Trip_List, new_trip_dst, direction)
            return Full_Trip_List

        else: # If there is opposite direction trip list
            node = node.next

    if node is not None:
        last_trip_dst = None
        while node is not None:
            next = node.next
            trip_list_start = node.start_floor
            trip_list_dst = node.destination_floor
            if direction and new_trip_dst < trip_list_dst:
                new_trip_start = trip_list_start

                new_trip_list = Trip_List(new_trip_start, new_trip_dst, direction)
                Full_Trip_List.insert_node_before_this_node(new_trip_list, node)

                break

            elif not direction and new_trip_dst > trip_list_dst:
                new_trip_start = trip_list_start

                new_trip_list = Trip_List(new_trip_start, new_trip_dst, direction)
                Full_Trip_List.insert_node_before_this_node(new_trip_list, node)

                break

            elif next is None:  # If This node is Last Trip List
                last_trip_dst = node.destination_floor  # This dst will be a starting floor of next trip

            elif next.direction != node.direction:
                new_trip_start = trip_list_dst
                new_trip_list = Trip_List(new_trip_start, new_trip_dst, direction)
                Full_Trip_List.insert_node_before_this_node(new_trip_list, next)
                next.start_floor = new_trip_dst

                break

            node = next

        if node is None and last_trip_dst is not None:
            new_trip_list = Trip_List(last_trip_dst, new_trip_dst, direction)
            Full_Trip_List.append_trip_list(new_trip_list)

        return Full_Trip_List

def make_trip_list(start_floor, dst_floor):
    trip_list = Trip_List(start_floor, dst_floor)

    return trip_list

if __name__ == '__main__':
    full_trip_list = Full_Trip_List()

    trip_list = Trip_List(-5, 5, True)

    full_trip_list.append_trip_list(trip_list)
    full_trip_list = append_trip_list_to_Full(full_trip_list, 4, True)
    full_trip_list = append_trip_list_to_Full(full_trip_list, 8, True)
    full_trip_list = append_trip_list_to_Full(full_trip_list, 7, False)
    full_trip_list = append_trip_list_to_Full(full_trip_list, 12, True)
    full_trip_list = append_trip_list_to_Full(full_trip_list, 10, False)

    full_trip_list.display()
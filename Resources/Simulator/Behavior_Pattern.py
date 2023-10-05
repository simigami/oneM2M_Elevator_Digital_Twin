from config import TEST_VARIABLES

class pattern:
    def __init__(self):
        self.parse_data_instance = parse_data()
        self.Number_Of_Trips_To_Floor = [0 for i in range(TEST_VARIABLES.total_floors)]
        self.Number_Of_Trips_To_Floor_Of_Each_Floor = [[0 for i in range(TEST_VARIABLES.total_floors)] for j in range(TEST_VARIABLES.total_floors)]
        self.trip_path = []
        self.floor = None
        self.trip_direction = None  # True = Up, False = Down

    def print_var(self):
        print(self.Number_Of_Trips_To_Floor)
        print(self.Number_Of_Trips_To_Floor_Of_Each_Floor)
        print(self.parse_data_instance.data)

    def reset_trip_path(self):
        self.trip_path = []

    def add_trip_path(self, floor):
        self.trip_path.append(floor)

    def pop_trip_path(self, floor):
        self.trip_path.pop(floor)

    def remove_trip_path(self, trip_path, floor):
        if floor in trip_path:
            trip_path.remove(floor)
        else:
            print("Error in trip_path, no such floor in list")

class parse_data:
    def __init__(self):
        self.data = {
            'Inout': '',
            'Timestamp': '',
            'Operation': '',
            'Previous Button Panel': '',
            'Current Button Panel' : '',
            'Current Floor' : [0, 0]
        }

def init():
    pattern_instance = pattern()

    return pattern_instance

def add_total_trip(pbl, cbl, lower_floor_cid, higher_floor_cid):
    if sum(TEST_VARIABLES.Number_Of_Trips_To_Floor) == 0:
        for i in range(len(pbl)):
            pbl[i] = int(pbl[i])

        for i in range(len(cbl)):
            cbl[i] = int(cbl[i])

        difference = list(set(pbl+cbl))
        for elem in difference:
            TEST_VARIABLES.Number_Of_Trips_To_Floor[elem] += 1
            TEST_VARIABLES.Number_Of_Trips_To_Floor_Of_Each_Floor[lower_floor_cid][elem] += 1

    else:
        for i in range(len(pbl)):
            pbl[i] = int(pbl[i])

        for i in range(len(cbl)):
            cbl[i] = int(cbl[i])

        difference = [x for x in cbl if x not in pbl]
        for elem in difference:
            TEST_VARIABLES.Number_Of_Trips_To_Floor[elem] += 1
            TEST_VARIABLES.Number_Of_Trips_To_Floor_Of_Each_Floor[lower_floor_cid][elem] += 1

def sort_algorithm(arr):
    pivot = arr[0]
    greater = [x for x in arr[1:] if x> pivot]
    lesser = [x for x in arr[1:] if x< pivot]

    return [pivot] + sorted(greater) + sorted(lesser, reverse=True)

def refresh_trip_path(pattern):
    pbl = pattern.parse_data_instance.data['Previous Button Panel']
    pbl_int = []
    cbl = pattern.parse_data_instance.data['Current Button Panel']
    cbl_int = []
    cf = pattern.parse_data_instance.data['Current Floor']

    for elem in pbl:
        if elem[0] == 'B':
            button_int = int(elem[1]) * -1
        elif elem[0] == 'C' or elem[0] == 'O':
            continue
        else:
            button_int = int(elem[0])
        pbl_int.append(button_int)

    for elem in cbl:
        if elem[0] == 'B':
            button_int = int(elem[1]) * -1
        elif elem[0] == 'C' or elem[0] == 'O':
            continue
        else:
            button_int = int(elem[0])
        cbl_int.append(button_int)

    floor_int = 0
    for elem in cf:
        if elem[0] == 'B':
            floor_int += int(elem[1]) * -1
        elif elem[0] == 'C' or elem[0] == 'O':
            pass
        else:
            button_int = int(elem[0])
    floor_int = round(floor_int / 2, 1)

    if len(pbl) == 0:
        pattern.reset_trip_path()

        for elem in cbl_int:
            pattern.add_trip_path(elem)

        pattern.trip_path.sort()
        pattern.floor = floor_int

    else:
        if floor_int >= pattern.floor:  # going up
            pattern.trip_direction = True
        else:   # going down
            pattern.trip_direction = False

        if pattern.trip_direction:
            disappearance = set(pbl_int)-set(cbl_int)
            appearance = set(cbl_int)-set(pbl_int)

            print(disappearance)
            print(appearance)

            for elem in disappearance:
                print(elem)
                pattern.pop_trip_path(elem)
                print("pop end")

            for elem in appearance:
                print(elem)
                pattern.add_trip_path(elem)
                print("add end")

            sort_algorithm(pattern.trip_path)

            print(pbl)
            print(cbl)
            print(pattern.trip_direction)
            print(pattern.trip_path)
            print('\n')
        else:
            pass

    return pattern

def parse_log_to_data(file_name, pattern):
    with open(file_name, 'r') as f:
        log = f.read()

    paragraphs = log.strip().split('\n\n')
    parsed_logs = []

    for paragraph in paragraphs:
        lines = paragraph.strip().split('\n')
        if len(lines) == 6:
            for i in range(len(lines)):
                if i != 2:
                    value = lines[i].split(': ')
                    pattern.parse_data_instance.data[value[0]] = value[1]
                else:
                    key, value = 'Operation', lines[i]
                    pattern.parse_data_instance.data[key] = value

            pattern.parse_data_instance.data['Previous Button Panel'] = eval(pattern.parse_data_instance.data['Previous Button Panel'])
            pattern.parse_data_instance.data['Current Button Panel'] = eval(pattern.parse_data_instance.data['Current Button Panel'])
            floor_list = pattern.parse_data_instance.data['Current Floor'].split(' between ')
            pattern.parse_data_instance.data['Current Floor'] = [floor_list[0], floor_list[1]]

            #print(pattern.parse_data_instance.data)
            pattern = refresh_trip_path(pattern)

        else:
            print("Error in lines, log attribute does not match")



    return parsed_logs

if __name__ == '__main__':
    pattern = init()

    path = rf"E:\ML\Elevator Git\Elevator_Results\temp.txt"
    parse_log_to_data(path, pattern)

    pattern.print_var()
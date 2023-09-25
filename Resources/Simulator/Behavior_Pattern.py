from config import TEST_VARIABLES
def init():
    TEST_VARIABLES.Number_Of_Trips_To_Floor = [0 for i in range(TEST_VARIABLES.total_floors)]
    TEST_VARIABLES.Number_Of_Trips_To_Floor_Of_Each_Floor = [[0 for i in range(TEST_VARIABLES.total_floors)] for j in range(TEST_VARIABLES.total_floors)]

    print(TEST_VARIABLES.Number_Of_Trips_To_Floor)
    print(TEST_VARIABLES.Number_Of_Trips_To_Floor_Of_Each_Floor)

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

if __name__ == '__main__':
    init()
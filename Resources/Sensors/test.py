def analyze_commands(commands):
    # 전체 이동 경로들을 저장할 리스트
    all_paths = []
    all_inside = []
    res = []

    # 그 이전 명령어를 저장할 변수
    prevCommand = ""
    currentStopIndex = 0
    prevStopIndex = 0

    for command in commands:
        if command.startswith("START AT"):
            if prevCommand == "" or prevCommand == "IDLE":
                prevCommand = "START"
                pass

            else:
                print("ERROR OCCURRED AT START COMMAND")
                exit(0)

        elif command.startswith("STOP"):
            floor = strFloorToInt([command.split()[2]])[0]

            if prevCommand == "START":
                all_paths.append([floor])
                currentStopIndex = len(all_paths) - 1
                prevStopIndex = currentStopIndex - 1

            elif prevCommand == "STOP":
                pass

            elif prevCommand == "MOV":
                currentSTOPList = all_paths[currentStopIndex]
                if floor not in all_inside:
                    all_paths.append([floor])
                    currentStopIndex = len(all_paths) - 1
                    prevStopIndex = currentStopIndex - 1

                else:
                    all_inside.remove(floor)
                    all_paths.append([floor])
                    currentStopIndex = len(all_paths) - 1
                    prevStopIndex = currentStopIndex - 1

            prevCommand = "STOP"

        elif command.startswith("MOV"):
            movList = strFloorToInt(command.split()[2].split(','))
            diff = list(set(movList).difference(set(all_inside)))

            for elem in diff:
                if prevCommand == "STOP":
                    if prevStopIndex == -1:
                        all_paths[currentStopIndex].append(elem)

                    else:
                        previousSTOPList = all_paths[prevStopIndex]
                        all_paths[currentStopIndex].append(elem)

                elif prevCommand == "MOV":
                    currentSTOPList = all_paths[currentStopIndex]
                    currentSTOPList.append(elem)

                all_inside.append(elem)

            prevCommand = "MOV"

        elif command.startswith("IDLE"):
            res = []
            # IDLE 명령어일 경우, 현재 이동 경로를 전체 경로에 추가하고 초기화
            for trip in all_paths:
                if len(trip) != 1:
                    res.append(trip)

            currentStopIndex = 0
            prevStopIndex = 0

            prevCommand = "IDLE"

    return res

def strFloorToInt(floor):
    temp = []

    for elem in floor:
        if len(elem) >= 2 and elem[0] == 'B':
            temp.append(-1 * int(elem[1:]))

        else:
            temp.append(int(elem[0:]))

    return temp

# 예시 명령어 세트
commands = '''START AT B5
STOP AT 6
MOV TO B1,1
STOP AT 2
STOP AT 1
MOV TO B3,B1
STOP AT B1
STOP AT B3
IDLE'''

commands = list(commands.split('\n'))

result = analyze_commands(commands)
print(result)  # 출력: [[6, 1], [2, -3]]
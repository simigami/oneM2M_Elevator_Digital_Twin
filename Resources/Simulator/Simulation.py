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
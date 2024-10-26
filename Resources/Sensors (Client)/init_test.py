# THIS file is responsible for gather initial data from config.ini and make init object to send data to DT Server
# config.ini file contains basic information of building and elevators
# 1 Building Must have fully implemented one config.ini file to make this object propertly
# In Each config.ini file, it must have all of information about building and elevators
# If You want to add or delete a Elevator informations in config.ini You can use modify.py to modify it
import ast
import copy
import configparser


class DataBase:
    def __init__(self):
        self.db = {}
    def add(self, key, value):
        if key in self.db:
            print("cannot add existing key")
            return

        self.db[key] = value
        return

    def modify(self, key, new_value):
        if key not in self.db:
            print("cannot modify non-existing key")
            return

        self.db[key] = new_value

    def delete(self, key):
        if key not in self.db:
            print("cannot delete non-existing key")
            return

        self.db.pop(key, None)

    def get_value(self, key):
        if key not in self.db:
            print("cannot get non-existing key's value")
            return

        return self.db[key]

class DTServer:
    def __init__(self):
        self.ipaddr = None
        self.port = None

    def set_ip(self, ip):
        self.ipaddr = ip

    def set_port(self, port):
        self.port = port

class Building:
    def __init__(self):
        self.building_database = DataBase()
        self.elevator_list = {}

    def appendEV(self, ev_name):
        new_elevator = Elevator()
        self.elevator_list[ev_name] = new_elevator

class Elevator:
    def __init__(self):
        self.default_info = DataBase()
        self.database = []

    def create_database(self):
        self.database.append(DataBase())

ini_path = r"E:\ML\Elevator Git\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\Resources\Sensors (Client)\config.ini"
def set_init_data():
    # Read config.ini
    config = configparser.ConfigParser()
    config.read(ini_path)
    sections = config.sections()

    # Make Building Object
    this_building = Building()
    this_server = DTServer()

    try:
        # Read config.ini and make Building Object
        for each_section in sections:
            options = config.options(each_section)

            if each_section == "BUILDING CONFIG":
                for each_option in options:
                    this_building.building_database.add(each_option, config[each_section][each_option])

            elif each_section == "DT SERVER CONFIG":
                for each_option in options:
                    if each_option == "server ip":
                        this_server.set_ip(config[each_section][each_option])
                    elif each_option == "server port":
                        this_server.set_port(config[each_section][each_option])

            else:
                # find option name "elevator name" and make Elevator Object
                if "elevator name" not in options:
                    Exception("elevator name is not in config.ini")

                ev_name = config[each_section]["elevator name"]
                this_building.appendEV(ev_name)

                for each_option in options:
                    if each_option == "each floor altimeter":
                        this_building.elevator_list[ev_name].default_info.add(each_option, ast.literal_eval(config[each_section][each_option]))

                    if each_option == "init floor":
                        this_building.elevator_list[ev_name].default_info.add(each_option, int(config[each_section][each_option]) if config[each_section][each_option].isdigit() else -1*int(config[each_section][each_option][1:]))

                    elif config[each_section][each_option].isdigit():
                        this_building.elevator_list[ev_name].default_info.add(each_option, int(config[each_section][each_option]))

                    elif config[each_section][each_option].replace('.', '', 1).isdigit():
                        this_building.elevator_list[ev_name].default_info.add(each_option, float(config[each_section][each_option]))

                    elif config[each_section][each_option].upper() == "TRUE":
                        this_building.elevator_list[ev_name].default_info.add(each_option, True)

                    elif config[each_section][each_option].upper() == "FALSE":
                        this_building.elevator_list[ev_name].default_info.add(each_option, False)

                    elif config[each_section][each_option] == "None":
                        this_building.elevator_list[ev_name].default_info.add(each_option, None)

                    else:
                        this_building.elevator_list[ev_name].default_info.add(each_option, config[each_section][each_option])

        return this_server, this_building

    except Exception as e:
        print(e)
        return None, None

if __name__ == '__main__':
    set_init_data()
# THIS file is responsible for gather initial data from config.ini and make init object to send data to DT Server
# config.ini file contains basic information of building and elevators
# 1 Building Must have fully implemented one config.ini file to make this object propertly
# In Each config.ini file, it must have all of information about building and elevators
# If You want to add or delete a Elevator informations in config.ini You can use modify.py to modify it
import ast
import copy
import configparser

class DTServer:
    def __init__(self):
        self.ipaddr = None
        self.port = None

    def set_ip(self, ip):
        self.ipaddr = ip

    def set_port(self, port):
        self.port = port

class Buliding:
    def __init__(self, building_name):
        self.building_name = building_name
        self.evlist = []
        self.outflag = False
        self.outname = None
        self.rts_path = None

    def set_out(self, flag):
        self.outflag = flag

    def set_outname(self, name):
        self.outname = name

    def appendEV(self, ev_name):
        newEV = Elevator(ev_name)
        self.evlist.append(newEV)

class Elevator:
    def __init__(self, ev_name):
        self.ev_name = ev_name
        self.altimeter_list = []
        self.underground_floor = 0
        self.ground_floor = 0
        self.energy_flag = False
        self.physical_flag = False
        self.physical_path = None
        self.acceleration = 0
        self.max_velocity = 0
        self.E_idle = 0
        self.E_standby = 0
        self.E_ref = 0

    def set_floor_altimeter(self, und, gnd, altimeter_list):
        self.underground_floor = und
        self.ground_floor = gnd
        self.altimeter_list = copy.deepcopy(altimeter_list)

        if len(self.altimeter_list) != und+gnd:
            print("altimeter length is different")
            self.altimeter_list = []
            self.underground_floor = 0
            self.ground_floor = 0

    def set_energy(self, flag):
        self.energy_flag = flag

    def set_energy_value(self, i, s, r):
        self.E_idle = i
        self.E_standby = s
        self.E_ref = r


def set_init_data():
    this_building = None
    this_server = DTServer()
    config = configparser.ConfigParser()
    config.read('config.ini')
    sections = config.sections()

    try:
        for each_section in sections:
            options = config.options(each_section)

            for each_option in options:
                if each_option == "building name":
                    this_building = Buliding(config[each_section][each_option])

                elif each_option == "server ip":
                    this_server.set_ip(config[each_section][each_option])

                elif each_option == "server port":
                    this_server.set_port(config[each_section][each_option])

                elif each_option == "outside call detection":
                    if config[each_section][each_option].upper() == "TRUE":
                        this_building.set_out(True)

                    else:
                        this_building.set_out(False)

                elif each_option == "outside call name":
                    if this_building.outflag is True:
                        this_building.set_outname(config[each_section][each_option])

                elif each_option == "rts data path":
                    this_building.rts_path = config[each_section][each_option]

                elif each_option == "elevator name":
                    this_building.appendEV(config[each_section][each_option])

                elif each_option == "underground floor":
                    this_building.evlist[-1].underground_floor = config[each_section][each_option]

                elif each_option == "ground floor":
                    this_building.evlist[-1].ground_floor = config[each_section][each_option]

                elif each_option == "each floor altimeter":
                    this_building.evlist[-1].altimeter_list = ast.literal_eval(config[each_section][each_option])

                elif each_option == "use physical system information check":
                    if config[each_section][each_option].upper() == "TRUE":
                        this_building.evlist[-1].physical_flag = True

                elif each_option == "physical data path" and this_building.evlist[-1].physical_flag is True and config[each_section][each_option] != "None":
                    this_building.evlist[-1].physical_path = config[each_section][each_option]

                elif each_option == "acceleration":
                    this_building.evlist[-1].acceleration = config[each_section][each_option]

                elif each_option == "max velocity":
                    this_building.evlist[-1].max_velocity = config[each_section][each_option]

                elif each_option == "energy consumption check":
                    if config[each_section][each_option].upper() == "TRUE":
                        this_building.evlist[-1].energy_flag = True

                    else:
                        this_building.evlist[-1].energy_flag = False

                elif each_option == "idle energy" and this_building.evlist[-1].energy_flag is True:
                    this_building.evlist[-1].E_idle = config[each_section][each_option]

                elif each_option == "standby energy" and this_building.evlist[-1].energy_flag is True:
                    this_building.evlist[-1].E_standby = config[each_section][each_option]

                elif each_option == "ref energy" and this_building.evlist[-1].energy_flag is True:
                    this_building.evlist[-1].E_ref = config[each_section][each_option]

    except OSError as err:
        print("OS error:", err)
    except ValueError:
        print(f"Could not convert data to an integer. {ValueError}")
    except Exception as err:
        print(f"Unexpected {err=}, {type(err)=}")
        raise

    return this_server, this_building

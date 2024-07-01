import datetime
import init
import Altimeter_Sensor
import Camera_Sensor
import Detect_Button_Elevator_Inside
def get_sensor_datas(init_data):
    building_name = init_data.building_name
    elevator_name = init_data.evlist[0].ev_name

    header = {
        rf"{elevator_name}": []
    }

    altimeter, temperature = Altimeter_Sensor.get_data()
    picture_output_path = Camera_Sensor.get_data()

    if picture_output_path is None:
        return None

    else:
        button_detected_elevator_inside = Detect_Button_Elevator_Inside.get_data(picture_output_path, debug=1)

        sensor_data = {
            "building_name": building_name,
            "device_name": elevator_name,
            "underground_floor": init_data.evlist[0].underground_floor,
            "ground_floor": init_data.evlist[0].ground_floor,
            "each_floor_altimeter": init_data.evlist[0].altimeter_list,
            "timestamp": datetime.datetime.now().isoformat(),
            "velocity": None,
            "altimeter": altimeter,
            "temperature": temperature,
            "button_inside": button_detected_elevator_inside,
            "button_outside": None,
            "E_Idle": None,
            "E_Standby": None,
            "E_Ref": None
        }

        if init_data.evlist[0].energy_flag is True:
            sensor_data['E_Idle'] = init_data.evlist[0].E_idle
            sensor_data['E_Standby'] = init_data.evlist[0].E_standby
            sensor_data['E_Ref'] = init_data.evlist[0].E_ref


        return sensor_data

if __name__ == '__main__':
    init_data = init.set_init_data()
    data = get_sensor_datas(init_data)
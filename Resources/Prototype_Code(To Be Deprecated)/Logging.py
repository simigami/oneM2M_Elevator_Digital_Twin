import os

try:
    from config_default import Config_DefaultPath

except Exception as e:
    pass
def log_green_button(config, previous_green_button_list, now_green_button_list, start_timestamp):
    if Config_DefaultPath.log_default_path is None:
        print("Error in Logging, Log Default Path is None\n")
        exit(1)

    else:
        txt_name = rf"Log_Condition_{start_timestamp}.txt"
        basename = os.path.basename(config.Detection_path['image_file_path'])

        os.chdir(Config_DefaultPath.log_default_path)
        if not previous_green_button_list == now_green_button_list:
            log = "Condition Changed at {}\nPrevious Condition was : {}\nLatest Condition is : {}\n\n".format(basename, previous_green_button_list, now_green_button_list)

            with open(txt_name, "a") as f:
                f.write(log)
            return now_green_button_list

        else:
            return None

def log_interval(interval, start_timestamp):
    if Config_DefaultPath.log_default_path is None:
        print("Error in Logging, Log Default Path is None\n")
        exit(1)

    else:
        os.chdir(Config_DefaultPath.log_default_path)
        txt_name = rf"Log_Floor_Lists_{start_timestamp}.txt"
        log = "Interval Between Previous Conditional Change is : {}s\n\n".format(interval)
        with open(txt_name, "a") as f:
            f.write(log)
        
def log_timelist(Text, start_timestamp):
    if Config_DefaultPath.log_default_path is None:
        print("Error in Logging, Log Default Path is None\n")
        exit(1)
    else:
        os.chdir(Config_DefaultPath.log_default_path)
        txt_name = rf"Log_Floor_Lists_{start_timestamp}.txt"
        with open(txt_name, "a") as f:
            f.write(Text)
    
def log_sensor(Text):
    if Config_DefaultPath.log_default_path is None:
        print("Error in Logging, Log Default Path is None\n")
        exit(1)
    
    else:
        os.chdir(Config_DefaultPath.log_default_path)
        txt_name = "Log_Sensors.txt"
        with open(txt_name, "a") as f:
            f.write(Text)

import os
import shutil
from config import TEST_PATH

def make_result_specific_folders():
	dir_name = "Configs"
	if not os.path.exists(dir_name):
		os.mkdir(dir_name)

	dir_name = "Videos"
	if not os.path.exists(dir_name):
		os.mkdir(dir_name)

	dir_name = "Logs"
	if not os.path.exists(dir_name):
		os.mkdir(dir_name)

	dir_name = "Pictures"
	if not os.path.exists(dir_name):
		os.mkdir(dir_name)


def copy_config_files():
	current = os.getcwd()

	dir_name = "No" + str(TEST_PATH.This_Elevator_Number)

	if TEST_PATH.os_name == "Linux":
		os.chdir(TEST_PATH.Installation_Location_Linux)
		os.chdir('../config')

	else:	
		os.chdir(TEST_PATH.Installation_Location_windows)
		os.chdir('../config')

	label_txt_name = fr"label_{TEST_PATH.This_Elevator_Number_str}.txt"
	yaml_name = "data.yaml"
	log_sensor_txt_name = "Log_Sensors.txt"

	if TEST_PATH.os_name == "Linux":
		destination = TEST_PATH.Configs_Folder_Location_Linux
		
	else:	
		destination = TEST_PATH.Configs_Folder_Location_windows
	
	shutil.copy(label_txt_name, destination)
	shutil.copy(yaml_name, destination)
	shutil.copy(log_sensor_txt_name, destination)

	if TEST_PATH.os_name == "Linux":
		TEST_PATH.label_linux = fr"{TEST_PATH.Configs_Folder_Location_Linux}/{label_txt_name}"
		TEST_PATH.yaml_linux = fr"{TEST_PATH.Configs_Folder_Location_Linux}/{yaml_name}"
		
	else:
		TEST_PATH.label_windows = fr"{TEST_PATH.Configs_Folder_Location_windows}/{label_txt_name}"
		TEST_PATH.yaml_windows = fr"{TEST_PATH.Configs_Folder_Location_windows}/{yaml_name}"

	os.chdir(current)

def make_dirs_for_program(debug=0):
	Installation_Directory = os.path.dirname(os.path.realpath(__file__))
	if TEST_PATH.os_name == "Linux":
		TEST_PATH.Installation_Location_Linux = Installation_Directory
		
	else:
		TEST_PATH.Installation_Location_windows = Installation_Directory


	#os.chdir('../..') This is a real code when test is finished
	os.chdir('../../../')

	Result_Folder_Location = "Elevator_Results"
	if not os.path.exists(Result_Folder_Location):
		os.mkdir(Result_Folder_Location)
	
	os.chdir(Result_Folder_Location)
	if TEST_PATH.os_name == "Linux":
		TEST_PATH.Result_Folder_Location_Linux = os.getcwd()
		
	else:
		TEST_PATH.Result_Folder_Location_windows = os.getcwd()
	
	
	dir_name = "No" + str(TEST_PATH.This_Elevator_Number)
	
	if not os.path.exists(dir_name):
		for i in range(TEST_PATH.Number_of_Elevators):
			Number = i+1
			dir_name = "No" + str(Number)
			if not os.path.exists(dir_name):
				os.mkdir(dir_name)
				os.chdir(dir_name)
				make_result_specific_folders()
				os.chdir('../')

			if Number == TEST_PATH.This_Elevator_Number and TEST_PATH.os_name == "Linux":
				TEST_PATH.This_Elevator_Number_str = dir_name

				os.chdir(dir_name)
				TEST_PATH.Specific_Result_Folder_Location_Linux = os.getcwd()

				dir_name = "Configs"
				TEST_PATH.Configs_Folder_Location_Linux = os.path.join(os.getcwd(), dir_name)
				copy_config_files()

				dir_name = "Videos"
				TEST_PATH.Videos_Folder_Location_Linux = os.path.join(os.getcwd(), dir_name)

				dir_name = "Logs"
				TEST_PATH.Logs_Folder_Location_Linux = os.path.join(os.getcwd(), dir_name)

				dir_name = "Pictures"
				TEST_PATH.Pictures_Folder_Location_Linux = os.path.join(os.getcwd(), dir_name)
				os.chdir(TEST_PATH.Result_Folder_Location_Linux)
				
			if Number == TEST_PATH.This_Elevator_Number and TEST_PATH.os_name != "Linux":
				TEST_PATH.This_Elevator_Number_str = dir_name

				os.chdir(dir_name)
				TEST_PATH.Specific_Result_Folder_Location_windows = os.getcwd()

				dir_name = "Configs"
				TEST_PATH.Configs_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)
				copy_config_files()

				dir_name = "Videos"
				TEST_PATH.Videos_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

				dir_name = "Logs"
				TEST_PATH.Logs_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

				dir_name = "Pictures"
				TEST_PATH.Pictures_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)
				os.chdir(TEST_PATH.Result_Folder_Location_windows)

	else:
		if TEST_PATH.os_name == "Linux":
			TEST_PATH.This_Elevator_Number_str = dir_name

			os.chdir(dir_name)
			TEST_PATH.Specific_Result_Folder_Location_Linux = os.getcwd()

			dir_name = "Configs"
			TEST_PATH.Configs_Folder_Location_Linux = os.path.join(os.getcwd(), dir_name)
			copy_config_files()

			dir_name = "Videos"
			TEST_PATH.Videos_Folder_Location_Linux = os.path.join(os.getcwd(), dir_name)

			dir_name = "Logs"
			TEST_PATH.Logs_Folder_Location_Linux = os.path.join(os.getcwd(), dir_name)

			dir_name = "Pictures"
			TEST_PATH.Pictures_Folder_Location_Linux = os.path.join(os.getcwd(), dir_name)
		
		else:
			TEST_PATH.This_Elevator_Number_str = dir_name

			os.chdir(dir_name)
			TEST_PATH.Specific_Result_Folder_Location_windows = os.getcwd()

			dir_name = "Configs"
			TEST_PATH.Configs_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)
			copy_config_files()

			dir_name = "Videos"
			TEST_PATH.Videos_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

			dir_name = "Logs"
			TEST_PATH.Logs_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

			dir_name = "Pictures"
			TEST_PATH.Pictures_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

	if(debug):
		if TEST_PATH.os_name == "Linux":
			print(TEST_PATH.Specific_Result_Folder_Location_Linux)
			print(TEST_PATH.Configs_Folder_Location_Linux)
			print(TEST_PATH.Videos_Folder_Location_Linux)
			print(TEST_PATH.Logs_Folder_Location_Linux)
			print(TEST_PATH.Pictures_Folder_Location_Linux)
			print(TEST_PATH.label_linux)
			print(TEST_PATH.yaml_linux)

		else:
			print(TEST_PATH.Specific_Result_Folder_Location_windows)
			print(TEST_PATH.Configs_Folder_Location_windows)
			print(TEST_PATH.Videos_Folder_Location_windows)
			print(TEST_PATH.Logs_Folder_Location_windows)
			print(TEST_PATH.Pictures_Folder_Location_windows)
			print(TEST_PATH.label_windows)
			print(TEST_PATH.yaml_windows)

def Reset_Path_Based_On_No(No):
	if TEST_PATH.os_name == "Linux":
		if os.path.exists(TEST_PATH.Result_Folder_Location_Linux):
			TEST_PATH.This_Elevator_Number_str = No

			os.chdir(TEST_PATH.Result_Folder_Location_Linux)
			os.chdir(No)

			TEST_PATH.Specific_Result_Folder_Location_Linux = os.getcwd()

			dir_name = "Configs"
			TEST_PATH.Configs_Folder_Location_Linux = os.path.join(os.getcwd(), dir_name)
			copy_config_files()

			dir_name = "Videos"
			TEST_PATH.Videos_Folder_Location_Linux = os.path.join(os.getcwd(), dir_name)

			dir_name = "Logs"
			TEST_PATH.Logs_Folder_Location_Linux = os.path.join(os.getcwd(), dir_name)

			dir_name = "Pictures"
			TEST_PATH.Pictures_Folder_Location_Linux = os.path.join(os.getcwd(), dir_name)
			os.chdir(TEST_PATH.Result_Folder_Location_Linux)

	else:
		if os.path.exists(TEST_PATH.Result_Folder_Location_windows):
			TEST_PATH.This_Elevator_Number_str = No

			os.chdir(TEST_PATH.Result_Folder_Location_windows)
			os.chdir(No)

			TEST_PATH.Specific_Result_Folder_Location_windows = os.getcwd()

			dir_name = "Configs"
			TEST_PATH.Configs_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)
			copy_config_files()

			dir_name = "Videos"
			TEST_PATH.Videos_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

			dir_name = "Logs"
			TEST_PATH.Logs_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

			dir_name = "Pictures"
			TEST_PATH.Pictures_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)
			os.chdir(TEST_PATH.Result_Folder_Location_windows)


if __name__ == '__main__':
	make_dirs_for_program()

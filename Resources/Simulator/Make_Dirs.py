import os
import logging
import shutil
import glob
import re
from config import TEST_PATH

# def check_this_video_path(Raspi_Number, timestamp_str):
# 	if not os.path.exists(Config_DefaultPath.folder_default_path):
# 		return False
# 	else:
# 		os.chdir(Config_DefaultPath.folder_default_path)
# 		if not os.path.exists('Pictures'):
# 			return False
# 		else:
# 			os.chdir('Pictures')
# 			if not os.path.exists(Raspi_Number):
# 				return False
# 			else:
# 				os.chdir(Raspi_Number)
# 				if not os.path.exists(timestamp_str):
# 					return False
# 				else:
# 					return True
#
# def video_list_from_folder(folder_path):
#     if not os.path.exists(folder_path):
#         print("Error in Video_To_Image, video folder not found")
#         exit(1)
#     else:
#         target = os.path.join(folder_path, '**/*.h264')
#         video_list = glob.glob(target, recursive=True)
#
#         if video_list is None:
#             print("Error in Video_To_Image, video list is None")
#             return None
#         else:
#             return video_list
#
# def get_Raspi_Number(video_path):
# 	if not os.path.exists(video_path):
# 		print("Error in Video_To_Image, video_path not found")
# 		exit(1)
# 	else:
# 		pattern = r"(No\d+)_(\d{14})+\.h264"
#
# 		match = re.search(pattern, video_path)
# 		if match:
# 			Raspi_Number = match.group(1)
# 			start_timestamp = match.group(2)
# 			return Raspi_Number, start_timestamp
# 		else:
# 			print("Error in Video_To_Image, Raspi Number is None. Wrong Video name")
# 			return None

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


def make_dirs_for_program(debug=0):
	Installation_Directory = os.path.dirname(os.path.realpath(__file__))
	TEST_PATH.Installation_Location_windows = Installation_Directory

	#os.chdir('../..') This is a real code when test is finished
	os.chdir('../../../')

	Result_Folder_Location = "Elevator_Results_Test"
	if not os.path.exists(Result_Folder_Location):
		os.mkdir(Result_Folder_Location)
	
	os.chdir(Result_Folder_Location)
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

			if Number == TEST_PATH.This_Elevator_Number:
				TEST_PATH.This_Elevator_Number_str = dir_name

				os.chdir(dir_name)
				TEST_PATH.Specific_Result_Folder_Location_windows = os.getcwd()

				dir_name = "Configs"
				TEST_PATH.Configs_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

				dir_name = "Videos"
				TEST_PATH.Videos_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

				dir_name = "Logs"
				TEST_PATH.Logs_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

				dir_name = "Pictures"
				TEST_PATH.Pictures_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)
				os.chdir(TEST_PATH.Result_Folder_Location_windows)
	else:
		TEST_PATH.This_Elevator_Number_str = dir_name

		os.chdir(dir_name)
		TEST_PATH.Specific_Result_Folder_Location_windows = os.getcwd()

		dir_name = "Configs"
		TEST_PATH.Configs_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

		dir_name = "Videos"
		TEST_PATH.Videos_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

		dir_name = "Logs"
		TEST_PATH.Logs_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

		dir_name = "Pictures"
		TEST_PATH.Pictures_Folder_Location_windows = os.path.join(os.getcwd(), dir_name)

	if(debug):
		print(TEST_PATH.Specific_Result_Folder_Location_windows)
		print(TEST_PATH.Configs_Folder_Location_windows)
		print(TEST_PATH.Videos_Folder_Location_windows)
		print(TEST_PATH.Logs_Folder_Location_windows)
		print(TEST_PATH.Pictures_Folder_Location_windows)



if __name__ == '__main__':
	# Raspi_Number = "No4"
	# make_dir_and_files_Linux()
	# make_dir_and_files_Windows(Raspi_Number)
	#
	# folder_path = rf"E:\ML\Elevator Git\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\Videos"
	# l = video_list_from_folder(folder_path)
	# for path in l:
	# 	get_Raspi_Number(path)
	make_dirs_for_program()

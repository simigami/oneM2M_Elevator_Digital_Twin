import random

class Config_Elevator_Specification:
	def __init__(self):
		self.capacity = None
		self.seater = None
		self.max_velocity = None
		self.motor_capacity = None
		self.transformer_capacity = None
		self.ELCB = None
		self.lighting_energy = None
		self.ventilation_energy = None

		self.trip_category = None
		self.door_open_close_time = None
		self.travel_distance = None
		self.average_travel_distance = None
		self.average_travel_distance_time = None

		self.ref_cycle_energy = None
		self.short_cycle_energy = None
		self.short_cycle_distance = None
		self.ref_cycle_distance = None

		self.idle_energy = None
		self.standby_5min_energy = None
		self.standby_30min_energy = None

		self.VT_variable = [0 for i in range(3)]  # each element is (a,b,c) in V = at^2+bt+c
		self.velocity = None  # m/s
		self.acceleration = None  # m/s2
		self.jerk = None  # m/s3
		self.Q = None  # Average car load (ISO-21745(2))
		self.S = None  # Percentage of average travel distance	(ISO-21745(2))
		self.kl = None  # load factor	(ISO-21745(2))
		self.trip = None  # Daily Trip Count
		self.nd = None  # Categorized number of starts per day

		self.Pid = None  # IDLE mode Wattage(W)
		self.Pst = None  # Stanby mode Wattage(W)
		self.Rid = None  # IDLE/Standby Time Ratio
		self.Rst = None  # IDLE/Standby Time Ratio

	def cal_vel(self, VT_variable, t):
		return (VT_variable[0]*t**2)+(VT_variable[1]*t)+VT_variable[2]

	def cal_acc(self, VT_variable, t):
		return (VT_variable[0]*t*2)+VT_variable[1]

	def cal_jerk(self, VT_variable, t):
		return (VT_variable[0]**2)

	def set_VT(self, a, b, c):
		self.VT_variable[0] = a
		self.VT_variable[1] = b
		self.VT_variable[2] = c

	def set_VAJ(self, t):
		self.velocity = self.cal_vel(self.VT_variable, t)  # m/s
		self.acceleration = self.cal_vel(self.VT_variable, t)  # m/s
		self.jerk = self.cal_vel(self.VT_variable, t)  # m/s
	def cal_acl(self, VT_variable, t):
		return (2*VT_variable[0]*t)+VT_variable[1]
	
	def cal_jerk(self, VT_variable):
		return 2*VT_variable[0]
	
	def Cal_Trip_Category(self, Trip):
		if Trip<=75:
			return 1
		elif Trip<=200:
			return 2
		elif Trip<=500:
			return 3
		elif Trip<=1000:
			return 4
		elif Trip<=2000:
			return 5
		else:
			return 6

	def Cal_Average_Trip_Hieght_Ratio(self, trip, trip_category):
		if trip<=2:
			return 1.0
		elif trip==3:
			return 0.67
		else:
			if 1<=trip_category<=3:
				return 0.49
			elif trip_category==4:
				return 0.44
			elif trip_category==5:
				return 0.39
			elif trip_category==6:
				return 0.32
	def rated_load(self, trip_category, Load):
		if trip_category >= 1 and trip_category <= 3:
			if Load <= 800:
				return 7.5
			elif Load <= 1275:
				return 4.5
			elif Load <= 2000:
				return 3.0
			else:
				return 2.0
		elif trip_category == 4:
			if Load <= 800:
				return 9.0
			elif Load <= 1275:
				return 6.0
			elif Load <= 2000:
				return 3.5
			else:
				return 2.2
		elif trip_category == 5:
			if Load <= 800:
				return 16.0
			elif Load <= 1275:
				return 11.0
			elif Load <= 2000:
				return 7.0
			else:
				return 4.5

	def load_factor(self, Q, cb=0.5, h=0):
		if (cb == 0.5):
			return 1 - (Q * 0.0164)
		elif (cb == 0.4):
			return 1 - (Q * 0.0192)
		elif cb == 0 and h == 1:
			return 1 + (Q * 0.0071)
		elif cb == 0.35 and h == 1:
			return 1 + (Q * 0.0100)
		elif cb == 0.7 and h == 1:
			return 1 + (Q * 0.0187)

	def Cal_idle_standby_ratio(self, trip_category):
		if trip_category==1:
			return 0.13, 0.55, 0.32
		elif trip_category==2:
			return 0.23, 0.45, 0.32
		elif trip_category==3:
			return 0.36, 0.31, 0.33
		elif trip_category==4:
			return 0.45, 0.19, 0.36
		else:
			return 0.42, 0.18, 0.41
	def Cal_Running_Energy_Per_day(self, nd, S, kl, Erc):
		# print(nd)
		# print(S)
		# print(kl)
		# print(Erc)
		return (nd*S*kl*Erc)/2

	def Cal_Standing_Energy_Per_day(self, nd, tav, Pid, Rid, Pst, Rst):
		return (24 - (nd / 3600) * tav)(Pid * Rid + Pst * Rst)

	def Cal_Running_Energy_Per_Meter(self, ref_cycle_energy, short_cycle_energy, ref_cycle_height, short_cycle_height):
		return 0.5*((ref_cycle_energy-short_cycle_energy)/(ref_cycle_height-short_cycle_height))

	def Cal_start_stop_Energy(self, ref_cycle_energy, running_energy_per_meter, ref_cycle_height):
		return 0.5*(ref_cycle_energy-2*running_energy_per_meter*ref_cycle_height)

	def Cal_average_cycle_Energy(self, running_energy_per_meter, sav, start_stop_energy):
		return (2*running_energy_per_meter*sav)+start_stop_energy

	def Cal_Average_Travel_Distance_Time(self, door_open_close_time, average_travel_distance, velocity, acceleration, jerk):
		return (average_travel_distance/velocity)+(velocity/acceleration)+(acceleration/jerk)+door_open_close_time
	def Cal_Average_running_time(self, trip, average_travel_distance_time):
		return trip * (average_travel_distance_time/3600)

	def Cal_Average_non_running_time(self, average_running_time):
		return 24-average_running_time

	def Cal_Daily_Running_Energy(self, kl, trip, average_cycle_energy):
		return (kl*trip*average_cycle_energy)/2
	def Cal_Daily_non_running_Energy(self, trip_category, Average_non_running_time, idle_energy, standby_5min_energy, standby_30min_energy):
		rid, rst5, rst30 = self.Cal_idle_standby_ratio(trip_category)
		return (Average_non_running_time/100)*(idle_energy*rid+standby_5min_energy*rst5+standby_30min_energy*rst30)

	def Cal_Total_Energy_Per_Day(self, running, non_runnig):
		return running+non_runnig

class Hyundai_Luxen:
	def __init__(self):
		self.Luxen = Config_Elevator_Specification()
		self.Luxen.capacity = 1350
		self.Luxen.Seater = 20
		self.Luxen.Max_Velocity = 2.5
		self.Luxen.Motor_Capacity = 24.6
		self.Luxen.Transformer_Capacity = 37.0
		self.Luxen.ELCB = 75

		self.Luxen.set_VT(1.0, 0.5, 1.0)	# This is random variable that satisfies v=2.5 when t=1.0
		self.Luxen.set_VAJ(1.0)

		self.Luxen.ref_cycle_energy = 170	# This Value is important
		self.Luxen.ref_cycle_distance = 100.0
		self.Luxen.door_open_close_time = 6.0

	def set_trip(self, trip):
		self.Luxen.trip = trip
	def set_trip_category(self):
		self.Luxen.trip_category = self.Luxen.Cal_Trip_Category(self.Luxen.trip)

	def set_average_travel_distance(self, value):
		self.Luxen.average_travel_distance = value

	def set_S(self):
		self.Luxen.S = self.Luxen.average_travel_distance / self.Luxen.ref_cycle_distance

	def set_Q(self):
		self.Luxen.Q  = self.Luxen.rated_load(self.Luxen.trip_category, self.Luxen.capacity)

	def set_kl(self):
		self.Luxen.kl = self.Luxen.load_factor(self.Luxen.Q)
	def set_enerygy(self, wid, wst5, wst30):
		self.Luxen.idle_energy = wid
		self.Luxen.standby_5min_energy = wst5
		self.Luxen.standby_30min_energy = wst30

if __name__ == '__main__':
	Testcase = [[394, 246, 157],[394.0, 246.1, 157.5], [2390.8, 139.8, 89.5], [1318.0, 127.9, 95.9], [1413.2, 211.5, 152.2], [2438.5, 117.8, 81.3], [1338.5, 118.3, 94.7], [341.4, 167.7, 119.1], [407.5, 264.8, 174.8], [278.0, 158.5, 112.5], [398.5, 258.1, 170.3], [1334.9, 252.6, 181.6], [2576.9, 1288.5, 811.7], [1565.0, 370.8, 248.4], [2297.7, 1122.5, 684.7], [1649.8, 488.4, 317.4], [833.5, 649.5, 357.2], [416.3, 264.4, 169.2], [2269.6, 1091.1, 741.9], [424.2, 118.7, 84.3], [208.0, 120.1, 81.7], [2165.1, 895.7, 573.3]]
	Elevator = Hyundai_Luxen()

	for energy in Testcase:
		Elevator.set_trip(random.randint(750,1000))
		Elevator.set_trip_category()

		wid, wst5, wst30 = energy
		Elevator.set_enerygy(wid, wst5, wst30)
		Elevator.set_average_travel_distance(round(random.uniform(30.0,45.0), 3))
		Elevator.set_S()
		Elevator.set_Q()
		Elevator.set_kl()

		running = Elevator.Luxen.Cal_Running_Energy_Per_day(Elevator.Luxen.trip, Elevator.Luxen.S, Elevator.Luxen.kl, Elevator.Luxen.ref_cycle_energy)

		average_travel_distance_time = Elevator.Luxen.Cal_Average_Travel_Distance_Time(Elevator.Luxen.door_open_close_time, Elevator.Luxen.average_travel_distance, Elevator.Luxen.velocity, Elevator.Luxen.acceleration, Elevator.Luxen.jerk)
		running_time = Elevator.Luxen.Cal_Average_running_time(Elevator.Luxen.trip, average_travel_distance_time)
		non_running_time = Elevator.Luxen.Cal_Average_non_running_time(running_time)

		non_running = Elevator.Luxen.Cal_Daily_non_running_Energy(Elevator.Luxen.trip_category, non_running_time, Elevator.Luxen.idle_energy, Elevator.Luxen.standby_5min_energy, Elevator.Luxen.standby_30min_energy)
		total_energy = Elevator.Luxen.Cal_Total_Energy_Per_Day(running, non_running)

		running /= 1000
		running = round(running, 3)

		non_running /= 1000
		non_running = round(non_running, 3)

		total_energy /= 1000
		total_energy = round(total_energy, 3)

		print("Day -> Running : {}kWh, Non-Running : {}kWh, Total Energy : {}kWh".format(running, non_running, total_energy))
		print("Annual -> Running : {}kWh, Non-Running : {}kWh, Total Energy : {}kWh\n".format(running*0.8*365, non_running*0.8*365, total_energy*0.8*365))
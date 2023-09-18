class Config_Elevator_Specification:
<<<<<<< HEAD
	def __init__(self):
		self.capacity = None
		self.seater = None
		self.max_velocity = None
		self.motor_capacity = None
		self.transformer_capacity = None
		self.ELCB = None

		self.VT_variable = [0 for i in range(3)]  # each element is (a,b,c) in V = at^2+bt+c

		self.travel_distance = None
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
=======
	Capacity = None
	Seater = None
	Max_Velocity = None
	Motor_Capacity = None
	Transformer_Capacity = None
	ELCB = None

	VT_variable = [0 for i in range(3)]  # each element is (a,b,c) in V = at^2+bt+c

	Travel_Distance = None
	Velocitiy = None  # m/s
	Acceleration = None  # m/s2
	Jerk = None  # m/s3
	Q = None  # Average car load (ISO-21745(2))
	S = None  # Percentage of average travel distance	(ISO-21745(2))
	kl = None  # load factor	(ISO-21745(2))
	Trip = None  # Daily Trip Count
	nd = None  # Categorized number of starts per day

	Pid = None  # IDLE mode Wattage(W)
	Pst = None  # Stanby mode Wattage(W)
	Rid = None  # IDLE/Standby Time Ratio

	Rst = None  # IDLE/Standby Time Ratio
	def cal_vel(self, VT_variable, t):
		self.Velocitiy =  (VT_variable[0]*t**2)+(VT_variable[1]*t)+VT_variable[2]
>>>>>>> 9d76d2498e4760cbce01d1e67b9cfaa4c8a45458

	def set_VT(self, a, b, c):
		self.VT_variable[0] = a
		self.VT_variable[1] = b
		self.VT_variable[2] = c
	
	def cal_acl(self, VT_variable, t):
		return (2*VT_variable[0]*t)+VT_variable[1]
	
	def cal_jerk(self, VT_variable):
		return 2*VT_variable[0]
	
	def cal_Ratio(self, Trip):
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
	
	def Cal_Running_Energy_Per_day(self, nd, S, kl, Erc):
		return (nd*S*kl*Erc)/2

<<<<<<< HEAD
	def Cal_Standing_Energy_Per_day(self, nd, tav, Pid, Rid, Pst, Rst):
		return (24 - (nd / 3600) * tav)(Pid * Rid + Pst * Rst)

	def rated_load(self, Category, Load):
=======
		def Cal_Standing_Energy_Per_day(self, nd, tav, Pid, Rid, Pst, Rst):
			return (24 - (nd / 3600) * tav)(Pid * Rid + Pst * Rst)

	def rated_load(Category, Load):
>>>>>>> 9d76d2498e4760cbce01d1e67b9cfaa4c8a45458
		if Category >= 1 and Category <= 3:
			if Load <= 800:
				return 7.5
			elif Load <= 1275:
				return 4.5
			elif Load <= 2000:
				return 3.0
			else:
				return 2.0
		elif Category == 4:
			if Load <= 800:
				return 9.0
			elif Load <= 1275:
				return 6.0
			elif Load <= 2000:
				return 3.5
			else:
				return 2.2
		elif Category == 5:
			if Load <= 800:
				return 16.0
			elif Load <= 1275:
				return 11.0
			elif Load <= 2000:
				return 7.0
			else:
				return 4.5

<<<<<<< HEAD
	def load_factor(self, Q, cb=0.5, h=0):
=======
	def load_factor(Q, cb=0.5, h=0):
>>>>>>> 9d76d2498e4760cbce01d1e67b9cfaa4c8a45458
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

class Hyundai_Luxen:
<<<<<<< HEAD
	def __init__(self):
		Luxen = Config_Elevator_Specification()
		Luxen.Capacity = 1350
		Luxen.Seater = 20
		Luxen.Max_Velocity = 2.5
		Luxen.Motor_Capacity = 24.6
		Luxen.Transformer_Capacity = 37.0
		Luxen.ELCB = 75
		Luxen.set_VT(1.0, 0.5, 1.0)	# This is random variable that satisfies v=2.5 when t=1.0

if __name__ == '__main__':
	luxen = Hyundai_Luxen()
=======
	Luxen = Config_Elevator_Specification()
	Luxen.Capacity = 1350
	Luxen.Seater = 20
	Luxen.Max_Velocity = 2.5
	Luxen.Motor_Capacity = 24.6
	Luxen.Transformer_Capacity = 37.0
	Luxen.ELCB = 75
	Luxen.set_VT_variable(1.0, 0.5, 1.0)	# This is random variable that satisfies v=2.5 when t=1.0
>>>>>>> 9d76d2498e4760cbce01d1e67b9cfaa4c8a45458

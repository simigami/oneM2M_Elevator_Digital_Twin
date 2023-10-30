import random

from config import TEST_VARIABLES

class values:
	def __init__(self):
		self.cb = None
		self.h = None
		self.max_load = None
		self.trip = None
		self.ref_cycle_energy = None
		self.short_cycle_energy = None
		self.ref_cycle_distance = None
		self.short_cycle_distance = None

	def set_cb(self, cb):
		self.cb = cb

	def set_h(self, h):
		self.h = h
	def set_max_load(self, max_load):
		self.max_load = max_load
	def set_trip(self, trip):
		self.trip = trip
	def set_ref_cycle_energy(self, ref_cycle_energy):
		self.ref_cycle_energy = ref_cycle_energy
	def set_short_cycle_energy(self, short_cycle_energy):
		self.short_cycle_energy = short_cycle_energy
	def set_ref_cycle_distance(self, ref_cycle_distance):
		self.ref_cycle_distance = ref_cycle_distance
	def set_short_cycle_distance(self, short_cycle_distance):
		self.short_cycle_distance = short_cycle_distance

class Config_Elevator_Specification:
	def __init__(self):
		self.cb = None
		self.h = None

		self.idle_0 = None
		self.idle_5 = None
		self.idle_30 = None

		self.standby_5 = None
		self.standby_30 = None

		self.max_load = None
		self.Q = None
		self.load_factor = None

		self.velocity = None
		self.acceleration = None
		self.jerk = None

		self.trip = None

		self.ref_cycle_energy = None
		self.ref_cycle_distance = None

		self.short_cycle_energy = None
		self.short_cycle_distance = None

		self.category = None
		self.average_distance_percentage = None

		self.Trd = None
		self.Tnr = None

		self.Sav = None

		self.Erm = None
		self.Essc = None
		self.Erav = None
		self.Erd = None
		self.Enr = None

	def set_values(self, values):
		self.cb = values.cb
		self.h = values.h
		self.max_load = values.max_load
		self.trip = values.trip

		self.ref_cycle_energy = values.ref_cycle_energy
		self.short_cycle_energy = values.short_cycle_energy
		self.ref_cycle_distance = values.ref_cycle_distance
		self.short_cycle_distance = values.short_cycle_distance

	def get_energy(self):
		if self.trip <= 75:
			self.category = 1
		elif self.trip <= 200:
			self.category = 2
		elif self.trip <= 500:
			self.category = 3
		elif self.trip <= 1000:
			self.category = 4
		elif self.trip <= 2000:
			self.category = 5
		else:
			self.category = 6

		if TEST_VARIABLES.total_floors <= 2:
			self.Sav = 1.0
		elif TEST_VARIABLES.total_floors == 3:
			self.Sav =  0.67
		else:
			if 1 <= self.category <= 3:
				self.Sav = 0.49
			elif self.category == 4:
				self.Sav = 0.44
			elif self.category == 5:
				self.Sav = 0.39
			elif self.category == 6:
				self.Sav = 0.32

		self.Erm = 0.5 * ((self.ref_cycle_energy-self.short_cycle_energy)/(self.ref_cycle_distance-self.short_cycle_distance))
		self.Essc = 0.5 * ((self.ref_cycle_energy)-(2 * self.Erm * self.ref_cycle_distance))
		self.Erav = 2 * self.Erm * self.Sav + 2 * self.Essc

		if 1 <= self.category <= 3:
			if self.max_load <= 800:
				self.Q = 0.075
			elif self.max_load <= 1275:
				self.Q = 0.045
			elif self.max_load <= 2000:
				self.Q = 0.03
			else:
				self.Q = 0.02
		elif self.category == 4:
			if self.max_load <= 800:
				self.Q = 0.09
			elif self.max_load <= 1275:
				self.Q = 0.06
			elif self.max_load <= 2000:
				self.Q = 0.035
			else:
				self.Q = 0.022
		elif self.category == 5:
			if self.max_load <= 800:
				self.Q = 0.16
			elif self.max_load <= 1275:
				self.Q = 0.11
			elif self.max_load <= 2000:
				self.Q = 0.07
			else:
				self.Q = 0.045

		if (self.cb == 0.5):
			self.load_factor = 1 - (self.Q * 0.0164)
		elif (self.cb == 0.4):
			self.load_factor = 1 - (self.Q * 0.0192)
		elif self.cb == 0 and self.h == 1:
			self.load_factor = 1 - (self.Q * 0.0071)
		elif self.cb == 0.35 and self.h == 1:
			self.load_factor = 1 - (self.Q * 0.0100)
		elif self.cb == 0.7 and self.h == 1:
			self.load_factor = 1 - (self.Q * 0.0187)

		#self.print_energy()

		self.Erd = 0.5 * (self.load_factor * self.trip * self.Erav)

	def print_energy(self):
		print("capacity : {}\nQ : {}\nload factor : {}\ntrip : {}\nref_cycle_E : {}\nref_cycle_dist : {}\nshort_cycle_E : {}\nshort_cycle_dist : {}\ncategoty : {}\nErm : {}\nEssc : {}\nErav : {}\nErd : {}\n\n".format(
			self.max_load,
			self.Q,
			self.load_factor,
			self.trip,
			self.ref_cycle_energy,
			self.ref_cycle_distance,
			self.short_cycle_energy,
			self.short_cycle_distance,
			self.category,
			self.Erm,
			self.Essc,
			self.Erav,
			self.Erd))

class energy_node():
	def __init__(self):
		self.max_load = None
		self.Q = None
		self.load_factor = None
		self.trip = None
		self.ref_cycle_energy = None
		self.ref_cycle_distance = None
		self.short_cycle_energy = None
		self.short_cycle_distance = None
		self.category = None
		self.Erm = None
		self.Essc = None
		self.Erav = None
		self.Erd = None

	def set_vaule(self, values):
		self.max_load = values.max_load
		self.Q = values.Q
		self.load_factor = values.load_factor
		self.trip = values.trip
		self.ref_cycle_energy = values.ref_cycle_energy
		self.ref_cycle_distance = values.ref_cycle_distance
		self.short_cycle_energy = values.short_cycle_energy
		self.short_cycle_distance = values.short_cycle_distance
		self.category = values.category
		self.Erm = values.Erm
		self.Essc = values.Essc
		self.Erav = values.Erav
		self.Erd = values.Erd

	def print_energy(self):
		print("capacity : {}\nQ : {}\nload factor : {}\ntrip : {}\nref_cycle_E : {}\nref_cycle_dist : {}\nshort_cycle_E : {}\nshort_cycle_dist : {}\ncategoty : {}\nErm : {}\nEssc : {}\nErav : {}\nErd : {}\n\n".format(
			self.max_load,
			self.Q,
			self.load_factor,
			self.trip,
			self.ref_cycle_energy,
			self.ref_cycle_distance,
			self.short_cycle_energy,
			self.short_cycle_distance,
			self.category,
			self.Erm,
			self.Essc,
			self.Erav,
			self.Erd))

if __name__ == '__main__':
	pass
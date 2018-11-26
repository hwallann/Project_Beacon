import random
PSI_ON_SURFACE = 14.22295170521
PSI_PR_MH2O = 1.422295170521

class Swimmer(object):
	"""docstring for ClassName"""
	def __init__(self, x = 0.9, y = -0.7, z = 0.0, pitch = 45.0, yaw = 45.0, roll = 90, sampleFrequency = 10, pressure = PSI_ON_SURFACE):
		self.x = x
		self.y = y
		self.z = z
		self.pitch = pitch
		self.yaw = yaw
		self.roll = roll
		self.sampleFrequency = sampleFrequency
		self.pressure = pressure

		self.drag = 1.4
		self.swimDistance = 1.414

	def __str__(self):
		jx, jy, jz, jpit, jya, jr, jpre = self.jitter_acc()
		jString = '{{"x":{0},"y":{1},"z":{2},"pitch":{3},"yaw":{4},"roll":{5},"pressure":{6}}}'.format(jx,jy,jz,jpit,jya,jr,jpre)
		return jString

	def jitter_acc(self):
		jx = self.__mini_jitter(self.x)
		jy = self.__mini_jitter(self.y)
		jz = self.__mini_jitter(self.z)
		jpit = self.__mini_jitter(self.pitch)
		jya = self.__mini_jitter(self.yaw)
		jr = self.__mini_jitter(self.roll)
		jpre = self.__mini_jitter(self.pressure)
		return jx, jy, jz, jpit, jya, jr, jpre

	def __mini_jitter(self, a):
		return random.gauss(a, 0.005)

	def __create_flip_range(self, lim, n):
		return [lim + i * 2 * - lim / (n - 1) for i in range(int(n))]

	def __ms2ToG(self, m):
		return float(m) / 0.981

	def rest(self, seconds = 3):
		ret_list = list()

		for i in range(seconds * self.sampleFrequency):
			ret_list.append(str(self))
		
		return ret_list

	def dive_land(self):
		dive_seconds = 0.5
		df = dive_seconds * self.sampleFrequency
		ret_list = list()

		arr_p = self.__create_flip_range(self.pitch, df)
		arr_y = self.__create_flip_range(self.y, df)

		for i in range(len(arr_p)):
			self.y = arr_y[i]
			self.pitch = arr_p[i]
			ret_list.append(str(self))

		return ret_list

	def turn_around(self):
		turn_seconds = 3
		tf = turn_seconds * self.sampleFrequency
		ret_list = list()

		arr_y = self.__create_flip_range(self.yaw, tf)

		for i in range(len(arr_y)):
			self.yaw = arr_y[i]
			ret_list.append(str(self))

		return ret_list


	def __calc_accelerations(self, d1, d2, t1, t2):
		v0 = 0
		v2 = 0
		a1 = (v0 * t1) + (2 * d1) / ((float(t1) * float(t1)))
		v1 = v0 + (a1 * t1)
		
		a2 = (float(v2) - float(v1)) / t2

		ab2 = (2*(d2 - (v1 * t2))) / (float(t2) * float(t2))

		print (str(a1) + '\n' + str(a2) + '\n' + str(ab2))

		return a1, a2

	def __diagonal_vertical_line_motion(self, d1, d2, t1, t2):
		f1 = t1 * self.sampleFrequency
		f2 = t2 * self.sampleFrequency

		a1, a2 = self.__calc_accelerations(d1, d2, t1, t2)

		sign = -1 if self.pitch % 360 > 180 else 1

		ppd1 = sign * (float(d1) / self.swimDistance ) / float(f1)
		ppd2 = sign * (float(d2) / self.swimDistance ) / float(f2)

		print("sign: " + str(sign))
		print("pp1: " + str(ppd1))
		print("pp2: " + str(ppd2))
		ch = a1
		print(str(self.y) + '\t' + str(ch))
		self.y += ch
		ret_list = list()
		for i in range(int(f1)):
			self.pressure += ppd1
			ret_list.append(str(self))

		ch = (-a1 + a2)
		print(str(self.y) + '\t' + str(ch))
		self.y += ch

		for i in range(int(f2)):
			self.pressure += ppd2
			ret_list.append(str(self))

		ch = -a2
		print(str(self.y) + '\t' + str(ch))
		self.y += ch

		return ret_list

	def swim(self):
		uppTime = 0.5
		downTime = 2

		dist = self.swimDistance / 2
		ret_list = self.__diagonal_vertical_line_motion(dist, dist, uppTime, downTime)

		'''
		uf = uppTime * self.sampleFrequency
		df = downTime * self.sampleFrequency

		a1, a2 = self.__calc_accelerations(self.swimDistance / 2, self.swimDistance / 2, uppTime, downTime)

		self.y += a1
		ret_list = list()
		for i in range(uf):
			ret_list.append(str(self))

		self.y -= a1 - a2

		for i in range(df):
			ret_list.append(str(self))

		self.y += a2
		'''

		return ret_list

	def kickoff(self):
		kickTime = 1
		driftTime = 4 - kickTime

		dDistance = self.swimDistance * 2
		kDistance = dDistance - self.swimDistance
		

		ret_list = self.__diagonal_vertical_line_motion(kDistance, dDistance, kickTime, driftTime)

		'''
		kf = uppTime * self.sampleFrequency
		df = downTime * self.sampleFrequency

		

		a1, a2 = self.__calc_accelerations(kDistance, dDistance, kickTime, driftTime)

		self.y += a1
		ret_list = list()
		for i in range(uf):
			ret_list.append(str(self))

		self.y -= a1 - a2

		for i in range(df):
			ret_list.append(str(self))

		self.y += a2
		'''

		return ret_list
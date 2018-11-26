import motion as Motion

def print_samples(samples):
	for sample in samples:
		print(sample)

def main ():
	print("Start")
	fakeSwim = Motion.Swimmer()
	print(fakeSwim)

	print("Rest")
	rest = fakeSwim.rest()
	print_samples(rest)

	print("Dive")
	dive = fakeSwim.dive_land()
	print_samples(dive)

	print("Strike")
	stroke = fakeSwim.swim()
	print_samples(stroke)

	print("Strike")
	stroke = fakeSwim.swim()
	print_samples(stroke)

	print("Strike")
	stroke = fakeSwim.swim()
	print_samples(stroke)

	print("Land")
	land = fakeSwim.dive_land()
	print_samples(land)

	print("Kickoff")
	kickoff = fakeSwim.kickoff()
	print_samples(kickoff)

	print("Rest")
	rest = fakeSwim.rest()
	print_samples(rest)

	print("Turn")
	turn = fakeSwim.turn_around()
	print_samples(turn) 




if __name__ == '__main__':
	main()
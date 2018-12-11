# Script to collect deviceTimes from files

import argparse as AP
import os
import numpy as np
from datetime import datetime

def StripAllIdentifiers(deviceTimes):
	newLines = ""
	lines = deviceTimes.split('\n')
	for line in lines:
		dTime = line.split('\t')[1].split(':')
		newTimes += ConvertTimeToMinSecMilliSec(dTime) + '\n'

def StripIdentifiers(identifierString):
	idString = identifierString.strip()
	
	idSplit = idString.split(':')
	idNumber = idSplit[0]
	idNumber = idNumber[:-4]
	idNumber = idNumber[3:]

	return idNumber, idSplit[1]

def ConvertTimeToMinSecMilliSec(deviceTime):
	timeString = deviceTime.split(':')

	for s in timeString:
		if 'null' in s:
			return deviceTime

	tMilli = int(timeString[1])
	tSecond = np.floor(tMilli / 1000)
	tMin = np.floor(tSecond / 60)

	tMilli -= tSecond * 1000
	tSecond -= tMin * 60

	retString = "Min: {2}, Sec: {1}, Milli: {0}\n".format(tMilli, tSecond, tMin)
	return retString


def main(fileName) :
	assert os.path.isfile(fileName) 
	
	fr = open(fileName)

	messageNumber = ""
	deviceTimes = ""
	currentRound = ""
	date = ""
	lineNumber = 0
	for l in fr.readlines():
		lineNumber += 1
		if ("Object" in l) and not ("tcxn" in l):
			messageNumber,_ = StripIdentifiers(l)
		if ("currentRound" in l):
			a,b = StripIdentifiers(l)
			currentRound = a + b
		if ("timestamp" in l):
			a,b = StripIdentifiers(l)
			date = "Date" + datetime.utcfromtimestamp(int(b) / 1000).strftime('%Y-%m-%d %H:%M:%S')
		if "deviceTime" in l:
			l = ConvertTimeToMinSecMilliSec(l)
			deviceTimes += messageNumber + '\t\t' + currentRound + '\t\t' + l.strip('\n') + '\t\t' + date + '\n'

	return (deviceTimes)


if __name__ == '__main__':
	parser = AP.ArgumentParser(description='Parse given file')
	parser.add_argument('--fileName', '-f', type=str,
		help='The name of the file')
	
	args = parser.parse_args()

	print (args)

	deviceTimes = main(args.fileName)

	print (deviceTimes)
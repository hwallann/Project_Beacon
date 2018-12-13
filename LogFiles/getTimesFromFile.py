# Script to collect deviceTimes from files

import json
import os
import argparse as AP
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
	if str(deviceTime) == 'null':
		return deviceTime

	tMilli = int(deviceTime)
	tSecond = np.floor(tMilli / 1000)
	tMin = np.floor(tSecond / 60)

	tMilli -= tSecond * 1000
	tSecond -= tMin * 60

	retString = "Min: {2}, Sec: {1}, Milli: {0}\n".format(tMilli, tSecond, tMin)
	return retString

def ParseAsSorted(file):
	messageNumber = ""
	deviceTimes = ""
	currentRound = ""
	date = ""
	for l in file.readlines():
		if ("Object" in l) and not ("tcxn" in l):
			messageNumber,_ = StripIdentifiers(l)
		if ("currentRound" in l):
			a,b = StripIdentifiers(l)
			currentRound = a + b
		if ("timestamp" in l):
			a,b = StripIdentifiers(l)
			date = "Date" + datetime.utcfromtimestamp(int(b) / 1000).strftime('%Y-%m-%d %H:%M:%S')
		if "deviceTime" in l:
			a,b = StripIdentifiers(l)
			l = ConvertTimeToMinSecMilliSec(b.strip())
			deviceTimes += messageNumber + '\t\t' + currentRound + '\t\t' + l.strip('\n') + '\t\t' + date + '\n'

	return (deviceTimes)

def ParseAsJson(file):
	jfile = json.loads(file.read())
	messageNumber = ""
	deviceTimes = ""
	currentRound = ""
	date = ""
	timeLine = ""
	testSpec = ""
	testType = ""
	lineNumber = 0
	for jObj in jfile:
		lineNumber += 1
		if ("currentRound" in jObj):
			currentRound = jObj["currentRound"]
		if ("timestamp" in jObj):
			timestamp = jObj["timestamp"]
			date = "Date" + datetime.utcfromtimestamp(int(timestamp) / 1000).strftime('%Y-%m-%d %H:%M:%S')
		if "deviceTime" in jObj:
			timeLine = ConvertTimeToMinSecMilliSec(jObj["deviceTime"])
		pl = jObj["payload"]
		
		if "testSpecification" in jObj:
			testSpec = jObj["testSpecification"]

		if "Speed" in pl:
			testType = "Speed"
		elif "Size" in pl:
			testType = "Size"
		else:
			testType = "Error"

		deviceTimes += str(lineNumber) + '\t\tcurrentRound ' + str(currentRound).strip() + '\t\tTestType ' + str(testType).strip() + '\t\tTestSpec ' + str(testSpec).strip() + '\t\t' + str(timeLine).strip() + '\t\t' + str(date).strip() + '\n'

	return (deviceTimes)

def main(fileName, isJSON = False) :
	assert os.path.isfile(fileName) 
	
	fr = open(fileName)

	if isJSON:
		return ParseAsJson(fr)
	else:
		return ParseAsSorted(fr)


if __name__ == '__main__':
	parser = AP.ArgumentParser(description='Parse given file')
	parser.add_argument('--fileName', '-f', type=str,
		help='The name of the file')
	parser.add_argument('--isJSON', '-j', type=bool,
		default=False, help='Should the file in json format?')
	
	args = parser.parse_args()

	print (args)

	deviceTimes = main(args.fileName, args.isJSON)

	print (deviceTimes)
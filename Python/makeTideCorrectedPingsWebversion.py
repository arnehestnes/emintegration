import glob
import os
import sys
import struct
import time
import datetime
import utm
import xml.etree.ElementTree as ET
import requests
import dateutil.parser


times = []
tides = []
noTides = -1

lastidt=0
outfile=0

def openTidefile():
	global times
	global tides
	global noTides
	
	try:
		f = open('kmall_bb.allbb')
	except Exception:
		print('bb not opened')
	line = f.readline()
	f.close()
	toks = line.split()
	if (len(toks) != 6):
		print("Error reading bb")
		sys.exit(0)
	minn = float(toks[0])
	mine = float(toks[1])
	maxn = float(toks[2])
	maxe = float(toks[3])
	mint = int(int(toks[4])/1e3) - (24*60*60)
	maxt = int(int(toks[5])/1e3) + (24*60*60)
	middle_n = minn + ((maxn - minn) / 2.0)
	middle_e = mine + ((maxe - mine) / 2.0)
	fromtime = datetime.datetime.fromtimestamp(mint)
	totime = datetime.datetime.fromtimestamp(maxt)
	rfrom = fromtime.isoformat()
	ttime = totime.isoformat()
	strs = 'http://api.sehavniva.no/tideapi.php?tide_request=locationdata&lat=%.8f&lon=%.8f&datatype=OBS&lang=nl&tzone=0&refcode=CD&fromtime=%s&totime=%s&interval=10' % (middle_n, middle_e, rfrom, ttime)
	r = requests.get(strs)
	
	root = ET.fromstring(r.text)
	for tidelevel in root.iter('waterlevel'):
		tide = tidelevel.attrib.get('value')
		time = tidelevel.attrib.get('time')
		yourdate = dateutil.parser.parse(time)
		# Tide in SIS is negative; ADD tide to get to correct level.
		tides.append(float(tide) * -0.01)
		times.append(int(yourdate.timestamp()))
	noTides = len(times)
		
def getTide(secSinceEpoch):
	global lastidt
	global times
	global tides
	global noTides
	
	if (noTides <= 0):
		openTidefile()
		if (noTides <= 0):
			printf("Cannot read tidefile")
			sys.exit(2)

	maxtime = times[noTides-1]
	if (secSinceEpoch < times[0] or secSinceEpoch > maxtime):
		return 999999
	p = lastidt
	while (times[p] >= secSinceEpoch):
		p = p - 1
	while (times[p] <= secSinceEpoch):
		p = p + 1
	n = p - 1
	if (times[n] <= secSinceEpoch and times[p] >= secSinceEpoch):
		atime = float(times[n])
		btime = float(times[p])
		atide = float(tides[n])
		btide = float(tides[p])
		ntide = atide+(secSinceEpoch-atime)*(btide-atide)/(btime-atime)
		lastidt = n
		return ntide
	return 999999


	
	
	
# I shall not humiliate any developer by documenting this main program.

cls = lambda: os.system('cls')

filelist = glob.glob('*.pings')
for file in filelist:
	try:
		f = open(file)
		nfile=file+".tidecorrected"
		outfile=open(nfile,'w', encoding='utf-8')
	except Exception:
		print('File',file,'not opened.')
		sys.exit(0)

	getTide(0)
	if (len(times) <= 0):
		print("no tides")
		sys.exit(1)
	
	line = f.readline()
	while (line):
		toks = line.split()
		if (len(toks) == 5):
			tm = int(int(toks[4])/1e3)
			tide = getTide(tm)
			toWlev = float(toks[3]) + tide
			if (tide < 99999):
				outputstr = "%s %s %s %.2f %s\n" % (toks[0], toks[1], toks[2], toWlev, toks[4])
				outfile.write(outputstr)
		else:
			outfile.write(line)

		line = f.readline()

cls()

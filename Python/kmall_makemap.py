import os
import sys
import numpy
import utm
from pyproj import Proj
import io
import subprocess
import matplotlib.pyplot as plt

# I shall not humiliate any developer by documenting this main program.
# However, installing pyproj for Python 3.6 was a challenge:
# 1. Install Visual Studio. I used 2017 Community edition.
# 2. easy_install pyproj
# This compiles proj.4 into Python. You can use other map projection packages, but this is my favorite.

os.system("cls")
strt="python kmall_allfiles.py "+sys.argv[1]
os.system(strt)

verbose=1
arglen = len(sys.argv)
if (arglen != 2):
	print ("Usage: sys.argv[0] <directory with boundingbox.txt>")
	quit()
	
bbfile=sys.argv[1]+"/boundingbox.txt"
if (os.path.exists(bbfile) == False):
	print (bbfile, "does not exist.")
	quit()

		
if (verbose == 1):
	print ("Processing ",bbfile	)
	
dephist = numpy.zeros((1000,), dtype=int)
with open(bbfile) as fk:
	a = fk.readline()
	b = a.strip()
	aminlat, aminlon, amaxlat, amaxlon = b.split()
	minlat = float(aminlat)
	minlon = float(aminlon)
	maxlat = float(amaxlat)
	maxlon = float(amaxlon)
	a = fk.readline()
	b = a.strip()
	amint, amaxt = b.split()
	mint = int(amint)
	maxt = int(amaxt)
	for linq in fk:
		bn = linq.strip()
		ig,jg = bn.split()
		idx = int(ig)
		num = int(jg)
		dephist[idx] = num
	fk.close()
	
if (verbose == 2):
	print (minlat, minlon, maxlat, maxlon)
	print (mint, maxt)
	for i in range (0,999):
		if (dephist[i] > 0):
			s = str(i)+" "+str(dephist[i])
			print (s)

mineast, minnorth, utm_zone1, zone_letter1 = utm.from_latlon(float(minlat), float(minlon))		
maxeast, maxnorth, utm_zone2, zone_letter2 = utm.from_latlon(float(maxlat), float(maxlon))		
if (utm_zone1 != utm_zone2 or zone_letter1 != zone_letter2):
	print ("Warning: data spanning two UTM-zones: "+str(utm_zone1)+zone_letter1+" and "+str(utm_zone2)+zone_letter2)
ns="+north"
if (float(minlat) < 0):
	ns="+south"
proj_str="+proj=utm +zone="+str(utm_zone1)+" "+ns+" +ellps=WGS84 +datum=WGS84 +units=m"
if (verbose == 1):
	print (proj_str)
myProj = Proj(proj_str)
e1, n1 = myProj(float(minlon), float(minlat))
e2, n2 = myProj(float(maxlon), float(minlat))
e3, n3 = myProj(float(minlon), float(maxlat))
e4, n4 = myProj(float(maxlon), float(maxlat))
mE = numpy.array((e1, e2, e3, e4))
mN = numpy.array((n1, n2, n3, n4))
minE = mE.min()
maxE = mE.max()
minN = mN.min()
maxN = mN.max()
if (verbose == 1):
	print (minE, minN, maxE, maxN)
ediff = maxE - minE
ndiff = maxN - minN

totnodep = 0
a_0_200m = 0
a_200_2000m = 0
a_2000_6000m = 0
a_6000_10000m = 0
for i in range (0,999):
	totnodep = totnodep + dephist[i]
	if (i < 20):
		a_0_200m = totnodep
	if (i < 200):
		a_200_2000m = totnodep - a_0_200m
	if (i < 600):
		a_2000_6000m = totnodep - a_200_2000m - a_0_200m
	if (i >= 600):
		a_6000_10000m = totnodep - a_2000_6000m - a_200_2000m - a_0_200m
if (verbose == 1):
	print (totnodep, a_0_200m, a_200_2000m, a_2000_6000m, a_6000_10000m)
ps200   = (100.0/float(totnodep))*float(a_0_200m)
ps2000  = (100.0/float(totnodep))*float(a_200_2000m)
ps6000  = (100.0/float(totnodep))*float(a_2000_6000m)
ps10000 = (100.0/float(totnodep))*float(a_6000_10000m)
if (verbose == 1):
	print (ps200, ps2000, ps6000, ps10000)

# Guess best grid cell size
gc = 1.0
if (ps200 > 70):
	gc = 1.0
if (ps2000 > 50):
	gc = 50.0
if (ps6000 > 50):
	gc = 200.0
if (ps10000 > 20):
	gc = 300.0

noCellsE = int((int(ediff)/int(gc)) + 1)
noCellsN = int((int(ndiff)/int(gc)) + 1)

if (noCellsE > 10000 or noCellsN > 10000):
	print ("Warning: area is very large", gc, noCellsE, noCellsN)
	
# Now try to make arrays and put data into them
meanDepth = numpy.zeros((noCellsE, noCellsN), dtype=float)
noDepth   = numpy.zeros((noCellsE, noCellsN), dtype=float)
if (verbose == 1):
	print (meanDepth.shape)

# Find all files
myfiles=[]
for start, dirs, files in os.walk(sys.argv[1]):
	for fname in files:
		v = os.path.join(start,fname)
		if (v.endswith(".bb")):
			vv = v[:-3]
			kv = vv.replace("\\","/")
			if (os.path.exists(kv) and kv.endswith(".kmall")):
				myfiles.append(kv)
#Here: a .bb-file (bounding box file) has been created for this .kmall-file.
#Now we know the .kmall-file is ok and is a part of this main project
for f in myfiles:
	if (verbose == 1):
		print (f)
	proc = subprocess.Popen(['python','-u','kmall_input.py','-v','2','-f',f], stdout=subprocess.PIPE)
	for line in io.TextIOWrapper(proc.stdout, encoding="utf-8"):
		kline = line.strip();
		slat, slon, sdep, stim = kline.split()
		lat = float(slat)
		lon = float(slon)
		dep = float(sdep)
		tim = int(stim)
		east, north=myProj(float(lon),float(lat))
		ceast = int(int(east) - int(mineast))
		cnort = int(int(north) - int(minnorth))
		eidx = int(int(ceast)/int(gc))
		nidx = int(int(cnort)/int(gc))
		nop = noDepth[eidx, nidx]
		nop = nop + 1
		noDepth[eidx, nidx] = nop
		if (nop == 1):
			meanDepth[eidx, nidx] = dep
		else:
			lastmean = meanDepth[eidx, nidx]
			navg = lastmean + ((dep - lastmean)/nop)
			meanDepth[eidx, nidx] = navg
	
#meanDepth = numpy.zeros((noCellsE, noCellsN), dtype=float)
#noDepth   = numpy.zeros((noCellsE, noCellsN), dtype=float)
# These arrays can now be plotted.
# Remember to add minE, minN and that the grid cell size is gc in both E, N

numpyfile=sys.argv[1]+"/meanDepth.npy"
numpy.save(numpyfile,meanDepth)
numpyfile=sys.argv[1]+"/noDepth.npy"
numpy.save(numpyfile, noDepth)

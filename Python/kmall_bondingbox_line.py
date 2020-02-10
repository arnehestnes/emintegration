import sys
import numpy

# I shall not humiliate any developer by documenting this main program.
minlat=minlon=999999999.0
maxlat=maxlon=-999999999.0
mintime=9999999999999999
maxtime=0
dephist = numpy.zeros((1000,), dtype=int)
for line in sys.stdin:
	stri = line.strip()
	mylist=stri.split()
	i =len(mylist)
	if (i == 4):
		lat, lon, dep, tim = mylist
		flat = float(lat)
		flon = float(lon)
		itim = int(tim)
		fdep = float(dep)
		if (flat < minlat):
			minlat = flat
		if (flat > maxlat):
			maxlat = flat
		if (flon < minlon):
			minlon = flon
		if (flon > maxlon):
			maxlon = flon
		if (itim < mintime):
			mintime = itim
		if (itim > maxtime):
			maxtime = itim
		idx = (int)(fdep/10.0)
		k = dephist[idx]
		k = k + 1;
		dephist[idx] = k
print(minlat, minlon, maxlat, maxlon)
print(mintime, maxtime)
for i in range (0,999):
	if (dephist[i] > 0):
		print(i,dephist[i])

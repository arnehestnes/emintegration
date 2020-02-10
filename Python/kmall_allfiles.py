import os
import sys
import numpy

# I shall not humiliate any developer by documenting this main program.

#Set this to 1 to force recreate bounding box files for all lines.
force_recreate_boundingbox=0

arglen = len(sys.argv)
if (arglen != 2):
	print("Usage: sys.argv[0] <directory with .kmall-files>")
	quit()
	
rawdir=sys.argv[1]
if (os.path.isdir(rawdir) == False):
	print(rawdir, "does not exist.")
	quit()

print("Processing ",rawdir)	
myfiles=[]
for start, dirs, files in os.walk(rawdir):
	for fname in files:
		v = os.path.join(start,fname)
		if (v.endswith(".kmall")):
			myfiles.append(v)

mlen = len(myfiles)
if (mlen < 1):
	print("No .kmall-files found in ",rawdir)
	quit()
	
for v in myfiles:
	bbfile = v+".bb"
# Recreate existing bounding box?
	if (os.path.isfile(bbfile) == False or force_recreate_boundingbox == 1):
		strt="python kmall_input.py -v 2 -f \""+v+"\" | python kmall_bondingbox_line.py > \""+bbfile+"\""
		if (os.path.isfile("kmall_input.exe") and os.path.isfile("kmall_bondingbox_line.exe")):
			strt="kmall_input.exe -v 2 -f \""+v+"\" | kmall_bondingbox_line.exe > \""+bbfile+"\""
		print(strt)
		os.system(strt)
	
myfiles=[]
for start, dirs, files in os.walk(rawdir):
	for fname in files:
		v = os.path.join(start,fname)
		if (v.endswith(".bb")):
			myfiles.append(v)

mlen = len(myfiles)
if (mlen < 1):
	print("No .bb-files found in ",rawdir)
	quit()

minlat = minlon = 9999999.0
maxlat = maxlon = -9999999.0
mintime=9999999999999999
maxtime=0
dephist = numpy.zeros((1000,), dtype=int)
for v in myfiles:
	with open(v) as f:
		line = f.readline()
		x = line.strip()
		milat, milon, mxlat, mxlon = x.split()
		fmilat = float(milat)
		fmilon = float(milon)
		fmxlat = float(mxlat)
		fmxlon = float(mxlon)
		if (fmilat < minlat):
			minlat = fmilat
		if (fmilon < minlon):
			minlon = fmilon
		if (fmxlat > maxlat):
			maxlat = fmxlat
		if (fmxlon > maxlon):
			maxlon = fmxlon
		timline = f.readline()
		x = timline.strip()
		imin, imax = x.split()
		xmin = int(imin)
		xmax = int(imax)
		if (xmin < mintime):
			mintime = xmin
		if (xmax > maxtime):
			maxtime = xmax
		for line in f:
			k = line.strip()
			idx, num = k.split()
			indx = int(idx)
			inum = int(num)
			n = dephist[indx]
			j = n + inum
			dephist[indx] = j
		f.close()

bbstr=str(minlat)+" "+str(minlon)+" "+str(maxlat)+" "+str(maxlon)+"\n"
tstr=str(mintime)+" "+str(maxtime)+"\n"

bbfile_total = rawdir+"/boundingbox.txt"
with open(bbfile_total,'w') as tbfile:
	tbfile.write(bbstr)
	tbfile.write(tstr)
	for i in range (0,999):
		if (dephist[i] > 0):
			hstr = str(i)+" "+str(dephist[i])+"\n"
			tbfile.write(hstr)
	tbfile.close()

import glob
import os
import sys
import utm


#Receive an array of all tokens from a seabedimage-file
#Return a string in csv-format
def makeStringFromSeabedImage(tokval):
	no = len(tokval)
	# Interpolate between positions.  First I skip the samples outside center
	# sample on each side
	prevX = ""
	prevY = ""
	thisX = ""
	thisY = ""
	prevIdx = -1
	retval = ""
	for i in range(no):
		found = tokval[i].find('(')
		if (found >= 0):
			nxt = tokval[i + 1].find(')')
			if (nxt < 0):
				return "error"
			if (len(thisY) < 1):
				prevY = tokval[i]
				prevX = tokval[i + 1]
				prevIdx = i + 2 #First valid value
				thisX = prevX
				thisY = prevY
			else:
				thisY = tokval[i]
				thisX = tokval[i + 1]
				#Then interpolate the values between prevIdx and i; i-1 being the last
				#value
				#First convert string to float, then to utm
				px = float(prevX[:-1])
				py = float(prevY[1:])
				tx = float(thisX[:-1])
				ty = float(thisY[1:])
				u = utm.from_latlon(py, px)
				x1 = u[0]
				y1 = u[1]
				u = utm.from_latlon(ty, tx)
				x2 = u[0]
				y2 = u[1]
				noSamp = i - prevIdx
				xd = float(x2 - x1) / float(noSamp)
				yd = float(y2 - y1) / float(noSamp)
				for s in range(noSamp):
					samp = float(tokval[prevIdx + s]) * 0.1
					x = x1 + xd * s
					y = y1 + yd * s
					str = "%.2f %.2f %.1f\n" % (x,y,samp)
					retval += str
					break_me_here = 0 #debug purposes
				prevX = thisX
				prevY = thisY
				prevIdx = i + 2

#Then add the outer samples.  Mirror the second (last) position to find the
#direction and interpolation distance
#You can change the output order if necessary to get a nice output in the csv-file
	first = -1
	for i in range(no):
		a = tokval[i].find('(')
		if (a >= 0):
			first = i
			break

	second = -1
	for i in range(no):
		a = tokval[i].find('(')
		if (a >= 0):
			if (i != first):
				second = i
				break

	last = -1
	for i in range(no):
		a = tokval[no - 1 - i].find('(')
		if (a >= 0):
			last = i
			break

	second_last = -1
	for i in range(no):
		a = tokval[no - 1 - i].find('(')
		if (a >= 0):
			if (i != last):
				second_last = i
				break
	
	last = no - last - 1
	second_last = no - second_last - 1

	px = float(tokval[first + 1][:-1])
	py = float(tokval[first][1:])
	firstu = utm.from_latlon(py, px)
	firstx = firstu[0]
	firsty = firstu[1]
	px = float(tokval[second + 1][:-1])
	py = float(tokval[second][1:])
	secondu = utm.from_latlon(py, px)
	secondx = secondu[0]
	secondy = secondu[1]
	fxd = (secondx - firstx)/(second - first - 2)
	fyd = (secondy - firsty)/(second - first - 2)

	px = float(tokval[last + 1][:-1])
	py = float(tokval[last][1:])
	lastu = utm.from_latlon(py, px)
	lastx = lastu[0]
	lasty = lastu[1]
	px = float(tokval[second_last + 1][:-1])
	py = float(tokval[second_last][1:])
	second_lastu = utm.from_latlon(py, px)
	second_lastx = second_lastu[0]
	second_lasty = second_lastu[1]
	lxd = (second_lastx - lastx)/(second_last - last - 2)
	lyd = (second_lasty - lasty)/(second_last - last - 2)

	for i in range(no):
		if (i == first):
			break;
		samp = float(tokval[i]) * 0.1
		x = firstx - i * fxd # use - as we are moving outwards from second through first and beyond
		y = firsty - i * fyd
		str = "%.2f %.2f %.1f\n" % (x,y,samp)
		retval += str


	for i in range(no):
		k = no - 1 - i
		if (k == last + 1):
			break;
		samp = float(tokval[last + i + 2]) * 0.1
		x = lastx + i * lxd
		y = lasty + i * lyd
		str = "%.2f %.2f %.1f\n" % (x,y,samp)
		retval += str

	return retval




# I shall not humiliate any developer by documenting this main program.
cls = lambda: os.system('cls')
cls()

filelist = glob.glob('*.seabedimage')
for file in filelist:
	try:
		f = open(file)
		nfile = file + ".csv"
		outfile = open(nfile,'w', encoding='utf-8')
	except Exception:
		print('File',file,'not opened.')
		sys.exit(0)

	print("Processing file: %s" % (file))
	line = f.readline()
	while (line):
		toks = line.split()
		if (len(toks) > 3):			
			outstr = makeStringFromSeabedImage(toks)
			if (outstr != "error"):
				outfile.write(outstr)
		line = f.readline()


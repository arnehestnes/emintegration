import glob
import os
import sys
import utm

cls = lambda: os.system('cls')


filelist = glob.glob('*.tidecorrected')
for file in filelist:
	try:
		f = open(file)
		nfile=file+".utm.csv"
		outfile=open(nfile,'w', encoding='utf-8')
	except Exception:
		print('File',file,'not opened.')
		sys.exit(0)

	line = f.readline()
	n = 0
	e = 0
	tide = 0
	while (line):
		toks = line.split()
		if (len(toks) < 5):
			line = f.readline()
			continue
		if (len(toks) == 5):
			n = float(toks[0])
			e = float(toks[1])
			tide = float(toks[3])
		else:
			cnt = 0
			while(cnt < len(toks)):
				lat = n + float(toks[cnt])
				lon = e + float(toks[cnt+1])
				dpt = float(toks[cnt+2]) + tide
				u = utm.from_latlon(lat, lon)
				outputstr = "%.2f %.2f %.2f\n" % (u[0], u[1], dpt * -1.0)
				outfile.write(outputstr)			
				cnt += 5

		line = f.readline()

cls()

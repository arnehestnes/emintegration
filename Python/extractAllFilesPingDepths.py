import glob
import os
import sys
import struct
import time
import datetime

clear = lambda: os.system('cls')

min_e = min_n = 99.0
max_e = max_n = 0.0
start_t = stop_t = -1

outfile=open('kmall_bb.allbb','w', encoding='utf-8')

lastidt=0
outfile=0

# Process one depth datagram, #MRZ
# lenghta and chunk are from processDatagram, see below
# millisec is decoded from the header, so I send it in as a parameter here
def processDepthDatagram(millisec, lenghta, chunk):
	global outfile
	global min_e
	global min_n
	global max_e
	global max_n
	global start_t
	global stop_t
	
# Headersize is 4 bytes smaller than in the headerfile, remember that the 4 bytes with the length has been dropped		
	headersize = 1+1+1+1+1+1+2+4+4
	
	partitionsize = 2+2
	commonsize = 2+2+8
	common = struct.Struct('HHBBBBBBBB')
	numBytesCmnPart, pingCnt, rxFansPerPing, rxFanIndex, swathsPerPing, swathAlongPosition, \
	txTransducerInd, rxTransducerInd, numRxTransducers, algorithmType = common.unpack_from(chunk, headersize + partitionsize)
	
	pinginfo_size = 2+2+4+1+1+1+1+1+1+2+ 11*4 +2+2+1+1+2+4+4+4+4+2+2+4+2+2+ 6*4 +1+1+1+1+8+8+4

	pinginfo = struct.Struct('HHfBBBBBBHfffffffffffhhBBHIfffHHfHHffffffBBBBddf')

	numBytesInfoData, padding0, pingRate_Hz, beamSpacing, depthMode,\
	subDepthMode, distanceBtwSwath, detectionMode, pulseForm, \
	padding01, frequencyMode_Hz, freqRangeLowLim_Hz, \
	freqRangeHighLim_Hz, maxTotalTxPulseLength_sec, \
	maxEffTxPulseLength_sec, maxEffTxBandWidth_Hz, \
	absCoeff_dBPerkm, portSectorEdge_deg, \
	starbSectorEdge_deg, portMeanCov_deg, \
	starbMeanCov_deg, portMeanCov_m, \
	starbMeanCov_m, modeAndStabilisation, \
	runtimeFilter1, runtimeFilter2,\
	pipeTrackingStatus, transmitArraySizeUsed_deg,\
	receiveArraySizeUsed_deg, transmitPower_dB,\
	SLrampUpTimeRemaining, padding1,\
	yawAngle_deg, numTxSectors, numBytesPerTxSector,\
	headingVessel_deg, soundSpeedAtTxDepth_mPerSec,\
	txTransducerDepth_m, z_waterLevelReRefPoint_m, \
	x_txTransducerArm_SCS_m, y_txTransducerArm_SCS_m,\
	latLongInfo, posSensorStatus, attitudeSensorStatus,\
	padding2, latitude_deg, longitude_deg,\
	ellipsoidHeightReRefPoint_m = pinginfo.unpack_from(chunk, headersize + partitionsize + commonsize)
# Bug in Python, fix it (binary alignments not correct)
	latlon = struct.Struct("d")
	klat = latlon.unpack_from(chunk, headersize + partitionsize + commonsize+124)
	klon = latlon.unpack_from(chunk, headersize + partitionsize + commonsize+124+8)
	ellheight = struct.Struct("f")
	ellipsheight = ellheight.unpack_from(chunk, headersize + partitionsize + commonsize+124+8+8)
	latitude_deg = klat[0]
	longitude_deg = klon[0]
	ellipsoidHeightReRefPoint_m = ellipsheight[0]
	
	sec = int(millisec/1000)
	
# Pointer offset to sectorInfo
	sectorInfo_offset = headersize + partitionsize + commonsize + pinginfo_size
	sectorInfo = struct.Struct('BBBBfffffffBBH')
	sectorInfo_size = 1+1+1+1+7*4+1+1+2
	i = 0
	while (i < numTxSectors):
		txSectorNumb, txArrNumber, txSubArray, padding0,\
		sectorTransmitDelay_sec, tiltAngleReTx_deg,\
		txNominalSourceLevel_dB, txFocusRange_m,\
		centreFreq_Hz, signalBandWidth_Hz, \
		totalSignalLength_sec, pulseShading, signalWaveForm,\
		padding1 = sectorInfo.unpack_from(chunk, sectorInfo_offset + i * sectorInfo_size)
		i+=1

	rxInfo_offset = sectorInfo_offset + numTxSectors * sectorInfo_size
	rxInfo = struct.Struct('HHHHffffHHHH')
	rxInfo_size = 2+2+2+2+4+4+4+4+2+2+2+2
	numBytesRxInfo, numSoundingsMaxMain, numSoundingsValidMain, numBytesPerSounding, \
	WCSampleRate, seabedImageSampleRate, BSnormal_dB, BSoblique_dB, \
	extraDetectionAlarmFlag, numExtraDetections, numExtraDetectionClasses, \
	numBytesPerClass = rxInfo.unpack_from(chunk, rxInfo_offset)
	
	extraDetClassInfo_offset = rxInfo_offset + rxInfo_size

	extraDetectionSize = 2+1+1
	extraDetectionStruct = struct.Struct('HBB')
		
	sounding_offset = extraDetClassInfo_offset + numExtraDetectionClasses * extraDetectionSize

	soundingStruct = struct.Struct('HBBBBBBBBHffffffHHffffffffffffffffffHHHH')
	sounding_size = 2+8+2+6*4+2+2+18*4+4*2

	outputstr = "\n%.8f %.8f %.2f %.2f %d\n" % (latitude_deg, longitude_deg, 
		ellipsoidHeightReRefPoint_m, z_waterLevelReRefPoint_m, millisec)
	outfile.write(outputstr)
	i = 0
	while(i < numSoundingsMaxMain):
		soundingIndex, txSectorNumb, detectionType, \
		detectionMethod, rejectionInfo1, rejectionInfo2, \
		postProcessingInfo, detectionClass, detectionConfidenceLevel, \
		padding, rangeFactor, qualityFactor, \
		detectionUncertaintyVer_m, detectionUncertaintyHor_m, \
		detectionWindowLength_sec, echoLength_sec, \
		WCBeamNumb, WCrange_samples, WCNomBeamAngleAcross_deg, \
		meanAbsCoeff_dBPerkm, reflectivity1_dB, reflectivity2_dB, \
		receiverSensitivityApplied_dB, sourceLevelApplied_dB, \
		BScalibration_dB, TVG_dB, beamAngleReRx_deg, \
		beamAngleCorrection_deg, twoWayTravelTime_sec, \
		twoWayTravelTimeCorrection_sec, deltaLatitude_deg, \
		deltaLongitude_deg, z_reRefPoint_m, y_reRefPoint_m, \
		x_reRefPoint_m, beamIncAngleAdj_deg, realTimeCleanInfo, \
		SIstartRange_samples, SIcentreSample, \
		SInumSamples = soundingStruct.unpack_from(chunk, sounding_offset + i * sounding_size)
		i+=1
			
# THIS IS IT. This is where we output xyz-points			
# Depths are referred to the reference point. To get it to the waterline, SUBSTRACT the distance from 
# Error estimates are also available: detectionUncertaintyVer_m and detectionUncertaintyHor_m
		waterlevel = z_reRefPoint_m - z_waterLevelReRefPoint_m
		plat = latitude_deg + deltaLatitude_deg
		plon = longitude_deg + deltaLongitude_deg
		outputstr = " %.8f %.8f %.2f %.2f %.2f" % (deltaLatitude_deg, deltaLongitude_deg, 
			z_reRefPoint_m, detectionUncertaintyVer_m, detectionUncertaintyHor_m)
		outfile.write(outputstr)
		n = float(latitude_deg)
		e = float(longitude_deg)
		t = int(millisec)
		if (start_t < 0 or t < start_t):
			start_t = t
		if (t > stop_t):
			stop_t = t
		if (min_e > e):
			min_e = e
		if (min_n > n):
			min_n = n
		if (e > max_e):
			max_e = e
		if (n > max_n):
			max_n = n


# Process one depth datagram, #MRZ, from the kmall-version from November 2019
# lenghta and chunk are from processDatagram, see below
# millisec is decoded from the header, so I send it in as a parameter here
def processDepthDatagram1(millisec, lenghta, chunk):
	global outfile
	global min_e
	global min_n
	global max_e
	global max_n
	global start_t
	global stop_t
	
# Headersize is 4 bytes smaller than in the headerfile, remember that the 4 bytes with the length has been dropped		
	headersize = 1+1+1+1+1+1+2+4+4
	
	partitionsize = 2+2
	commonsize = 2+2+8
	common = struct.Struct('HHBBBBBBBB')
	numBytesCmnPart, pingCnt, rxFansPerPing, rxFanIndex, swathsPerPing, swathAlongPosition, \
	txTransducerInd, rxTransducerInd, numRxTransducers, algorithmType = common.unpack_from(chunk, headersize + partitionsize)
	
	# Changed from version 0
	pinginfo_size = 2+2+4+1+1+1+1+1+1+2+ 11*4 +2+2+1+1+2+4+4+4+4+2+2+4+2+2+ 6*4 +1+1+1+1+8+8+4 +4+1+1+2

	pinginfo = struct.Struct('HHfBBBBBBHfffffffffffhhBBHIfffHHfHHffffffBBBBddf')

	numBytesInfoData, padding0, pingRate_Hz, beamSpacing, depthMode,\
	subDepthMode, distanceBtwSwath, detectionMode, pulseForm, \
	padding01, frequencyMode_Hz, freqRangeLowLim_Hz, \
	freqRangeHighLim_Hz, maxTotalTxPulseLength_sec, \
	maxEffTxPulseLength_sec, maxEffTxBandWidth_Hz, \
	absCoeff_dBPerkm, portSectorEdge_deg, \
	starbSectorEdge_deg, portMeanCov_deg, \
	starbMeanCov_deg, portMeanCov_m, \
	starbMeanCov_m, modeAndStabilisation, \
	runtimeFilter1, runtimeFilter2,\
	pipeTrackingStatus, transmitArraySizeUsed_deg,\
	receiveArraySizeUsed_deg, transmitPower_dB,\
	SLrampUpTimeRemaining, padding2,\
	yawAngle_deg, numTxSectors, numBytesPerTxSector,\
	headingVessel_deg, soundSpeedAtTxDepth_mPerSec,\
	txTransducerDepth_m, z_waterLevelReRefPoint_m, \
	x_txTransducerArm_SCS_m, y_txTransducerArm_SCS_m,\
	latLongInfo, posSensorStatus, attitudeSensorStatus,\
	padding3, latitude_deg, longitude_deg,\
	ellipsoidHeightReRefPoint_m = pinginfo.unpack_from(chunk, headersize + partitionsize + commonsize)
# Bug in Python, fix it (binary alignments not correct)
	latlon = struct.Struct("d")
	klat = latlon.unpack_from(chunk, headersize + partitionsize + commonsize+124)
	klon = latlon.unpack_from(chunk, headersize + partitionsize + commonsize+124+8)
	ellheight = struct.Struct("f")
	ellipsheight = ellheight.unpack_from(chunk, headersize + partitionsize + commonsize+124+8+8)
	latitude_deg = klat[0]
	longitude_deg = klon[0]
	ellipsoidHeightReRefPoint_m = ellipsheight[0]
	# Changed in Version 1
	bsCorrectionOffset_dB = ellheight.unpack_from(chunk, headersize + partitionsize + commonsize+124+8+8+4)[0]
	byterec = struct.Struct("B")
	lambertsLawApplied = byterec.unpack_from(chunk, headersize + partitionsize + commonsize+124+8+8+4+4)[0]
	iceWindow = byterec.unpack_from(chunk, headersize + partitionsize + commonsize+124+8+8+4+4+1)[0]
	shortrec = struct.Struct("H")
	padding4 = shortrec.unpack_from(chunk, headersize + partitionsize + commonsize+124+8+8+4+4+1+1)[0]

	sec = int(millisec/1000)
	
# Pointer offset to sectorInfo
	sectorInfo_offset = headersize + partitionsize + commonsize + pinginfo_size
	# Changed from version 0
	sectorInfo = struct.Struct('BBBBfffffffBBHfff')
	sectorInfo_size = 1+1+1+1+7*4+1+1+2  +4+4+4
	i = 0
	while (i < numTxSectors):
		txSectorNumb, txArrNumber, txSubArray, padding0,\
		sectorTransmitDelay_sec, tiltAngleReTx_deg,\
		txNominalSourceLevel_dB, txFocusRange_m,\
		centreFreq_Hz, signalBandWidth_Hz, \
		totalSignalLength_sec, pulseShading, signalWaveForm,\
		padding1, highVoltageLevel_dB, sectorTrackingCorr_dB, effectiveSignalLength_sec = sectorInfo.unpack_from(chunk, sectorInfo_offset + i * sectorInfo_size)
		i+=1

	rxInfo_offset = sectorInfo_offset + numTxSectors * sectorInfo_size
	rxInfo = struct.Struct('HHHHffffHHHH')
	rxInfo_size = 2+2+2+2+4+4+4+4+2+2+2+2
	numBytesRxInfo, numSoundingsMaxMain, numSoundingsValidMain, numBytesPerSounding, \
	WCSampleRate, seabedImageSampleRate, BSnormal_dB, BSoblique_dB, \
	extraDetectionAlarmFlag, numExtraDetections, numExtraDetectionClasses, \
	numBytesPerClass = rxInfo.unpack_from(chunk, rxInfo_offset)
	
	extraDetClassInfo_offset = rxInfo_offset + rxInfo_size

	extraDetectionSize = 2+1+1
	extraDetectionStruct = struct.Struct('HBB')
		
	sounding_offset = extraDetClassInfo_offset + numExtraDetectionClasses * extraDetectionSize

	soundingStruct = struct.Struct('HBBBBBBBBHffffffHHffffffffffffffffffHHHH')
	sounding_size = 2+8+2+6*4+2+2+18*4+4*2

	outputstr = "\n%.8f %.8f %.2f %.2f %d\n" % (latitude_deg, longitude_deg, 
		ellipsoidHeightReRefPoint_m, z_waterLevelReRefPoint_m, millisec)
	outfile.write(outputstr)
	i = 0
	while(i < numSoundingsMaxMain):
		soundingIndex, txSectorNumb, detectionType, \
		detectionMethod, rejectionInfo1, rejectionInfo2, \
		postProcessingInfo, detectionClass, detectionConfidenceLevel, \
		padding, rangeFactor, qualityFactor, \
		detectionUncertaintyVer_m, detectionUncertaintyHor_m, \
		detectionWindowLength_sec, echoLength_sec, \
		WCBeamNumb, WCrange_samples, WCNomBeamAngleAcross_deg, \
		meanAbsCoeff_dBPerkm, reflectivity1_dB, reflectivity2_dB, \
		receiverSensitivityApplied_dB, sourceLevelApplied_dB, \
		BScalibration_dB, TVG_dB, beamAngleReRx_deg, \
		beamAngleCorrection_deg, twoWayTravelTime_sec, \
		twoWayTravelTimeCorrection_sec, deltaLatitude_deg, \
		deltaLongitude_deg, z_reRefPoint_m, y_reRefPoint_m, \
		x_reRefPoint_m, beamIncAngleAdj_deg, realTimeCleanInfo, \
		SIstartRange_samples, SIcentreSample, \
		SInumSamples = soundingStruct.unpack_from(chunk, sounding_offset + i * sounding_size)
		i+=1
			
# THIS IS IT. This is where we output xyz-points			
# Depths are referred to the reference point. To get it to the waterline, SUBSTRACT the distance from 
# Error estimates are also available: detectionUncertaintyVer_m and detectionUncertaintyHor_m
		waterlevel = z_reRefPoint_m - z_waterLevelReRefPoint_m
		plat = latitude_deg + deltaLatitude_deg
		plon = longitude_deg + deltaLongitude_deg
		outputstr = " %.8f %.8f %.2f %.2f %.2f" % (deltaLatitude_deg, deltaLongitude_deg, 
			z_reRefPoint_m, detectionUncertaintyVer_m, detectionUncertaintyHor_m)
		outfile.write(outputstr)
		n = float(latitude_deg)
		e = float(longitude_deg)
		t = int(millisec)
		if (start_t < 0 or t < start_t):
			start_t = t
		if (t > stop_t):
			stop_t = t
		if (min_e > e):
			min_e = e
		if (min_n > n):
			min_n = n
		if (e > max_e):
			max_e = e
		if (n > max_n):
			max_n = n

# What happens in processDatagram? Read the documentation of the kmall-format.
# This is the processing of the datagram to find out what datagram type this is.
# The processing of each datagram type takes place in specific routines
def processDatagram(lengtha, chunk):
	header_without_length = struct.Struct('ccccBBHII')
	dgm_type0,dgm_type1,dgm_type2,dgm_type3,dgm_version,sysid,emid,sec,nsec = header_without_length.unpack_from(chunk,0)
	dgm_type = dgm_type0 + dgm_type1 + dgm_type2 + dgm_type3
		
# Decode time 
	nanosec = sec;
	nanosec *= 1E9;
	nanosec += nsec;
	millisec = nanosec;
	millisec /= 1E6;	
	
	strk = dgm_type.decode()
	if (strk == '#MRZ'):
		if (dgm_version == 0):
			processDepthDatagram(millisec, lengtha, chunk)
		if (dgm_version == 1):
			processDepthDatagram1(millisec, lengtha, chunk)

	
	
	
# I shall not humiliate any developer by documenting this main program.
# The processing of the datagram takes place in the routine processDatagram.


files = glob.glob('*.kmall')
for file in files:
	try:
		f = open(file, 'rb')
		nfile=file+".pings"
		outfile=open(nfile,'w', encoding='utf-8')
	except Exception:
		print('File',file,'not opened.')
		sys.exit(0)

	print(file)
# Process the file:
	f.seek(0, 2)
	file_size = f.tell()
	f.seek(0, 0)
	remaining = file_size

# Read all datagrams and process each of them
	while (remaining > 0):
# First read 4 bytes that contains the length of the chunk
		lenghta = struct.unpack("I",f.read(4))
		remaining -= 4
# Then read the chunk. Note that the length read includes the 4 bytes in the integer.
		dgmsize = lenghta[0] - 4
		chunk = f.read(dgmsize)
		remaining -= dgmsize
# Then process this chunk
		processDatagram(dgmsize, chunk)
	
	f.close()
	outfile.close()
	
outfile=open('kmall_bb.allbb','w', encoding='utf-8')
outstring = "%.8f %.8f %.8f %.8f %d %d\n" % (min_n, min_e, max_n, max_e, start_t, stop_t)
outfile.write (outstring)
outfile.close()

clear()


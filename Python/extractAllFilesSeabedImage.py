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
# lengtha and chunk are from processDatagram, see below
# millisec is decoded from the header, so I send it in as a parameter here
def processDepthDatagram1(millisec, lengtha, chunk):
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
	
	pinginfo_size = 2+2+4+1+1+1+1+1+1+2+ 11*4 +2+2+1+1+2+4+4+4+4+2+2+4+2+2+ 6*4 +1+1+1+1+8+8+4+8

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
	# Changed in Version 1
	bsCorrectionOffset_dB = ellheight.unpack_from(chunk, headersize + partitionsize + commonsize + 124 + 8 + 8 + 4)[0]
	byterec = struct.Struct("B")
	lambertsLawApplied = byterec.unpack_from(chunk, headersize + partitionsize + commonsize + 124 + 8 + 8 + 4 + 4)[0]
	iceWindow = byterec.unpack_from(chunk, headersize + partitionsize + commonsize + 124 + 8 + 8 + 4 + 4 + 1)[0]
	shortrec = struct.Struct("H")
	padding4 = shortrec.unpack_from(chunk, headersize + partitionsize + commonsize + 124 + 8 + 8 + 4 + 4 + 1 + 1)[0]
	
	sec = int(millisec/1000)
	
# Pointer offset to sectorInfo
	sectorInfo_offset = headersize + partitionsize + commonsize + pinginfo_size
	# Changed from version 0
	sectorInfo = struct.Struct('BBBBfffffffBBHfff')
	sectorInfo_size = 1 + 1 + 1 + 1 + 7 * 4 + 1 + 1 + 2 + 4 + 4 + 4
	i = 0
	while (i < numTxSectors):
		txSectorNumb, txArrNumber, txSubArray, padding0,\
		sectorTransmitDelay_sec, tiltAngleReTx_deg,\
		txNominalSourceLevel_dB, txFocusRange_m,\
		centreFreq_Hz, signalBandWidth_Hz, \
		totalSignalLength_sec, pulseShading, signalWaveForm,\
		padding1, highVoltageLevel_dB, sectorTrackingCorr_dB, effectiveSignalLength_sec  = sectorInfo.unpack_from(chunk, sectorInfo_offset + i * sectorInfo_size)
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

    #Offset to seabed image
	seabedImageStart = sounding_offset + (sounding_size * (numSoundingsMaxMain + numExtraDetections))
	seabedStruct = struct.Struct('h')
	sbed_len = lengtha + 4 - seabedImageStart - 4
	tot_no_sbed = sbed_len / 2
	verify_length = tot_no_sbed * 2
	lenStruct = struct.Struct('I')
	dgmlenver = seabedImageStart + sbed_len
	dgmlen = lenStruct.unpack_from(chunk,dgmlenver-4)[0] # should be 4 more then len

	outputstr = "\n%.8f %.8f %d\n" % (latitude_deg, longitude_deg, millisec)
	outfile.write(outputstr)
	sbed_start = seabedImageStart # This is the pointer to the start of the seabed image for current beam
	no_sbed_found = 0
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
			
# THIS IS IT. This is where we output seabed image
		plat = latitude_deg + deltaLatitude_deg
		plon = longitude_deg + deltaLongitude_deg
		next_sbd_start = sbed_start + (2 * SInumSamples)
		if (y_reRefPoint_m < 0): # Reverse the output of the samples, see documentation
			sbed_start = next_sbd_start - 2
			center_samp = SInumSamples - SIcentreSample
		else:
			center_samp = SIcentreSample

		for n in range (0, SInumSamples):
			no_sbed_found += 1
			if (n == center_samp): # Put in position of center sample
				outputstr = " (%.8f %.8f) " % (plat, plon)
				outfile.write(outputstr)
			sbed_sample = seabedStruct.unpack_from(chunk, sbed_start)[0]
			outputstr = " %d" % (sbed_sample)
			outfile.write(outputstr)
			if (y_reRefPoint_m < 0):
				sbed_start -= 2
			else:
				sbed_start += 2 # jump 2 bytes (short) forwards

		sbed_start = next_sbd_start
		# There are 9 samples per extra detection, and there may be 2 bytes padding at the end
		if (i > 798):
			break_me_here = 0 # for debugging purposes


# Process one depth datagram, #MRZ
# lengtha and chunk are from processDatagram, see below
# millisec is decoded from the header, so I send it in as a parameter here
def processDepthDatagram(millisec, lengtha, chunk):
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

    #Offset to seabed image
	seabedImageStart = sounding_offset + (sounding_size * (numSoundingsMaxMain + numExtraDetections))
	seabedStruct = struct.Struct('h')
	sbed_len = lengtha + 4 - seabedImageStart - 4
	tot_no_sbed = sbed_len / 2
	verify_length = tot_no_sbed * 2
	lenStruct = struct.Struct('I')
	dgmlenver = seabedImageStart + sbed_len
	dgmlen = lenStruct.unpack_from(chunk,dgmlenver-4)[0] # should be 4 more then len

	outputstr = "\n%.8f %.8f %d\n" % (latitude_deg, longitude_deg, millisec)
	outfile.write(outputstr)
	sbed_start = seabedImageStart # This is the pointer to the start of the seabed image for current beam
	no_sbed_found = 0
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
			
# THIS IS IT. This is where we output seabed image
		plat = latitude_deg + deltaLatitude_deg
		plon = longitude_deg + deltaLongitude_deg
		next_sbd_start = sbed_start + (2 * SInumSamples)
		if (y_reRefPoint_m < 0): # Reverse the output of the samples, see documentation
			sbed_start = next_sbd_start - 2
			center_samp = SInumSamples - SIcentreSample
		else:
			center_samp = SIcentreSample

		for n in range (0, SInumSamples):
			no_sbed_found += 1
			if (n == center_samp): # Put in position of center sample
				outputstr = " (%.8f %.8f) " % (plat, plon)
				outfile.write(outputstr)
			sbed_sample = seabedStruct.unpack_from(chunk, sbed_start)[0]
			outputstr = " %d" % (sbed_sample)
			outfile.write(outputstr)
			if (y_reRefPoint_m < 0):
				sbed_start -= 2
			else:
				sbed_start += 2 # jump 2 bytes (short) forwards

		sbed_start = next_sbd_start
		# There are 9 samples per extra detection, and there may be 2 bytes padding at the end
		if (i > 398):
			break_me_here = 0 # for debugging purposes


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

clear()
files = glob.glob('*.kmall')
for file in files:
	try:
		f = open(file, 'rb')
		nfile=file+".seabedimage"
		outfile=open(nfile,'w', encoding='utf-8')
		print ("Processing file: %s" %(file))
	except Exception:
		print('File',file,'not opened.')
		sys.exit(0)
	
# Process the file:
	f.seek(0, 2)
	file_size = f.tell()
	f.seek(0, 0)
	remaining = file_size

# Read all datagrams and process each of them
	while (remaining > 0):
# First read 4 bytes that contains the length of the chunk
		lengtha = struct.unpack("I",f.read(4))
		remaining -= 4
# Then read the chunk. Note that the length read includes the 4 bytes in the integer.
		dgmsize = lengtha[0] - 4
		chunk = f.read(dgmsize)
		remaining -= dgmsize
# Then process this chunk
		processDatagram(dgmsize, chunk)
	
	f.close()
	outfile.close()
	
clear()


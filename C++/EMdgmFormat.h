/*h+*/
/*!
 \file    EMdgmFormat.h

 \brief  Definitions of structs used for building EM output datagrams of *.kmall format. 


 \copyright     2017 Kongsberg Maritime AS


 \date    March 01 2017

 */

/*

  Revision History:

  01  01 SEP 2016  Rev A.
  02  01 MAR 2017  Rev B.
  03  05 JUL 2017  Rev C.
  04  08 DES 2017  Rev D.
  05  25 MAY 2018  Rev E.
  06  17 OCT 2018  Rev F beta.
 */

/*! \mainpage Overview of EM datagrams on .*kmall format
 * 
 * <b>Introduction</b>
 * <br>
 * The K-Controller is responsible for logging all the data from the
 * Kongsberg multibeam echosounder systems, MBES, also refered to as the EM series echosounders. K-Controller runs on a PC-architecture computer running Windows. 
 * Data from the MBES arrives at the PC via an Ethernet connection as UDP packets of variable length. 
 * The UDP packages containing output datagrams are usually logged to disk on a hydrographic work station (HWS). The output 
 * datagrams may also be exported to user provided programs on the HWS or on an external Ethernet 
 * network or from the echosounder by multicast distribution. <br>
 * <br>
 * To separate the EM datagrams described herein from previous EM datagram formats, the file extension
 * of this format is called *.kmall. Files are named according to nnnn_YYYYMMDD_HHMMSS, where nnnn is 
 * the line number, followed by date and time. Time in the first datagram in the file is decoded and used in
 * the filename. (Note that the clock in the PC is not used).The ship's name may be appended at the end of the file 
 * name by operators choice: nnnn_YYYYMMDD_HHMMSS_ShipName.<br>
 * <br>
 * The majority of the output datagrams are in binary format. The datagrams have 4 bytes alignment. Little 
 * endian and C++11 data types are used. All datagrams starts with the general header, EMdgmHeader_def, and ends with a 32 bit 
 * unsigned value containing the size of the datagram, in number of bytes. The length field at the start and end
 * of the datagram are included in the length count. Thus to read the file, first read four bytes, then
 * read that number of bytes from the file which is then one datagram. <br>
 * <br>
 * In the general header, the name of the datagram is found. The name of the datagram start by #, then a three letter code follows. 
 * Tables of output datagrams are given below.<br>
 * <br>
 * The datagrams from the echosounder arrives at the PC in one stream. The timestamp of
 * the data may not be strictly sequential. For example motion sensor data is grouped
 * together and sent from the echosounder to SIS/K-Controller in one datagram containing up to 100 motion sensor
 * measurements with individual timestamps. Therefore they may arrive one second later than
 * the depth datagram. This makes it necessary to read several motion sensor datagrams to
 * find the correct one to be used when applying motion to the depth data. The individual
 * datagram types must be handled separately and the interpolation to other datagram types
 * must be done based on the timestamps.<br>
 * <br>
 * The external sensor (S) datagrams contain information from sensors other than the echo sounder. The datagrams may contain both corrected 
 * data and data as received from the sensor. The corrected data are data as used during the raytracing and depth calculations by 
 * the processing unit (PU) in real-time. Corrected data are only provided for active sensors. For post processing purposes the raw data 
 * are provided, also for inactive sensors. Generally, raw data has the same format as the external sensors provide. E.g. raw position data 
 * from GGA are presented as a text string according to the NMEA standard:  
 * <i> $INGGA,094354.04,5925.695652,N,01027.921711,E,4,07,1.3,-1.55,M,40.51,M,1.0,0000*7D </i><br>
 * <br>
 * <br>
 * <b>Lists of output datagrams from processing unit (PU): </b><br>
 * <br>
 * <br>
 *<b>Installation and runtime datagrams:</b><br>
 * Datagram type code    | Datagram struct name | Description                          
 * ----------------------|--------------------- |--------------------------------------
 * #IIP                  | EMdgmIIP_def        | Installation parameters and sensor setup. 
 * #IOP                  | EMdgmIOP_def        | Runtime parameters as chosen by operator. 
 * #IBE                  | EMdgmIB_def         | Built in test (BIST) error report.        
 * #IBR                  | EMdgmIB_def         | Built in test (BIST) reply.              
 * #IBS                  | EMdgmIB_def         | Built in test (BIST) short reply.        
 * <br>
 *<b>Table of multibeam datagrams:</b><br>
 * Datagram type code    | Datagram struct name | Description                          
 * ----------------------|--------------------- |--------------------------------------
 * #MRZ                  | EMdgmMRZ_def        | Multibeam (M) raw range (R) and depth(Z) datagram 
 * #MWC                  | EMdgmMWC_def        | Multibeam (M) water (W) column (C) datagram       
 * 
 * <br>
 * <b>Table of external sensor output datagrams:</b><br>
 *  Datagram type code   | Datagram struct name| Description                              
 * ----------------------|---------------------|------------------------------------------
 * #SPO                  | EMdgmSPO_def      | Sensor (S) data for position (PO)      
 * #SKM                  | EMdgmSKM_def      | Sensor (S) KM binary sensor format     
 * #SVP                  | EMdgmSVP_def      | Sensor (S) data from sound velocity (V) profile (P) or CTD
 * #SVT                  | EMdgmSVT_def      | Sensor (S) data for sound velocity (V) at transducer (T)
 * #SCL                  | EMdgmSCL_def      | Sensor (S) data from clock (CL)        
 * #SDE                  | EMdgmSDE_def      | Sensor (S) data from depth (DE) sensor 
 * #SHI                  | EMdgmSHI_def      | Sensor (S) data for height (HI)        
 *
 * <br>
 * <b>Table of compatibility datagrams:</b><br>
 *  Datagram type code   | Datagram struct name| Description                              
 * ----------------------|---------------------|------------------------------------------     
 * #CPO                  | EMdgmCPO_def      | Compatibility (C) data for position (PO)
 * #CHE                  | EMdgmCHE_def      | Compatibility (C) data for heave (HE)
 * 
 *  <br>
 *  <br>
 *  <br>
 *  <br>
 */


/*! \page Datagram Format philosophy
 <b>Datagram format philosoph</b><br>


*/
/*! \page Datagram format philosophy
\TAGFILES = "C:/prosjekter/workspace_git/UmapShared/Documents/DatagramFormat2015_dOxygen/images/EgenInnledning.pdf"


*/


/*! \page advanced Coordinate systems
 <b>Datagram Type description</b><br>


*/


/*! \page advanced Coordinate systems
 <b>Vessel Coordinate System (VCS)</b><br>
 Origo of the VCS is the vessel reference point. The VCS is defined according to the right hand rule, see figure 1.<br>
 x-axis pointing forward parallel to the vessel main axis.<br>
 y-axis pointing starboard parallel to the deck plane.<br>
 z-axis pointing down parallel to the mast. <br>

 Rotation of the vessel coordinate system around an axis is defined as positive in the clockwise 
 direction, also according to the right hand rule. <br>
 Roll - rotation around the x-axis.<br> 
 Pitch - rotation around the y-axis.<br>
 Heading - rotation around the z-axis. Heading as input in depth calculations is sensor data referred to true north.<br>

 \image html RightHandRule_small.jpg   "Figure 1: The right hand rule coordinate system. Characters r, p and h denote roll, pitch and heading, respectively."
 \image latex RightHandRule_small.eps   "Figure 1: The right hand rule coordinate system. Characters r, p and h denote roll, pitch and heading, respectively."

 <br>
 <br>

 <b>Array Coordinate System (ACS)</b><br>
 Origo of the ACS is at the centre of the array face. The ACS is defined according to the right hand rule.<br>
 x-axis pointing forward along the array (parallel to the vessel main axis).<br>
 y-axis pointing starboard along the array plane.<br>
 z-axis pointing down orthogonal to the array plane.<br>

 <b>Surface Coordinate System (SCS)</b><br>
 Origo of the SCS is the vessel reference point at the time of transmission. The SCS is defined according to the right hand rule.<br>
 x-axis pointing forward along the horizontal projection of the vessel main axis.<br>
 y-axis pointing horizontally to starboard, orthogonal to the horizontal projection of the vessel main axis.<br>
 z-axis pointing down along the g-vector. <br>
 To move SCS into the waterline, use reference point height corrected for roll and pitch at the time of transmission.<br>

 <b>Fixed Coordinate System (FCS)</b><br>
 Origo of the FCS is fixed somewhere in the nominal sea surface. The FCS is defined according to the right hand rule.<br>
 x-axis pointing north.<br>
 y-axis pointing east.<br>
 z-axis pointing down along the g-vector. <br>

 \image html Coordinate_systems_small.jpg
 \image latex Coordinate_systems_small.eps
 *  <br>
 *  <br>
 *  <br>
 *  <br>
 *  Se also \ref advanced2 "Reference points and offsets"
 *  <br>
\ref mainpage "Back to main page"
 */


/*! \page advanced2 Reference points and offsets
 *  <b> Time </b><br>
 *  Datagram time stamp for multibeam datagrams are the time of the midpoint of the first transmitted pulse of the ping. 
 *  All delays are referred to this time if not specifically stated. <br>
 *  For sensor datagrams, the timestamp in the datagram header is timestamped when received in PU.  
 *  <br>
 *  <br>
 *  <b> Transducer orientation</b><br>
 *  Angular offsets (installation angles) for TX and RX are defined the angular offset between the array coordinate system and the vessel coordinate system. 
 *  For the EM 2040 echosounder, the default orientation of the transducers are with the cable plug facing the port side (TX) and forward (RX) (i.e. if standing on the 
 *  boats deck, face in the boats forward direction, and looking downwards). <br>
 *  <br>
 *  \image html defaultTXRXorient.png
 *  <br>
 *  If the orientation of the EM 2040 TX is with the cable plug to the starboard (or aft for RX), the installation heading is 180 degrees in the #IIP datagram. 
 *  This misalignment must be indicated in the K-Controller, under 'Installation', 'Angular offset' menu, by choosing the right configuration  (figure TX Transducer: Orientation). 
 *  The Offset angles (deg.) for TX and RX in the 'Angular offset' menu in the K-Controller are then relative to the orientation indicated in the figures in the menu. 
 *  However, when processing in the PU, the heading angles are 180 + heading offset.
 *  <br>
 *  <br>
 *  <b> Transducer subarrays</b><br>
 *  For most EM type echo sounders, all elements in the TX and RX transducer arrays are
 * used for all beams, giving a common x,y,z location for each transducer. But for the EM
 * 2040 this is different. The TX transducer has 3 separate arrays, used for port, centre and
 * starboard sectors, and the RX array also has an along-ship offset.<br><br>
 * The transducer subarray offsets x,y,z are taken into account by the PU when calculating the #MRZ datagram. If 
 * it is needed to recalculate XYZ data in post-processing, these subarray offsets should also be taken into account.<br>
 * For the EM 2040, the transducer installation coordinates refers to the center of the
 * transducer faces. The installation coordinates are given in the Installation parameters.<br><br>
 * The figure below illustrates the offsets of the 3 subarrays for EM2040. The TX transducer is seen from behind (default orientation). Subarray 0 = port subarray,
 * 1 = centre subarray, 2 = starboard subarray. Here port/starboard refer to the array coordinate system. E.g. if the TX is installed with a heading offset of 180 degrees, the port subarray (ref. array, subarray 0) 
 * will be on the vessels starboard side.
 * <br>
 *  \image html txSubArrays_EM2040.png
 *  <b> Vessel reference point </b><br>
 *  Sensor data as well as the installation offsets (location offsets and angular offsets) of the sensors, are all 
 *  referred to the vessel coordinate system. 
 *  Sensor data that are described as corrected have been adjusted to compensate for the offset between 
 *  the sensor and the vessel reference point. The offsets to be used for 
 *  correction are set by the operator in K-Controller, Installation.
 *  If data are described as raw, the offsets and delays have not been applied, 
 *  and data in the output datagrams are as received from input sensors.<br>
 *  <br>
 *  The beam data (x, y and z) are referred to the vessel reference point, and in the surface coordinate system (SCS). Heave, roll, pitch,
 *  sound speed at the transducer depth and ray bending through the water column have
 *  been applied. The distance from the vessel reference point to the water lever (z_waterLevelReRefPoint_m) should be subtracted from 
 *  the beam depths to derive the depths re the water line. See also \ref advanced "Coordinate systems"<br>
 *  <br>
 *  <br>
 *  <br>
 *  <br> 
 */


/*! \page documentInformation Document information
 * <b>Document information</b><br>
 * <ul>
 * <li>Product: EM <br>
 * <li>Document: EM datagram on *.kmall format <br>
 * <li>Document number: 410224  <br>
 * <li>Revision: F beta <br>
 * <li>Date of issue: October 2018  <br>
 * </ul>
 * <br>
 * <b>Copyright</b><br>
 * The information contained in this document remains the sole property of Kongsberg Maritime AS. 
 * No part of this document may be copied or reproduced in any form or by any means, and the information 
 * contained within it is not to be communicated to a third party, without the prior written consent of Kongsberg Maritime AS.<br>
 * <br>
 * <b>Warning</b><br>
 * The equipment to which this manual applies must only be used for the purpose for which it was designed. 
 * Improper use or maintenance may cause damage to the equipment and/or injury to personnel. The user must 
 * be familiar with the contents of the appropriate manuals before attempting to operate or work on the equipment.<br>
 * <br>
 * Kongsberg Maritime disclaims any responsibility for damage or injury caused by improper installation, use 
 * or maintenance of the equipment.<br>
 * <br>
 * <b>Disclaimer</b><br>
 * Kongsberg Maritime AS endeavours to ensure that all information in this document is correct and fairly stated, 
 * but does not accept liability for any errors or omissions.<br>
 *  <br>
 *  <br>
 *  <br>
 *  <br> 
 */


#ifndef _EMDGMFORMAT_H
#define _EMDGMFORMAT_H

#include "stdint.h"

#ifndef _VXW
#pragma pack(4)
#endif

#define EM_DGM_FORMAT_VERSION "Rev F beta 2018-10-17" /*!< EM datagrams on *.kmall format version with date. */

#define MAX_NUM_BEAMS 1024 /*!< Maximum number of beams. Actual number of beams in datagram given by parameter numSoundingsMaxMain in struct EMdgmMRZ_rxInfo_def.*/
#define MAX_EXTRA_DET 1024 /*!< Maximum number of extra detections. Actual number of extra detections in datagram given by parameter numExtraDetections in struct EMdgmMRZ_rxInfo_def.*/
#define MAX_EXTRA_DET_CLASSES 11   /*!< Maximum number of classes of extra detections. Actual number of extra detection classes in datagram given by parameter numExtraDetectionClasses in struct EMdgmMRZ_rxInfo_def.*/
#define MAX_SIDESCAN_SAMP 60000   /*!< Maximum number of seabed image / sidescan data samples. Actual number to be found in parameter SInumSamples in struct EMdgmMRZ_sounding_def.*/
#define MAX_SIDESCAN_EXTRA_SAMP 15000   /*!< Maximum number of extra seabed image / sidescan data samples at the port and starboard flanks.*/
#define MAX_NUM_TX_PULSES 9  /*!< Maximum number of TX pulses per ping. Actual length in datagram given by parameter numTxSectors in struct EMdgmMRZ_pingInfo_def.*/
#define MAX_ATT_SAMPLES  148 /*!< Maximum number of attitude samples allowed in one datagram. Actual length in datagram given by parameter numSamplesArray in struct EMdgmSdataInfo_def*/
#define MAX_SVP_POINTS 2000  /*!< Maximum number of sound velocity profile depth points. Actual length in datagram given by numSamples in the struct EMdgmSVP_def*/
#define MAX_SVT_SAMPLES 1  /*!< Maximum number of sound velocity probe samples. Actual length in datagram given by numSamplesArray in the struct EMdgmSVTinfo_def*/
#define MAX_DGM_SIZE 64000  /*!< Maximum length for datagram / UDP packet. */
#define MAX_NUM_MST_DGMS 256 /*!< Maximum number of #MST datagram partitions that can be sent from PU. See struct EMdgmMpartition_def.*/
#define MAX_NUM_MWC_DGMS 256 /*!< Maximum number of #MWC datagram partitions that can be sent from PU. See struct EMdgmMpartition_def.*/
#define MAX_NUM_MRZ_DGMS 32  /*!< Maximum number of #MRZ datagram partitions that can be sent from PU. See struct EMdgmMpartition_def.*/

#define MAX_SPO_DATALENGTH 250  /*!< Maximum length (bytes) for sensor data from position sensor. */
#define MAX_ATT_DATALENGTH 250  /*!< Maximum length (bytes) for sensor data from attitude sensor. */
#define MAX_SVT_DATALENGTH 64   /*!< Maximum length (bytes) for sensor data from sensor for sound velocity at transducer.  */
#define MAX_SCL_DATALENGTH 64   /*!< Maximum length (bytes) for sensor data from clock sensor. */
#define MAX_SDE_DATALENGTH 32   /*!< Maximum length (bytes) for sensor data from depth sensor. */
#define MAX_SHI_DATALENGTH 32   /*!< Maximum length (bytes) for sensor data from height sensor. */

#define MAX_CPO_DATALENGTH 250  /*!< Maximum length (bytes) for sensor data from position sensor (compatibility). */
#define MAX_CHE_DATALENGTH 64   /*!< Maximum length (bytes) for heave data used for water column (compatibility). */

#define UNAVAILABLE_POSFIX 0xffff /*!< Unavailable sensor data, output to datagram. */
#define UNAVAILABLE_LATITUDE 200.0f /*!< Unavailable sensor data, output to datagrams. */
#define UNAVAILABLE_LONGITUDE 200.0f /*!< Unavailable sensor data, output to datagram. */
#define UNAVAILABLE_SPEED -1.0f /*!< Unavailable sensor data output, to datagram. */
#define UNAVAILABLE_COURSE -4.0f /*!< Unavailable sensor data output, to datagram. */
#define UNAVAILABLE_ELLIPSOIDHEIGHT -999.0f /*!< Unavailable sensor data output, to datagram. */
/*
 * 
 */
/*********************************************
            Datagram names
 *********************************************/

/* I - datagrams */
#define EM_DGM_I_INSTALLATION_PARAM        "#IIP"	
#define EM_DGM_I_OP_RUNTIME                "#IOP"	

/* S-datagrams */
#define EM_DGM_S_POSITION                  "#SPO"
#define EM_DGM_S_KM_BINARY                 "#SKM"
#define EM_DGM_S_SOUND_VELOCITY_PROFILE    "#SVP"
#define EM_DGM_S_SOUND_VELOCITY_TRANSDUCER "#SVT"
#define EM_DGM_S_CLOCK                     "#SCL"
#define EM_DGM_S_DEPTH                     "#SDE"
#define EM_DGM_S_HEIGHT                    "#SHI"


/* M-datagrams */
#define EM_DGM_M_RANGE_AND_DEPTH        "#MRZ"  
#define EM_DGM_M_WATER_COLUMN           "#MWC"


/* C-datagrams */
#define EM_DGM_C_POSITION         "#CPO"
#define EM_DGM_C_HEAVE            "#CHE"




/*********************************************

   General datagram header        

 *********************************************/

/*! 
 *  \brief  Definition of general datagram header.*/
struct EMdgmHeader_def
{	
	uint32_t numBytesDgm;    	/*!< Datagram length in bytes. The length field at the start (4 bytes) and end of the datagram (4 bytes) are included in the length count. */
	uint8_t dgmType[4];	 		/*!< Multi beam datagram type definition, e.g. #AAA  */
	uint8_t dgmVersion;         /*!< Datagram version. */
	uint8_t systemID;			/*!< System ID. Parameter used for separating datagrams from different echosounders if more than one system is connected to SIS/K-Controller.  */
	uint16_t echoSounderID;  	/*!< Echo sounder identity, e.g. 122, 302, 710, 712, 2040, 2045, 850. */
	uint32_t time_sec;          /*!< UTC time in seconds. Epoch 1970-01-01. time_nanosec part to be added for more exact time.*/
	uint32_t time_nanosec;      /*!< Nano seconds remainder. time_nanosec part to be added to time_sec for more exact time.*/
};

typedef struct EMdgmHeader_def EMdgmHeader, *pEMdgmHeader;

/********************************************* 

   Sensor datagrams    

 *********************************************/

/*!
 *  \brief  Sensor (S) output datagram - common part for all external sensors. */
struct EMdgmScommon_def
{
	uint16_t numBytesCmnPart;  /*!< Size in bytes of current struct. Used for denoting size of rest of datagram in cases where only one datablock is attached.*/
	uint16_t sensorSystem;     /*!< Sensor system number, as indicated when setting up the system in K-Controller installation menu. 
	                                 E.g. position system 0 referes to system POSI_1 in installation datagram #IIP. Check if this sensor system is active by using #IIP datagram. 
	                                 <br><br>
	                                 #SCL - clock datagram:<br>
	                                  Bit number | Sensor system
                                    ------------|-------------------
                                              0 | Time syncronisation from clock data
                                              1 | Time syncronisation from active position data
                                              2 | 1 PPS is used
                                               
	                                 */
	uint16_t sensorStatus;     /*!< Sensor status. To indicate quality of sensor data is valid or invalid. Quality may be invalid even if sensor is active and the PU receives data.
	                                Bit code vary according to type of sensor. <br>
	                                <br>
	                                Bits 0 -7 common to all sensors and #MRZ sensor status:<br>
	                            
                                     Bit number | Sensor data
                                    ------------|-------------------
                                              0 | 0 = Data OK <br> 1 = Data OK and sensor is chosen as active <br> #SCL only: 1 = Valid data and 1PPS OK
                                              1 | 0
                                              2 | 0 = Data OK <br> 1 = Reduced performance <br> #SCL only: 1 = Reduced performance, no time synchronisation of PU
                                              3 | 0
                                              4 | 0 = Data OK <br> 1 = Invalid data
                                              5 | 0
                                              6 | 0 = Velocity from sensor <br> 1 = Velocity calculated by PU
											  7 | 0
                                                                                          
                                For #SPO (position) and CPO (position compatibility) datagrams, bit 8 - 15:<br>                                                           
                                     Bit number | Sensor data
                                    ------------|-------------------
                                              8 | 0
                                              9 | 0 = Time from PU used (system) <br> 1 = Time from datagram used (e.g. from GGA telegram)
                                             10 | 0 = No motion correction <br> 1 = With motion correction
                                             11 | 0 = Normal quality check <br> 1 = Operator quality check. Data always valid
                                             12 | 0 
                                             13 | 0
                                             14 | 0
                                             15 | 0                           
                                    */
	uint16_t padding;
};

typedef struct EMdgmScommon_def EMdgmScommon, *pEMdgmScommon;

/*!
 * \brief Information of repeated sensor data in one datagram.
 * 
 * Info about data from sensor. Part included if data from sensor appears multiple times in a datagram.*/
struct EMdgmSdataInfo_def
{
	uint16_t numBytesInfoPart;  /*!< Size in bytes of current struct. */
	uint16_t numSamplesArray;  /*!< Number of sensor samples added in datagram.*/
	uint16_t numBytesPerSample;  /*!< Length in bytes of one whole sample (decoded and raw data).*/
	uint16_t numBytesRawSensorData; /*!< Length in bytes of raw sensor data. */
};

typedef struct EMdgmSdataInfo_def EMdgmSdataInfo, *pEMdgmSdataInfo;


/************************************
   #SPO - Sensor Position data       
 ************************************/
/*!
 *  \brief #SPO - Sensor position data block. Data from active sensor is corrected data for position system installation parameters. Data is also 
 *  corrected for motion ( roll and pitch only) if enabled by K-Controller operator. Data given both decoded and corrected (active sensors), and raw as received from sensor in text string. 
 *  */
struct EMdgmSPOdataBlock_def
{ 
	uint32_t timeFromSensor_sec;            /*!< UTC time from position sensor. Unit seconds. Epoch 1970-01-01. Nanosec part to be added for more exact time.*/
	uint32_t timeFromSensor_nanosec;  	    /*!< UTC time from position sensor. Unit nano seconds remainder.*/
	float posFixQuality_m;  /*!< Only if available as input from sensor. Calculation according to format.*/
	double correctedLat_deg;   /*!< Motion corrected (if enabled in K-Controller) data as used in depth calculations. Referred to vessel reference point. Unit decimal degree. Parameter is set to define #UNAVAILABLE_LATITUDE if sensor inactive.*/ 
	double correctedLong_deg;   /*!< Motion corrected (if enabled in K-Controller) data as used in depth calculations. Referred to vessel reference point. Unit decimal degree. Parameter is set to define #UNAVAILABLE_LONGITUDE if sensor inactive.*/
	float speedOverGround_mPerSec;  /*!< Speed over ground. Unit m/s. Motion corrected (if enabled in K-Controller) data as used in depth calculations. If unavailable or from inactive sensor, value set to define #UNAVAILABLE_SPEED. */
	float courseOverGround_deg;   /*!< Course over ground. Unit degree. Motion corrected (if enabled in K-Controller) data as used in depth calculations. If unavailable or from inactive sensor, value set to define #UNAVAILABLE_COURSE. */
	float ellipsoidHeightReRefPoint_m;  /*!< Height of vessel reference point above the ellipsoid. Unit meter. Motion corrected (if enabled in K-Controller) data as used in depth calculations. If unavailable or from inactive sensor, value set to define #UNAVAILABLE_ELLIPSOIDHEIGHT.  */
	int8_t posDataFromSensor[MAX_SPO_DATALENGTH]; /*!< Position data as received from sensor, i.e. uncorrected for motion etc. */
};

typedef struct EMdgmSPOdataBlock_def EMdgmSPOdataBlock, *pEMdgmSPOdataBlock;

/*!
 *  \brief   #SPO - Struct of position sensor datagram.   <br>
 *  <br>
 *  Data from active sensor will be motion corrected if indicated by operator. Motion correction is applied to latitude, longitude, speed, course and ellipsoidal height.
 *  If the sensor is inactive, the fields will be marked as unavailable, defined by the parameters define #UNAVAILABLE_LATITUDE etc.
 *  */
struct EMdgmSPO_def
{
	struct EMdgmHeader_def header;
	struct EMdgmScommon_def cmnPart;
	struct EMdgmSPOdataBlock_def sensorData;
};

#define SPO_VERSION 0
typedef struct EMdgmSPO_def EMdgmSPO, *pEMdgmSPO;


/************************************
   #SKM - KM binary sensor data       
 ************************************/
/*!
 *  \brief  Sensor (S) output datagram - info of KMB datagrams. */
struct EMdgmSKMinfo_def
{
	uint16_t numBytesInfoPart;  /*!< Size in bytes of current struct. Used for denoting size of rest of datagram in cases where only one datablock is attached.*/
	uint8_t sensorSystem;     /*!< Attitude system number, as numbered in installation parameters. E.g. system 0 referes to system ATTI_1 in installation datagram #IIP. */  
	uint8_t sensorStatus;     /*!< Sensor status. Summarise the status fields of all KM binary samples added in this datagram (status in struct KMbinary_def). 
	                               Only available data from input sensor format is summarised. Available data found in sensorDataContents. <br>
                                   Bits 0 -7 common to all sensors and #MRZ sensor status:<br>
	                            
                                     Bit number | Sensor data
                                    ------------|-------------------
                                              0 | 0 = Data OK <br> 1 = Data OK and sensor is chosen as active
                                              1 | 0
                                              2 | 0 = Data OK <br> 1 = Reduced performance
                                              3 | 0
                                              4 | 0 = Data OK <br> 1 = Invalid data
                                              5 | 0
                                              6 | 0 = Velocity from sensor <br> 1 = Velocity calculated by PU
                                              
	 */

	uint16_t sensorInputFormat;  /*!< Format of raw data from input sensor, given in numerical code according to table below. 
		                              Code | Sensor format
		                              -----|--------------
		                                 1 | KM binary Sensor Input  
		                                 2 | EM 3000 data
		                                 3 | Sagem
		                                 4 | Seapath binary 11
		                                 5 | Seapath binary 23
		                                 6 | Seapath binary 26
		                                 7 | POS M/V GRP 102/103
		                                 8 | Coda Octopus MCOM  
		                                 */

	uint16_t numSamplesArray;  /*!< Number of KM binary sensor samples added in this datagram.*/
	uint16_t numBytesPerSample;  /*!< Length in bytes of one whole KM binary sensor sample.*/
	uint16_t sensorDataContents; /*!< Field to indicate which information is available from the input sensor, at the given sensor format. <br> <br>
	                                 0 = not available <br>
	                                 1 = data is available <br> <br>
	                                 The bit pattern is used to detemine sensorStatus from status field in #KMB samples. 
	                                 Only data available from sensor is check up against invalid/reduced performance in status, and summaries in sensorStatus. 
	                                 <br><br>
	                                 E.g. the binary 23 format does not contain delayed heave. This is indicated by setting bit 6 in sensorDataContents to 0. In each sample in #KMB output from PU, the status field (struct KMbinary_def) 
	                                 for INVALID delayed heave (bit 6) is set to 1. The summaries sensorStatus in struct EMdgmSKMinfo_def will then be sets to 0 if all available data is ok. <br>
	                                 <br>
	                                 Expected data field in sensor input:<br>
                                     <br>
                                     Bit number | Sensor data
                                     -----------|-------------------
                                              0 | Horizontal position and velocity
                                              1 | Roll and pitch
                                              2 | Heading
                                              3 | Heave and vertical velocity
                                              4 | Acceleration
                                              5 | Error fields
                                              6 | Delayed heave
	 */
};

typedef struct EMdgmSKMinfo_def EMdgmSKMinfo, *pEMdgmSKMinfo;
/*!
 *  \brief #SKM - Sensor attitude data block. Data given timestamped, not corrected. 
 *  
 *  See \ref advanced "Coordinate systems" for definition of positive angles and axis.
 *  */
struct KMbinary_def
{
	uint8_t dgmType[4];	 		/*!< #KMB  */
	uint16_t numBytesDgm;    	/*!< Datagram length in bytes. The length field at the start (4 bytes) and end of the datagram (4 bytes) are included in the length count. */
	uint16_t dgmVersion;         /*!< Datagram version. */
	uint32_t time_sec;       /*!< UTC time from inside KM sensor data. Unit second. Epoch 1970-01-01 time. Nanosec part to be added for more exact time.
	                             <br> <br> If time is unavailable from attitude sensor input, time of reception on serial port is added to this field.*/
	uint32_t time_nanosec;   /*!< Nano seconds remainder. Nanosec part to be added to time_sec for more exact time.
	                          <br> <br> If time is unavailable from attitude sensor input, time of reception on serial port is added to this field.*/
	uint32_t status;   /*!< Bit pattern for indicating validity of sensor data, and reduced performance. The status word consists of 32 single bit flags numbered from 0 to 31, where 0 is the least significant bit. <br>
	                       Bit number 0-7 indicate if from a sensor data is invalid. 0 = valid data, 1 = invalid data. <br>
	                       Bit number 16-> indicate if data from sensor has reduced performance. 0 = valid data, 1 = reduced performance. <br>
	                       <br>

	                       Invalid data:<br>
	                       <br>
	                       Bit number | Sensor data
	                       -----------|-------------------
	                                0 | Horizontal position and velocity
	                                1 | Roll and pitch
	                                2 | Heading
	                                3 | Heave and vertical velocity
	                                4 | Acceleration
	                                5 | Error fields
	                                6 | Delayed heave

	                      <br>
	                       Reduced performance:<br>
	                       <br>

	                       Bit number | Sensor data
	                       -----------|-------------------
	                               16 | Horizontal position and velocity
	                               17 | Roll and pitch
	                               18 | Heading
	                               19 | Heave and vertical velocity
	                               20 | Acceleration
	                               21 | Error fields
	                               22 | Delayed heave

	 */
	/**@}*/
	/*! \name Position. */
	/**@{*/
	double latitude_deg;   /*!< Position in decimal degrees.*/
	double longitude_deg;  /*!< Position in decimal degrees.*/
	float ellipsoidHeight_m;  /*!< Height of sensor reference point above the ellipsoid. Positive above ellipsoid. ellipsoidHeight_m is not corrected for motion and installation offsets of the position sensor. */
	/**@}*/
	/*! \name Attitude. */
	/**@{*/
	float roll_deg;     /*!< Roll. Unit degree. */
	float pitch_deg;    /*!< Pitch. Unit degree. */
	float heading_deg;  /*!< Heading of vessel. Unit degree. Relative to the fixed coordinate system, i.e. true north. */
	float heave_m;  /*!< Heave. Unit meter. Positive downwards.*/

	/**@}*/
	/*! \name Rates. */
	/**@{*/
	float rollRate;  /*!< Roll rate. Unit degree/s*/
	float pitchRate;  /*!< Pitch rate. Unit degree/s*/
	float yawRate;  /*!< Yaw (heading) rate. Unit degree/s*/

	/**@}*/
	/*! \name Velocities. */
	/**@{*/
	float velNorth; /*!< Velocity North (X). Unit m/s*/
	float velEast;  /*!< Velocity East (Y). Unit m/s*/
	float velDown;  /*!< Velocity downwards (Z). Unit m/s*/

	/**@}*/
	/*! \name Errors in data. Sensor data quality, as standard deviations. */
	/**@{*/
	float latitudeError_m;   /*!< Latitude error. Unit meter. */
	float longitudeError_m;   /*!< Longitude error. Unit meter. */
	float ellipsoidHeightError_m;   /*!< Ellipsoid height error. Unit meter. */
	float rollError_deg;     /*!< Roll error. Unit degree. */
	float pitchError_deg;   /*!< Pitch error. Unit degree. */
	float headingError_deg;  /*!< Heading error. Unit degree. */
	float heaveError_m;   /*!< Heave error. Unit meter. */

	/**@}*/
	/*! \name Acceleration.  */
	/**@{*/
	float northAcceleration;   /*!< Unit m/s^2. */
	float eastAcceleration;    /*!< Unit m/s^2. */
	float downAcceleration;    /*!< Unit m/s^2. */
};

typedef struct KMbinary_def KMbinary, *pKMbinary;

/*!
 *  \brief #SKM - delayed heave. Included if available from sensor. 
 */
struct KMdelayedHeave_def
{
	uint32_t time_sec;
	uint32_t time_nanosec;
	float delayedHeave_m;  /*!< Delayed heave. Unit meter.*/
};
typedef struct KMdelayedHeave_def KMdelayedHeave, *pKMdelayedHeave;

/*!
 *  \brief #SKM - all available data.  
 */
struct EMdgmSKMsample_def
{
	struct KMbinary_def KMdefault;
	struct KMdelayedHeave_def delayedHeave;
};

typedef struct EMdgmSKMsample_def EMdgmSKMsample, *pEMdgmSKMsample;

/*!
 *  \brief #SKM - data from attitude and attitude velocity sensors. 
 *  
 *  Datagram may contain several sensor measurements. The number of samples in datagram is listed in numSamplesArray in the struct EMdgmSKMinfo_def. 
 *  Time given in datagram header, is time of arrival of data on serial line or on network. Time inside #KMB sample is time
 *  from the sensors data. 
 *  If input is other than KM binary sensor input format, the data are converted to the KM binary format by the PU.  All parameters are uncorrected.
 *  For processing of data, installation offsets, installation angles and attitude values are needed to correct the data for motion.
 */
struct EMdgmSKM_def
{
	struct EMdgmHeader_def header;
	struct EMdgmSKMinfo_def infoPart;   	
	struct EMdgmSKMsample_def sample[MAX_ATT_SAMPLES];
};

#define SKM_VERSION 1
typedef struct EMdgmSKM_def EMdgmSKM, *pEMdgmSKM;


/************************************
    #SVP - Sound Velocity Profile       
 ************************************/
/*!
 *  \brief #SVP - Sound Velocity Profile. Data from one depth point contains information specified in this struct.
 *  
 *  */
struct EMdgmSVPpoint_def
{
	float depth_m;  /*!< Depth at which measurement is taken. Unit m. Valid range from 0.00 m to 12000 m. */
	float soundVelocity_mPerSec;  /*!< Measured sound velocity from profile. Unit m/s. For a CTD profile, this will be the calculated sound velocity. */
	uint32_t padding;  /*!< Former absorption coefficient. Voided. */
	float temp_C;     /*!< Water temperature at given depth. Unit Celsius. For a Sound velocity profile (S00), this will be set to 0.00.*/
	float salinity; /*!< Salinity of water at given depth. For a Sound velocity profile (S00), this will be set to 0.00.*/
};

typedef struct EMdgmSVPpoint_def EMdgmSVPpoint, *pEMdgmSVPpoint;

/*!
 *  \brief #SVP - Sound Velocity Profile. Data from sound velocity profile or from CTD profile. Sound 
 *  velocity is measured directly or estimated, respectively.
 *  
 *  */
struct EMdgmSVP_def
{
	struct EMdgmHeader_def header;
	uint16_t numBytesCmnPart;  /*!< Size in bytes of body part struct. Used for denoting size of rest of datagram.*/
	uint16_t numSamples;  /*!< Number of sound velocity samples. */
	                           
	uint8_t sensorFormat[4];      /*!< Sound velocity profile format: <br>
	                                'S00' = sound velocity profile <br>
	                                'S01' = CTD profile */
	uint32_t time_sec;     /*!< Time extracted from the Sound Velocity Profile.
                                    Parameter is set to zero if not found.*/
	double latitude_deg;  /*!<  Latitude in degrees. Negative if southern hemisphere. Position extracted from the Sound Velocity Profile.
	                                    Parameter is set to define #UNAVAILABLE_LATITUDE if not available.*/ 
	double longitude_deg; /*!<  Longitude in degrees. Negative if western hemisphere. Position extracted from the Sound Velocity Profile.
	                                    Parameter is set to define #UNAVAILABLE_LONGITUDE if not available.*/

	struct EMdgmSVPpoint_def sensorData[MAX_SVP_POINTS];  /*!< SVP point samples, repeated numSamples times. */

};

#define SVP_VERSION 1
typedef struct EMdgmSVP_def EMdgmSVP, *pEMdgmSVP;

/************************************
* #SVT - Sensor sound Velocity measured at Transducer
************************************/
/*!
*  \brief Part of Sound Velocity at Transducer datagram
*  */
struct EMdgmSVTinfo_def
{
    uint16_t numBytesInfoPart;  /*!< Size in bytes of current struct. Used for denoting size of rest of datagram in cases where only one datablock is attached.*/
    uint16_t sensorStatus;     /*!< Sensor status. To indicate quality of sensor data is valid or invalid. Quality may be invalid even if sensor is active and the PU receives data.
                                    Bit code vary according to type of sensor. <br>
                                    <br>
                                    Bits 0 -7 common to all sensors and #MRZ sensor status:<br>
                                
                                     Bit number | Sensor data
                                    ------------|-------------------
                                              0 | 0 = Data OK <br> 1 = Data OK and sensor is chosen as active <br> #SCL only: 1 = Valid data and 1PPS OK
                                              1 | 0
                                              2 | 0 = Data OK <br> 1 = Reduced performance <br> #SCL only: 1 = Reduced performance, no time synchronisation of PU
                                              3 | 0
                                              4 | 0 = Data OK <br> 1 = Invalid data
                                              5 | 0
                                              6 | 0 = Velocity from sensor <br> 1 = Velocity calculated by PU
    */
    uint16_t sensorInputFormat;  /*!< Format of raw data from input sensor, given in numerical code according to table below. 
                                      Code | Sensor format
                                      -----|--------------
                                         1 | AML NMEA  
                                         2 | AML SV
                                         3 | AML SVT
                                         4 | AML SVP
                                         5 | Micro SV
                                         6 | Micro SVT
                                         7 | Micro SVP
                                         8 | Valeport MiniSVS
    */
    uint16_t numSamplesArray;  /*!< Number of sensor samples added in this datagram.*/
    uint16_t numBytesPerSample;  /*!< Length in bytes of one whole SVT sensor sample.*/
    uint16_t sensorDataContents; /*!< Field to indicate which information is available from the input sensor, at the given sensor format. <br> <br>
                                     0 = not available <br>
                                     1 = data is available <br> <br>
                                     Expected data field in sensor input:<br>
                                     <br>
                                     Bit number | Sensor data
                                     -----------|-------------------
                                              0 | Sound Velocity
                                              1 | Temperature
                                              2 | Pressure
                                              3 | Salinity

     */
    float filterTime_sec; /*!< Time parameter for moving median filter. Unit seconds */
    float soundVelocity_mPerSec_offset; /*!< Offset for measured sound velocity set in K-Controller. Unit m/s */
};

/*!
*  \brief #SVT - Sound Velocity at Transducer. Data sample.
*  */
struct EMdgmSVTsample_def
{                        
    uint32_t time_sec;           /*!< Time in second. Epoch 1970-01-01. time_nanosec part to be added for more exact time.*/
    uint32_t time_nanosec;       /*!< Nano seconds remainder. time_nanosec part to be added to time_sec for more exact time.*/
    float soundVelocity_mPerSec; /*!< Measured sound velocity from sound velocity probe. Unit m/s. */
    float temp_C;   /*!< Water temperature from sound velocity probe. Unit Celsius.*/
    float pressure_Pa;  /*!< Pressure. Unit Pascal. */
    float salinity; /*!< Salinity of water. Measured in g salt/kg sea water  */
};

typedef struct EMdgmSVTsample_def EMdgmSVTsample, *pEMdgmSVTsample;

/*!
*  \brief #SVT - Sound Velocity at Transducer.  Data for sound velocity and temperature are measured directely on the sound velocity probe.
*  */
struct EMdgmSVT_def
{
    struct EMdgmHeader_def header;
    struct EMdgmSVTinfo_def infoPart;
    struct EMdgmSVTsample_def sensorData[MAX_SVT_SAMPLES];
};

#define SVT_VERSION 0
typedef struct EMdgmSVT_def EMdgmSVT, *pEMdgmSVT;

/************************************
    #SCL - Sensor CLock datagram
 ************************************/
/*!
 *  \brief Part of clock datagram giving offsets and the raw input in text format
 *  */
struct EMdgmSCLdataFromSensor_def
{
	float offset_sec;  /*!< Offset in seconds from K-Controller operator input. */
	int32_t clockDevPU_nanosec;    /*!< Clock deviation from PU. Difference between time stamp at receive of sensor data and 
	                                  time in the clock source. Unit nanoseconds. Difference smaller than +/- 1 second if 1PPS is active and
	                                  sync from ZDA. */
	uint8_t dataFromSensor[MAX_SCL_DATALENGTH];   /*!< Clock data as received from sensor, 
	                                                  in text format. Data are uncorrected for offsets.*/
};

typedef struct EMdgmSCLdataFromSensor_def EMdgmSCLdataFromSensor, *pEMdgmSCLdataFromSensor;

/*!
 *  \brief #SCL - CLock datagram.
 *  */
struct EMdgmSCL_def
{
	struct EMdgmHeader_def header;
	struct EMdgmScommon_def cmnPart;  
	struct EMdgmSCLdataFromSensor_def sensData;
};

#define SCL_VERSION 0
typedef struct EMdgmSCL_def EMdgmSCL, *pEMdgmSCL;


/************************************
    #SDE - Sensor Depth data       
 ************************************/
/*!
 *  \brief Part of depth datagram giving depth as used, offsets, scale factor and data as received from sensor (uncorrected).
 *  */
struct EMdgmSDEdataFromSensor_def
{
	float depthUsed_m;  /*!< Depth as used. Corrected with installation parameters. Unit meter. */
	float offset;  /*!< Offset used measuring this sample. */
	float scale;  /*!< Scaling factor for depth. */
	double latitude_deg;  /*!<  Latitude in degrees. Negative if southern hemisphere. Position extracted from the Sound Velocity Profile.
                                    Parameter is set to define #UNAVAILABLE_LATITUDE if not available from sensor.*/ 
	double longitude_deg; /*!<  Longitude in degrees. Negative if western hemisphere. Position extracted from the Sound Velocity Profile.
                                    Parameter is set to define #UNAVAILABLE_LONGITUDE if not if not available from sensor..*/ 
	uint8_t dataFromSensor[MAX_SDE_DATALENGTH];  /*!< */
};

typedef struct EMdgmSDEdataFromSensor_def EMdgmSDEdataFromSensor, *pEMdgmSDEdataFromSensor;

/*!
 *  \brief #SDE - Depth datagram.
 *  */
struct EMdgmSDE_def
{
	struct EMdgmHeader_def header;
	struct EMdgmScommon_def cmnPart;
	struct EMdgmSDEdataFromSensor_def sensorData;
};

#define SDE_VERSION 0
typedef struct EMdgmSDE_def EMdgmSDE, *pEMdgmSDE;

/************************************
    #SHI - Sensor Height data      
 ************************************/
/*!
 *  \brief Part of Height datagram, giving corrected and uncorrected data as received from sensor.
 *  */
struct EMdgmSHIdataFromSensor_def
{
	uint16_t sensorType;  /*!< */
	float heigthUsed_m;  /*!< Height corrected using installation parameters, if any. Unit meter. */
	uint8_t dataFromSensor[MAX_SHI_DATALENGTH];  /*!< */
};

typedef struct EMdgmSHIdataFromSensor_def EMdgmSHIdataFromSensor, *pEMdgmSHIdataFromSensor;

/*!
 *  \brief #SHI - Height datagram.
 *  */
struct EMdgmSHI_def
{
	struct EMdgmHeader_def	header;
	struct EMdgmScommon_def cmnPart;
	struct EMdgmSHIdataFromSensor_def sensData;
};

#define SHI_VERSION 0
typedef struct EMdgmSHI_def EMdgmSHI, *pEMdgmSHI;


/********************************************* 

   Multibeam datagrams    

 *********************************************/
/*!
 *  \brief  Multibeam (M) datagrams - data partition information. General for all M datagrams.
 *  
 *  If a multibeam depth datagram (or any other large datagram) exceeds the limit of an UPD package (64 kB), the datagram is split into several datagrams =< 64 kB before sending from the PU.
 *  The parameters in this struct will give information of the partitioning of datagrams. K-controller/SIS re-joins all UDP packets/datagram parts to one datagram, and store it as one datagram 
 *  in the .kmall files. Datagrams stored in .kmall files will therefore always have numOfDgm = 1 and dgmNum = 1, and may have size > 64 kB. The maximum number of partitions from PU is given
 *  by MAX_NUM_MWC_DGMS and MAX_NUM_MRZ_DGMS. */
struct EMdgmMpartition_def
{
	uint16_t numOfDgms;   /*!< Number of datagram parts to re-join to get one Multibeam datagram. E.g. 3. */
	uint16_t dgmNum;      /*!< Datagram part number, e.g. 2  (of 3).*/
};

typedef struct EMdgmMpartition_def EMdgmMpartition, *pEMdgmMpartition;

/*!
 *  \brief   Multibeam (M) datagrams - body part. Start of body of all M datagrams.
 *  
 * Contains information of transmitter and receiver used to find data in datagram. The table below illustrates how the indexes will be filled out in
 * different system configurations. Each vertical column is data from one datagram. See index description table and figure below for more information.
 * chapter for #MRZ datagram, for more details.
 * 
 * \image html RXfan_illu.jpg
 * 
 * <br>
 * <b>Counters and indexes in multibeam datagrams:</b><br>
 * Case: Single swath         | Single RX,\n 4 pings shown,\n 4 datagrams	| Dual RX,\n 2 pings shown,\n 4 datagrams |	Dual TX, Dual RX,\n in single swath mode,\n  4 pings shown,\n 8 datagrams
 * ---------------------------|-------------------------|-----------------------|------------------------------------------------------------
 * Ping counter               |	1	2	3	4 | 1   1	2	2 |	1	1	2	2 	3	3	4	4
 * RX fans per ping           |	1	1	1	1 |	2	2	2	2 |	2	2	2	2 	2	2	2	2
 * RX fan index               |	0	0	0	0 |	0	1	0	1 |	0	1	0	1 	0	1	0	1
 * # of swaths per ping       |	1	1	1	1 |	1	1	1	1 |	1	1	1	1 	1	1	1   1
 * Swath along position       |	0	0	0	0 |	0	0	0	0 |	0	0	0	0 	0	0	0	0
 * RX Transducer index        |	0 	0	0	0 | 0	1	0	1 |	0	1	0	1 	0	1	0	1
 * TX Transducer index	     |  0   0	0	0 | 0	0	0	0 |	0	1	0	1 	0	1	0	1
 * # of active RX Transducers |	1	1	1	1 |	2	2	2	2 |	2	2	2	2 	2	2	2	2
 * 
 * <br>
 * <br>
 * 
 * Case: Multi swath          | 	Single RX,\n Dual swath,\n 2 pings shown,\n 4 datagrams |	Single RX,\n 4 swaths along,\n 1 ping shown,\n 4 datagrams	| Dual TX, Dual RX,\n  in Dual swath mode,\n 2 pings shown,\n 8 datagrams
 * ---------------------------|--------------------------------------|-----------------------------------------|------------------------------------------------------------
 * Ping counter               |	1	1	2	2 |	1	1	1	1 |	1	1	1	1	2	2	2	2
 * RX fans per ping           |	2	2	2	2 |	4	4	4	4 |	4	4	4	4	4	4	4	4
 * RX fan index               |	0	1	0	1 |	0	1	2	3 |	0	1	2	3	0	1	2	3
 * # of swaths per ping       |	2	2	2	2 |	4	4	4	4 |	2	2	2	2	2	2	2	2
 * Swath along position       |	0	1	0	1 |	0	1	2	3 |	0	0	1	1	0	0	1	1
 * RX Transducer index        |	0	0	0	0 |	0	0	0	0 |	0	1	0	1	0	1	0	1
 * TX Transducer index        |	0	0	0	0 |	0	0	0	0 |	0	1	0	1	0	1	0	1
 * # of active RX Transducers |	1	1	1	1 |	1	1	1	1 |	2	2	2	2	2	2	2	2
 *  
 */
struct EMdgmMbody_def
{
	uint16_t numBytesCmnPart;    /*!<Used for denoting size of current struct, EMdgmMbody_def.*/
	uint16_t pingCnt;  /*!< A ping is made of one or more RX fans and one or more TX pulses transmitted at approximately the same time.
	                      Ping counter is incremented at every set of TX pulses (one or more pulses transmitted at approximately the same time).*/
	uint8_t rxFansPerPing;    /*!< Number of rx fans per ping gives information of how many #MRZ datagrams are generated per ping. 
	                                 Combined with swathsPerPing, number of datagrams to join for a complete swath can be found. */
	uint8_t rxFanIndex;       /*!< Index 0 is the aft swath, port side. */
	uint8_t swathsPerPing;     /*!< Number of swaths per ping. A swath is a complete set of across track data. A swath may contain several transmit sectors and RX fans.*/
	uint8_t swathAlongPosition;  /*!< Alongship index for the location of the swath in multi swath mode. Index 0 is the aftmost swath.*/
	uint8_t txTransducerInd;   /*!< Transducer used in this rx fan. Index:  <br> 0 = TRAI_TX1 <br> 1 = TRAI_TX2 etc. */
	uint8_t rxTransducerInd;   /*!< Transducer used in this rx fan. Index:  <br> 0 = TRAI_RX1 <br> 1 = TRAI_RX2 etc. */
	uint8_t numRxTransducers;  /*!< Total number of receiving units. */
	uint8_t algorithmType;  /*!< For future use. 0 - current algorithm, >0 - future algorithms. */

};

typedef struct EMdgmMbody_def EMdgmMbody, *pEMdgmMbody;

/************************************
    #MRZ - multibeam data for raw range, 
    depth, reflectivity, seabed image(SI) etc.
 ************************************/
/*!
 *  \brief #MRZ - ping info. 
 * Information on vessel/system level, i.e. information common to all beams in the current ping.<br>
 */
struct EMdgmMRZ_pingInfo_def
{
	uint16_t numBytesInfoData;   /*!< Number of bytes in current struct.*/
	uint16_t padding0;   /*!< Byte alignment.*/

	/*! \name Ping info*/
	/**@{*/
	float pingRate_Hz;  /*!< Ping rate. Filtered/averaged. */
	uint8_t beamSpacing;  /*!<  0 = Eqidistance <br> 1 = Equiangle <br> 2 = High density <br> */
	uint8_t depthMode;   /*!< Depth mode. Describes setting of depth in K-Controller. Depth mode influences the PUs choice of pulse length and pulse type.
	                          If operator has manually chosen the depth mode to use, this is flagged by adding 100 to the mode index.<br>
	                         <br>
	                         <br>

	                            Number | Auto setting |  Number   | Manual setting  
	                            -------|--------------|-----------|------------------
	                            0 | Very shallow | 100 | Very shallow 
	                            1 | Shallow      | 101 | Shallow
	                            2 | Medium       | 102 | Medium
	                            3 | Deep         | 103 | Deep
	                            4 | Deeper       | 104 | Deeper
	                            5 | Very deep    | 105 | Very deep
								6 | Extra deep   | 106 | Extra deep
								7 | Extreme deep | 107 | Extreme deep 
	                            */

	uint8_t subDepthMode;  /*!< For advanced use when depth mode is set manually.  0 = Sub depth mode is not used (when depth mode is auto). */
	uint8_t distanceBtwSwath; /*!< Achieved distance between swaths, in percent relative to required swath distance. <br>
	                               0 = function is not used   <br>
	                               100 = achieved swath distance equals required swath distance. <br>*/

	uint8_t detectionMode;   /*!< Detection mode.  Bottom detection algorithm used. <br> 0 = normal <br> 1 = waterway <br> 2 = tracking <br> 3 = minimum depth 
								If system running in simulation mode <br> detectionmode + 100 = simulator. */
	uint8_t pulseForm; /*!< Pulse forms used for current swath.  <br> 0 = CW <br> 1 = mix <br> 2 = FM */
	uint16_t padding1;

	float frequencyMode_Hz; /*!< Ping frequency in hertz. 
	                            E.g. for EM 2040: 200 000 Hz, 300 000 Hz or 400 000 Hz. If values is less than 100, 
	                            it refers to a code defined in the table below.<br>

	                            Value | Frequency | Valid for EM model
	                          --------|-----------|--------------------
	                            -1    | Not used  | -
	                            0     | 40 - 100 kHz | EM 710, EM 712
	                            1     | 50 - 100 kHz | EM 710, EM 712
	                            2     | 70 - 100 kHz | EM 710, EM 712
	                            3     | 50 kHz | EM 710, EM 712
	                            4     | 40 kHz | EM 710, EM 712
	                            180 000 - 400 000 | 180 -400 kHz | EM 2040C (10 kHz steps)
	                            200 000  |  200 kHz | EM 2040
	                            300 000  | 300 kHz | EM 2040
	                            400 000  | 400 kHz  | EM 2040

	 */
	float freqRangeLowLim_Hz;   /*!< Lowest centre frequency of all sectors in this swath. Unit hertz. E.g. for EM 2040: 260 000 Hz. */
	float freqRangeHighLim_Hz; /*!<  Highest centre frequency of all sectors in this swath. Unit hertz. E.g. for EM 2040: 320 000 Hz. */
	float maxTotalTxPulseLength_sec; /*!< Total signal length of the sector with longest tx pulse. Unit second. */
	float maxEffTxPulseLength_sec; /*!< Effective signal length (-3dB envelope) of the sector with longest effective tx pulse. Unit second.*/
	float maxEffTxBandWidth_Hz; /*!< Effective bandwidth (-3dB envelope) of the sector with highest bandwidth. */
	float absCoeff_dBPerkm;  /*!< Average absorption coefficient, in dB/km, for vertical beam at current depth. Not currently in use. */

	float portSectorEdge_deg;  /*!< Port sector edge, used by beamformer, Coverage is refered to z of SCS.. Unit degree. */
	float starbSectorEdge_deg; /*!< Starboard sector edge, used by beamformer. Coverage is referred to z of SCS. Unit degree. */
	float portMeanCov_deg;  /*!< Coverage achieved, corrected for raybending. Coverage is referred to z of SCS. Unit degree. */
	float starbMeanCov_deg; /*!< Coverage achieved, corrected for raybending. Coverage is referred to z of SCS. Unit degree. */
	int16_t portMeanCov_m;  /*!< Coverage achieved, corrected for raybending. Coverage is referred to z of SCS. Unit meter.*/
	int16_t starbMeanCov_m; /*!< Coverage achieved, corrected for raybending. Unit meter.*/

	uint8_t modeAndStabilisation; /*!< Modes and stabilisation settings as chosen by operator. Each bit refers to one setting in K-Controller.
	                                   Unless otherwise stated, default: 0 = off, 1 = on/auto.<br>
	                                   Bit | Mode 
	                                   -----|----------
                                     1     | Pitch stabilisation
                                     2     | Yaw stabilisation
                                     3     | Sonar mode
                                     4     | Angular coverage mode
                                     5     | Sector mode
                                     6     | Swath along position (0 = fixed, 1 = dynamic)
                                     7-8   | Future use
	 */
	uint8_t runtimeFilter1;  /*!< Filter settings as chosen by operator. Refers to settings in runtime display of K-Controller. 
	                            Each bit refers to one filter setting. 0 = off, 1 = on/auto. <br>
	                         Bit | Filter choice 
                           ------|----------
                           1     | Slope filter
                           2     | Aeration filer
                           3     | Sector filter
                           4     | Interference filter
                           5     | Special amplitude detect
                           6-8   | Future use
	 */
	uint16_t runtimeFilter2; /*!< Filter settings as chosen by operator. Refers to settings in runtime display of K-Controller. 4 bits used per filter. <br>   
	                         
	                         Bit | Filter choice | Setting
                           ------|---------------|----------     
                           1-4     | Range gate size | 0 = small, 1 = normal, 2 = large
                           5-8     | Spike filter strength | 0 = off, 1= weak, 2 = medium, 3 = strong
                           9-12    | Penetration filter | 0 = off, 1 = weak, 2 = medium, 3 = strong
                           13-16   | Phase ramp | 0 = short, 1 = normal, 2 = long
	 */

	uint32_t pipeTrackingStatus;  /*!< Pipe tracking status. Describes how angle and range of top of pipe is determined. <br> 0 = for future use <br> 1 = PU uses guidance from SIS. <br> */
	float transmitArraySizeUsed_deg; /*!< Transmit array size used. Direction along ship. Unit degree. */ 
	float receiveArraySizeUsed_deg;  /*!< Receiver array size used. Direction across ship. Unit degree. */ 
	float transmitPower_dB; /*!< Operator selected tx power level re maximum. Unit dB. E.g. 0 dB, -10 dB, -20 dB. */
	uint16_t SLrampUpTimeRemaining; /*!< For marine mammal protection. The parameters describes time remaining until max source level (SL) is achieved. Unit %. */ 
	uint16_t padding2;  /*!< Byte alignment.*/
	float yawAngle_deg; /*!< Yaw correction angle applied. Unit degree.*/

	/**@}*/

	/*! \name Info of tx sector data block, EMdgmMRZ_txSectorInfo*/
	/**@{*/
	uint16_t numTxSectors;  /*!<Number of transmit sectors. Also called Ntx in documentation. Denotes how many times the struct EMdgmMRZ_txSectorInfo is repeated in the datagram.*/
	uint16_t numBytesPerTxSector;  /*!< Number of bytes in the struct EMdgmMRZ_txSectorInfo, containing tx sector specific information. The struct is repeated numTxSectors times.*/
	/**@}*/

	/*! \name Info at time of midpoint of first tx pulse*/
	/**@{*/
	float headingVessel_deg;  /*!< Heading of vessel at time of midpoint of first tx pulse. From active heading sensor.*/
	float soundSpeedAtTxDepth_mPerSec; /*!<  At time of midpoint of first tx pulse. Value as used in depth calculations. Source of sound speed defined by user in K-Controller.*/
	float txTransducerDepth_m; /*!< Tx transducer depth in meters below waterline, at time of midpoint of first tx pulse. For the tx array (head) used by this RX-fan. Use depth of TX1 to move depth point (XYZ) from water line to transducer (reference point of old datagram format).*/
	float z_waterLevelReRefPoint_m; /*!< Distance between water line and vessel reference point in meters. At time of midpoint of first tx pulse. Measured in the surface coordinate system (SCS).See  \ref advanced 'Coordinate systems' for definition. Used this to move depth point (XYZ) from vessel reference point to waterline.*/
	float x_kmallToall_m;   /*!< Distance between *.all reference point and *.kmall reference point (vessel refernece point) in meters, in the surface coordinate system, at time of midpoint of first tx pulse. Used this to move depth point (XYZ) from vessel reference point to the horisontal location (X,Y) of the active position sensor's reference point (old datagram format).*/
	float y_kmallToall_m;   /*!< Distance between *.all reference point and *.kmall reference point (vessel refernece point) in meters, in the surface coordinate system, at time of midpoint of first tx pulse. Used this to move depth point (XYZ) from vessel reference point to the horisontal location (X,Y) of the active position sensor's reference point (old datagram format).*/

	uint8_t latLongInfo; /*!< Method of position determination from position sensor data: <br> 0 = last position received <br>   1 = interpolated  <br> 2 = processed */
	uint8_t posSensorStatus; /*!< Status/quality for data from active position sensor. 0 = valid data, 1 = invalid data, 2 = reduced performance. To be specified in later release.*/
	uint8_t attitudeSensorStatus; /*!< Status/quality for data from active attitude sensor. To be specified in later release. */
	uint8_t padding3;  /*!< Padding for byte alignment.*/
	double latitude_deg; /*!< Latitude (decimal degrees) of vessel reference point at time of midpoint of first tx pulse. Negative on southern hemisphere. Parameter is set to define #UNAVAILABLE_LATITUDE if not available.*/
	double longitude_deg; /*!< Longitude (decimal degrees) of vessel reference point at time of midpoint of first tx pulse. Negative on western hemisphere. Parameter is set to define #UNAVAILABLE_LONGITUDE if not available.*/
	float ellipsoidHeightReRefPoint_m; /*!< Height of vessel reference point above the ellipsoid, derived from active GGA sensor. ellipsoidHeightReRefPoint_m is GGA height corrected for motion and installation offsets of the position sensor. */
	/**@}*/
	
	};

typedef struct EMdgmMRZ_pingInfo_def EMdgmMRZ_pingInfo, *pEMdgmMRZ_pingInfo;

/*!
 *  \brief   #MRZ - sector information. 
 *  
 * Information specific to each transmitting sector. sectorInfo is repeated numTxSectors (Ntx)- times in datagram. 
 */
struct EMdgmMRZ_txSectorInfo_def
{
	uint8_t txSectorNumb;  /*!< TX sector index number, used in the sounding section. Starts at 0. */
	uint8_t txArrNumber;  /*!< TX array number. Single TX, txArrNumber = 0. */
	uint8_t txSubArray;  /*!< Default = 0. E.g. for EM2040, the transmitted pulse consists of three sectors, each transmitted from separate txSubArrays. 
	                          Orientation and numbers are relative the array coordinate system. Sub array installation offsets can be found in the installation datagram, #IIP. <br> 0 = Port subarray <br> 1 = middle subarray <br> 2 = starboard subarray <br> */ 
	uint8_t padding0;    /*!< Byte alignment.*/
	float sectorTransmitDelay_sec;  /*!< Transmit delay of the current sector/subarray. Delay is the time from the midpoint of the current transmission to midpoint of the first transmitted pulse of the ping, i.e. relative to the time used in the datagram header.*/
	float tiltAngleReTx_deg; /*!< Along ship steering angle of the TX beam (main lobe of transmitted pulse), angle referred to transducer array coordinate system. 
	                              Unit degree. See  \ref advanced 'Coordinate systems'*/
	float txNominalSourceLevel_dB;  /*!< Unit dB re 1 microPascal.*/
	float txFocusRange_m; /*!< 0 = no focusing applied.*/
	float centreFreq_Hz;  /*!< Centre frequency. Unit hertz. */
	float signalBandWidth_Hz;     /*!< FM mode: effective bandwidth <br> CW mode: 1/(effective TX pulse length) */
	float totalSignalLength_sec;    /*!< Also called pulse length. Unit second. */
	uint8_t pulseShading;   /*!< Transmit pulse is shaded in time (tapering). Amplitude shading in %. cos2- function used for shading the TX pulse in time. */
	uint8_t signalWaveForm; /*!< Transmit signal wave form.  <br> 0 = CW <br> 1 = FM upsweep <br> 2 = FM downsweep. */
	uint16_t padding1;      /*!< Byte alignment.*/
	
};

typedef struct EMdgmMRZ_txSectorInfo_def EMdgmMRZ_txSectorInfo, *pEMdgmMRZ_txSectorInfo;

/*!
 *  \brief   #MRZ - receiver specific information. 
 *  
 *  Information specific to the receiver unit used in this swath.*/
struct EMdgmMRZ_rxInfo_def
{
	uint16_t numBytesRxInfo;    /*!< Bytes in current struct. */
	uint16_t numSoundingsMaxMain; /*!< Maximum number of main soundings (bottom soundings) in this datagram, extra detections (soundings in water column) excluded. Also referred to as Nrx. Denotes how many bottom points (or loops) given in the struct EMdgmMRZ_sounding_def. */
	uint16_t numSoundingsValidMain;  /*!< Number of main soundings of valid quality. Extra detections not included. */
	uint16_t numBytesPerSounding; /*!< Bytes per loop of sounding (per depth point), i.e. bytes per loops of the struct EMdgmMRZ_sounding_def. */
	float WCSampleRate;   /*!< Sample frequency divided by water column decimation factor. Unit hertz.*/
	float seabedImageSampleRate; /*!< Sample frequency divided by seabed image decimation factor. Unit hertz.*/
	float BSnormal_dB;  /*!< Backscatter level, normal incidence. Unit dB*/
	float BSoblique_dB; /*!< Backscatter level, oblique incidence. Unit dB*/
	uint16_t extraDetectionAlarmFlag;  /*!< extraDetectionAlarmFlag = sum of alarm flags. Range 0-10.*/
	uint16_t numExtraDetections; /*!< Sum of extradetection from all classes. Also refered to as Nd.*/
	uint16_t numExtraDetectionClasses; /*!< Range 0-10.*/
	uint16_t numBytesPerClass;  /*!< Number of bytes in the struct EMdgmMRZ_extraDetClassInfo_def.*/

};

typedef struct EMdgmMRZ_rxInfo_def EMdgmMRZ_rxInfo, *pEMdgmMRZ_rxInfo;

/*!
 *  \brief #MRZ - Extra detection class information. 
 *  
 *  To be entered in loop numExtraDetectionClasses - times.*/
struct EMdgmMRZ_extraDetClassInfo_def
{
	uint16_t numExtraDetInClass;  /*!<Number of extra detection in this class.*/
	int8_t padding;  /*!< Byte alignment.*/
	uint8_t alarmFlag;  /*!<  0 = no alarm <br>
	                          1 = alarm.*/
};

typedef struct EMdgmMRZ_extraDetClassInfo_def EMdgmMRZ_extraDetClassInfo, *pEMdgmMRZ_extraDetClassInfo;

/*!
 *  \brief #MRZ - Data for each sounding, e.g. XYZ, reflectivity, two way travel time etc. 
 *  
 * Also contains information necessary to read seabed image following this datablock (number of samples in SI etc.).
 * To be entered in loop (numSoundingsMaxMain + numExtraDetections) times. <br>
 * 
 */
struct EMdgmMRZ_sounding_def
{

	uint16_t soundingIndex; /*!< Sounding index. Cross reference for seabed image. Valid range: 0 to (numSoundingsMaxMain+numExtraDetections)-1, i.e. 0 - (Nrx+Nd)-1. */
	uint8_t txSectorNumb;  /*!< Transmitting sector number. Valid range: 0-(Ntx-1), where Ntx is numTxSectors. */

	/*! \name Detection info. */
	/**@{*/
	uint8_t detectionType;   /*!< Bottom detection type. Normal bottom detection, extra detection, or rejected.  <br> 0 = normal detection <br> 1 = extra detection  <br> 2 = rejected detection  
	                           In case 2, the estimated range has been used to fill in amplitude samples in the seabed image datagram.*/
	uint8_t detectionMethod; /*!< Method for determining bottom detection, e.g. amplitude or phase.  <br> 0 = no valid detection <br> 1 = amplitude detection <br> 2 = phase detection <br> 3-15 for future use. */
	uint8_t rejectionInfo1;  /*!< For Kongsberg use.*/ 
	uint8_t rejectionInfo2;  /*!< For Kongsberg use.*/
	uint8_t postProcessingInfo;  /*!< For Kongsberg use.*/
	uint8_t detectionClass; /*!< Detection class based on detected range.*/
	uint8_t detectionConfidenceLevel;  /*!< Detection confidence level.*/
	uint16_t padding; /*!< Byte alignment.*/
	float rangeFactor; /*!< Unit %. rangeFactor = 100 if main detection.  */
	float qualityFactor;  /*!< Estimated standard deviation as % of the detected depth. Quality Factor (QF) is calculated from IFREMER Quality Factor (IFQ):  <br> QF=Est(dz)/z=100*10^-IQF */
	float detectionUncertaintyVer_m;  /*!< Vertical uncertainty, based on quality factor (QF, qualityFactor).*/
	float detectionUncertaintyHor_m;  /*!< Horizontal uncertainty, based on quality factor (QF, qualityFactor).*/
	float detectionWindowLength_sec;  /*!< Detection window length. Unit second. Sample data range used in final detection.*/
	float echoLength_sec; /*!< Measured echo length. Unit second. */
	/**@}*/

	/*! \name Water column paramters. */
	/**@{*/
	uint16_t WCBeamNumb;       /*!< Water column beam number. Info for plotting soundings together with water column data.*/
	uint16_t WCrange_samples;  /*!< Water column range. Range of bottom detection, in samples. */
	float WCNomBeamAngleAcross_deg; /*!< Water column nominal beam angle across. Re vertical.*/
	/**@}*/

	/*! \name Reflectivity data (backscatter (BS) data).*/
	/**@{*/
	float meanAbsCoeff_dBPerkm; /*!< Mean absorption coefficient, alfa. Used for TVG calculations. Value as used. Unit dB/km.*/
	float reflectivity1_dB;  /*!< Beam intensity, using the traditional KM special TVG.*/
	float reflectivity2_dB;  /*!< Beam intensity (BS), using TVG = X log(R) + 2 alpha R. X (operator selected) is common to all beams in datagram. 
                                  Alpha (variabel meanAbsCoeff_dBPerkm) is given for each beam (current struct). <br>
                                  BS = EL - SL - M + TVG + BScorr, <br> 
                                  where EL= detected echo level (not recorded in datagram), and the rest of the parameters are found below.
	                          */
	float receiverSensitivityApplied_dB; /*!< Receiver sensitivity (M), in dB, compensated for RX beampattern at actual transmit frequency at current vessel attitude. */
	float sourceLevelApplied_dB; /*!< Source level (SL) applied (dB): <br> 
	                                 SL =  SLnom + SLcorr <br> 
	                                 where SLnom = Nominal maximum SL, recorded per TX sector (variabel txNominalSourceLevel_dB in struct EMdgmMRZ_txSectorInfo_def) and <br> 
	                                 SLcorr = SL correction relative to nominal TX power based on measured high voltage power level and any use of digital power control. <br>
	                                 SL is corrected for TX beampattern along and across at actual transmit frequency at current vessel attitude.*/
	float BScalibration_dB; /*!< Backscatter (BScorr) calibration offset applied (default = 0 dB). */
	float TVG_dB; /*!< Time Varying Gain (TVG) used when correcting reflectivity.*/ 
	/**@}*/

	/*! \name Range and angle data. */
	/**@{*/
	float beamAngleReRx_deg;  /*!< Angle relative to the RX transducer array, except for ME70, where the angles are relative to the horizontal plane.*/
	float beamAngleCorrection_deg;  /*!< Applied beam pointing angle correction.*/
	float twoWayTravelTime_sec;  /*!< Two way travel time (also called range). Unit second. */
	float twoWayTravelTimeCorrection_sec; /*!< Applied two way travel time correction. Unit second.*/
	/**@}*/

	/*! \name Georeferenced depth points. */
	/**@{*/
	float deltaLatitude_deg;   /*!< Distance from vessel reference point at time of first tx pulse in ping, to depth point. Measured in the surface coordinate system (SCS), see \ref advanced "Coordinate systems" for definition. <br>                                   
	                                 Unit decimal degrees.*/
	float deltaLongitude_deg;  /*!< Distance from vessel reference point at time of first tx pulse in ping, to depth point. Measured in the surface coordinate system (SCS), see \ref advanced "Coordinate systems" for definition. <br>                                   
	                                 Unit decimal degree. */
	float z_reRefPoint_m; /*!< Vertical distance z. Distance from vessel reference point at time of first tx pulse in ping, to depth point. 
	                      Measured in the surface coordinate system (SCS), see  \ref advanced "Coordinate systems" for definition.  */
	float y_reRefPoint_m; /*!< Horizontal distance y. Distance from vessel reference point at time of first tx pulse in ping, to depth point. 
	                     Measured in the surface coordinate system (SCS), see  \ref advanced "Coordinate systems" for definition. */
	float x_reRefPoint_m; /*!<  Horizontal distance x. Distance from vessel reference point at time of first tx pulse in ping, to depth point. 
	                     Measured in the surface coordinate system (SCS), see  \ref advanced "Coordinate systems" for definition. */

	float beamIncAngleAdj_deg; /*!< Beam incidence angle adjustment (IBA) unit degree. */
	uint16_t realTimeCleanInfo;	/*!< For future use. */
	/**@}*/

	/*! \name Seabed image. */
	/**@{*/
	uint16_t SIstartRange_samples; /*!< Seabed image start range, in sample number from transducer. Valid only for the current beam. */
	uint16_t SIcentreSample;  /*!<  Seabed image. Number of the centre seabed image sample for the current beam.*/
	uint16_t SInumSamples;	  /*!< Seabed image. Number of range samples from the current beam, used to form the seabed image. */
	/**@}*/
};

typedef struct EMdgmMRZ_sounding_def EMdgmMRZ_sounding, *pEMdgmMRZ_sounding;

/*!
 *  \brief #MRZ - Extra seabed image samples.*/
struct EMdgmMRZ_extraSI_def
{

	uint16_t portStartRange_samples;  /*!< Start range of port side extra seabed image samples. Range in samples.*/
	uint16_t numPortSamples;           /*!< Number of extra seabed image samples on port side. */
	int16_t portSIsample_desidB[MAX_SIDESCAN_EXTRA_SAMP]; /*!< Port side extra seabed image samples, as amplitudes in 0.1 dB.*/
	uint16_t starbStartRange_samples; /*!< Start range of starboard side extra seabed image samples. Range in samples.*/
	uint16_t numStarbSamples;   /*!< Number of extra seabed image samples on starboard side. */
	int16_t starbSIsample_desidB[MAX_SIDESCAN_EXTRA_SAMP]; /*!< Starboard side extra seabed image samples, as amplitudes in 0.1 dB.*/

};

typedef struct EMdgmMRZ_extraSI_def EMdgmMRZ_extraSI, *pEMdgmMRZ_extraSI;

/*!
 *  \brief   #MRZ - Multibeam Raw Range and Depth datagram. The datagram also contains seabed image data.
 *  
 *  Datagram consists of several structs. The MRZ datagram replaces several old datagrams: raw range (N 78), 
 *  depth (XYZ 88), seabed image (Y 89) datagram, quality factor (O 79) and runtime (R 52).
 *  
 *  Depths points (x,y,z) are calculated in meters, georeferred to the position of the vessel reference point at the time of the first transmitted pulse of the ping. 
 *  The depth point coordinates x and y are in the surface coordinate system (SCS), and are also given as delta latitude and delta longitude, 
 *  referred to origo of the VCS/SCS, at the time of the midpoint of the first transmitted pulse of the ping (equals time used in the datagram header timestamp).  
 *  See \ref advanced "Coordinate systems" for introduction to spatial reference points and coordinate systems. Reference points are also described in \ref advanced2 "Reference points and offsets".
 *  Explanation of the xyz reference points is also illustrated in the figure below.
 *  \image html "xyz illustrasjon small.jpg"  "Depth point and reference systems"
 *  \image latex "xyz illustrasjon small.eps"  "Depth point and reference systems"
 *  */
struct EMdgmMRZ_def
{
	struct EMdgmHeader_def header;   
	struct EMdgmMpartition_def partition;
	struct EMdgmMbody_def cmnPart;
	struct EMdgmMRZ_pingInfo_def pingInfo;                            
	struct EMdgmMRZ_txSectorInfo_def sectorInfo[MAX_NUM_TX_PULSES];   
	struct EMdgmMRZ_rxInfo_def rxInfo;                                
	struct EMdgmMRZ_extraDetClassInfo_def extraDetClassInfo[MAX_EXTRA_DET_CLASSES];  
	struct EMdgmMRZ_sounding_def sounding[MAX_NUM_BEAMS+MAX_EXTRA_DET];              
	int16_t SIsample_desidB[MAX_SIDESCAN_SAMP];               /*!< Seabed image sample amplitude, in 0.1 dB.  Actual number of seabed image samples (SIsample_desidB) to be 
	                                                           found by summing parameter SInumSamples in struct EMdgmMRZ_sounding_def for all beams. 
	                                                           Seabed image data are raw beam sample data taken from the RX beams. The data samples are 
	                                                           selected based on the bottom detection ranges. First sample for each beam is the one with the lowest range. 
	                                                           The centre sample from each beam is geo referenced (x, y, z data from the detections). 
	                                                           The BS corrections applied at the centre 
	                                                           sample are the same as used for reflectivity2_dB (struct EMdgmMRZ_sounding_def).*/               

}; 

#define MRZ_VERSION 0
typedef struct EMdgmMRZ_def EMdgmMRZ, *pEMdgmMRZ;

/************************************
    #MWC - water column datagram        
 ************************************/
/*! \brief #MWC - data block 1: transmit sectors, general info for all sectors */
struct EMdgmMWCtxInfo_def
{
	uint16_t numBytesTxInfo;  /*!< Number of bytes in current struct. */
	uint16_t numTxSectors;  /*!< Number of transmitting sectors (Ntx). 
	                             Denotes the number of times the struct EMdgmMWCtxSectorData is repeated in the datagram. */
	uint16_t numBytesPerTxSector;   /*!< Number of bytes in EMdgmMWCtxSectorData. */
	int16_t padding;  /*!< Byte alignment.*/
	float heave_m; /*!< Heave at vessel reference point, at time of ping, i.e. at midpoint of first tx pulse in rxfan.*/
};

typedef struct EMdgmMWCtxInfo_def EMdgmMWCtxInfo, *pEMdgmMWCtxInfo;

/*! \brief #MWC - data block 1: transmit sector data, loop for all i = numTxSectors.*/
struct EMdgmMWCtxSectorData_def
{
	float tiltAngleReTx_deg;  /*!< Along ship steering angle of the TX beam (main lobe of transmitted pulse), angle referred to transducer face. Angle as used by beamformer (includes stabilisation). Unit degree.*/
	float centreFreq_Hz;        /*!< Centre frequency of current sector. Unit hertz. */
	float txBeamWidthAlong_deg; /*!< Corrected for frequency, sound velocity and tilt angle. Unit degree. */
	uint16_t txSectorNum;    /*!< Transmitting sector number.*/
	int16_t padding;  /*!< Byte alignment.*/
};

typedef struct EMdgmMWCtxSectorData_def EMdgmMWCtxSectorData, *pEMdgmMWCtxSectorData;

/*! \brief #MWC - data block 2: receiver, general info */
struct EMdgmMWCrxInfo_def
{
	uint16_t numBytesRxInfo;  /*!< Number of bytes in current struct. */
	uint16_t numBeams;  /*!< Number of beams in this datagram (Nrx).*/
	uint8_t numBytesPerBeamEntry;      /*!< Bytes in EMdgmMWCrxBeamData struct, 
	                                          excluding sample amplitudes (which have varying lengths)*/
	uint8_t phaseFlag;  /*!< 0 = off  <br> 1 = low resolution  <br> 2 = high resolution. */
	uint8_t TVGfunctionApplied;  /*!< Time Varying Gain function applied (X).  X log R + 2 Alpha R + OFS + C, 
	                                   where X and C is documented in #MWC datagram. OFS is gain offset to compensate for TX source level, receiver sensitivity etc.*/
	int8_t TVGoffset_dB; /*!< Time Varying Gain offset used (OFS), unit dB. X log R + 2 Alpha R + OFS + C, 
	                               where X and C is documented in #MWC datagram. OFS is gain offset to compensate for TX source level, receiver sensitivity etc.*/
	float sampleFreq_Hz;   /*!< The sample rate is normally decimated to be approximately  the same as the bandwidth of the transmitted pulse. Unit hertz.*/
	float soundVelocity_mPerSec;  /*!< Sound speed at transducer, unit m/s.*/

};

typedef struct EMdgmMWCrxInfo_def EMdgmMWCrxInfo, *pEMdgmMWCrxInfo;

/*! \brief #MWC - data block 2: receiver, specific info for each beam. */
struct EMdgmMWCrxBeamData_def
{
	float beamPointAngReVertical_deg;
	uint16_t startRangeSampleNum;
	uint16_t detectedRangeInSamples;    /*!< Two way range in samples. Approximation to calculated distance from tx to bottom detection [meters] = soundVelocity_mPerSec * detectedRangeInSamples / (sampleFreq_Hz * 2). The detected range
                                          is set to zero when the beam has no bottom detection*/
	uint16_t beamTxSectorNum;
	uint16_t numSampleData;  /*!< Number of sample data for current beam. Also denoted Ns. */
	int8_t  *sampleAmplitude05dB_p;  /*!< Pointer to start of array with Water Column data. Lenght of array = numSampleData. Sample amplitudes in 0.5 dB resolution. Size of array is numSampleData * int8_t. Amplitude array is followed by phase information if phaseFlag >0. Use (numSampleData * int8_t) to jump to next beam, or to start of phase info for this beam, if phase flag > 0.*/
};

typedef struct EMdgmMWCrxBeamData_def EMdgmMWCrxBeamData, *pEMdgmMWCrxBeamData;

/*! \brief #MWC - Beam sample phase info, specific for each beam and water column sample. numBeams * numSampleData = (Nrx * Ns) entries. Only added to datagram if phaseFlag = 1.  Total size of phase block is numSampleData * int8_t.  */
struct EMdgmMWCrxBeamPhase1_def
{
	int8_t rxBeamPhase; /*!< Rx beam phase in 180/128 degree resolution.*/
};

typedef struct EMdgmMWCrxBeamPhase1_def EMdgmMWCrxBeamPhase1, *pEMdgmMWCrxBeamPhase1;

/*! \brief #MWC - Beam sample phase info, specific for each beam and water column sample. numBeams * numSampleData = (Nrx * Ns) entries. Only added to datagram if phaseFlag = 2.  Total size of phase block is numSampleData * int16_t.*/
struct EMdgmMWCrxBeamPhase2_def
{
	int16_t rxBeamPhase; /*!< Rx beam phase in 0.01 degree resolution.*/
};

typedef struct EMdgmMWCrxBeamPhase2_def EMdgmMWCrxBeamPhase2, *pEMdgmMWCrxBeamPhase2;

/*! \brief #MWC - Multibeam Water Column Datagram. Entire datagram containing several sub structs.*/
struct EMdgmMWC_def
{
	struct EMdgmHeader_def header;
	struct EMdgmMpartition_def partition;
	struct EMdgmMbody_def cmnPart;
	struct EMdgmMWCtxInfo_def txInfo; 
	struct EMdgmMWCtxSectorData_def sectorData[MAX_NUM_TX_PULSES]; 
	struct EMdgmMWCrxInfo_def rxInfo;
	struct EMdgmMWCrxBeamData_def *beamData_p; /*!< Pointer to beam related information. Struct defines information about data for a beam. Beam information is followed by sample amplitudes in 0.5 dB resolution .
	                                  Amplitude array is followed by phase information if phaseFlag >0. These data defined by struct EMdgmMWCrxBeamPhase1_def (int8_t) or struct EMdgmMWCrxBeamPhase2_def (int16_t) if indicated in the field phaseFlag in struct 
	                                  EMdgmMWCrxInfo_def.<br>
	                                 Lenght of data block for each beam depends on the operators choise of phase information (see table).

                                     phaseFlag   | Beam block size
                                     --------------|---------------------
                                              0            | numBytesPerBeamEntry +  numSampleData* size(sampleAmplitude05dB_p)
                                              1            | numBytesPerBeamEntry +  numSampleData* size(sampleAmplitude05dB_p) + numSampleData* size(EMdgmMWCrxBeamPhase1_def)
                                              2            |  numBytesPerBeamEntry +  numSampleData* size(sampleAmplitude05dB_p) + numSampleData* size(EMdgmMWCrxBeamPhase2_def)

                                              <br>
                                              */
}; 

#define MWC_VERSION 0
typedef struct EMdgmMWC_def EMdgmMWC, *pEMdgmMWC;




/********************************************* 

   Compatibility datagrams for .all to .kmall conversion support

 *********************************************/

/************************************
   #CPO - Compatibility position sensor data       
 ************************************/
/*!
 *  \brief #CPO - Compatibility sensor position compatibility data block. Data from active sensor is referenced to position at antenna footprint at water level. Data is
 *  corrected for motion ( roll and pitch only) if enabled by K-Controller operator. Data given both decoded and corrected (active sensors), and raw as received from sensor in text string. 
 *  */
struct EMdgmCPOdataBlock_def
{ 
	uint32_t timeFromSensor_sec;            /*!< UTC time from position sensor. Unit seconds. Epoch 1970-01-01. Nanosec part to be added for more exact time.*/
	uint32_t timeFromSensor_nanosec;  	    /*!< UTC time from position sensor. Unit nano seconds remainder.*/
    float posFixQuality_m;  /*!< Only if available as input from sensor. Calculation according to format.*/
    double correctedLat_deg;   /*!< Motion corrected (if enabled in K-Controller) data as used in depth calculations. Referred to antenna footprint at water level. Unit decimal degree. Parameter is set to define #UNAVAILABLE_LATITUDE if sensor inactive.*/ 
    double correctedLong_deg;   /*!< Motion corrected (if enabled in K-Controller) data as used in depth calculations. Referred to antenna footprint at water level. Unit decimal degree. Parameter is set to define #UNAVAILABLE_LONGITUDE if sensor inactive.*/
    float speedOverGround_mPerSec;  /*!< Speed over ground. Unit m/s. Motion corrected (if enabled in K-Controller) data as used in depth calculations. If unavailable or from inactive sensor, value set to define #UNAVAILABLE_SPEED. */
    float courseOverGround_deg;   /*!< Course over ground. Unit degree. Motion corrected (if enabled in K-Controller) data as used in depth calculations. If unavailable or from inactive sensor, value set to define #UNAVAILABLE_COURSE. */
    float ellipsoidHeightReRefPoint_m;  /*!< Height of antenna footprint above the ellipsoid. Unit meter. Motion corrected (if enabled in K-Controller) data as used in depth calculations. If unavailable or from inactive sensor, value set to define #UNAVAILABLE_ELLIPSOIDHEIGHT.  */
    int8_t posDataFromSensor[MAX_CPO_DATALENGTH]; /*!< Position data as received from sensor, i.e. uncorrected for motion etc. */
};

typedef struct EMdgmCPOdataBlock_def EMdgmCPOdataBlock, *pEMdgmCPOdataBlock;

/*!
 *  \brief   #CPO - Struct of compatibility position sensor datagram.   <br>
 *  <br>
 *  Data from active sensor will be motion corrected if indicated by operator. Motion correction is applied to latitude, longitude, speed, course and ellipsoidal height.
 *  If the sensor is inactive, the fields will be marked as unavailable, defined by the parameters define #UNAVAILABLE_LATITUDE etc.
 *  */
struct EMdgmCPO_def
{
    struct EMdgmHeader_def header;
    struct EMdgmScommon_def cmnPart;
    struct EMdgmCPOdataBlock_def sensorData;
};

#define CPO_VERSION 0
typedef struct EMdgmCPO_def EMdgmCPO, *pEMdgmCPO;


/************************************
    #CHE - Compatibility heave data      
 ************************************/
/*!
 *  \brief #CHE - Heave compatibility data part. Heave reference point is at transducer instead of at vessel reference point. 
 *  */
struct EMdgmCHEdata_def
{
    float heave_m;  /*!< Heave. Unit meter. Positive downwards.*/
};

typedef struct EMdgmCHEdata_def EMdgmCHEdata, *pEMdgmCHEdata;

/*!
 *  \brief   #CHE - Struct of compatibility heave sensor datagram.   <br>
 *  <br>
 *  Used for backward compatibility with .all datagram format. Sent before #MWC (water column datagram) datagram if compatibility mode is enabled.
 *  The multibeam datagram body is common with the #MWC datagram.
 *  */
struct EMdgmCHE_def
{
    struct EMdgmHeader_def  header;
    struct EMdgmMbody_def cmnPart;
    struct EMdgmCHEdata_def data;
};

#define CHE_VERSION 0
typedef struct EMdgmCHE_def EMdgmCHE, *pEMdgmCHE;


/********************************************* 

   Installation and runtime datagrams    

 *********************************************/

/************************************
    #IIP - Info Installation PU     
 ************************************/
/*!
 *  \brief Definition of #IIP datagram containing installation parameters and sensor format settings. <br>
 *  Details in separate document <a href="Installationparameter.pdf" target="_blank"><b>Installation parameters</b></a>
 *  */
struct EMdgmIIP_def
{
	EMdgmHeader header;
	uint16_t numBytesCmnPart;  /*!< Size in bytes of body part struct. Used for denoting size of rest of the datagram.*/
	uint16_t info;				/*!< Information. For future use. */ 
	uint16_t status;				/*!< Status. For future use. */  
	uint8_t install_txt;		/*!< Installation settings as text format. Parameters separated by ; and lines separated by , delimiter.
                                         <br><br>
                                         For detailed description of text strings, see the separate document
                                          <a href="Installationparameter.pdf" target="_blank"><b>Installation parameters</b></a>*/
};

#define IIP_VERSION 0
typedef struct EMdgmIIP_def dgm_IIP, *pdgm_IIP;


/************************************
    #IOP -  Runtime datagram  
 ************************************/
/*!
 *  \brief Definition of #IOP datagram containing runtime parameters, exactly as chosen by operator 
 *  in the K-Controller/SIS menus.
 *  <br>
 *  For detailed description of text strings, see the separate document
                                          <a href="operatorsettingparameters_EM_2040.pdf" target="_blank"><b>Runtime parameters set by operator</b></a>
 *  */
struct EMdgmIOP_def
{
	EMdgmHeader header;
	uint16_t numBytesCmnPart;  /*!< Size in bytes of body part struct. Used for denoting size of rest of the datagram.*/
	uint16_t info;				/*!< Information. For future use. */
	uint16_t status;				/*!< Status. For future use.*/
	uint8_t runtime_txt;		/*!< Runtime paramters as text format. Parameters separated by ; and lines separated by , delimiter. 
	                                     Text strings refer to names in menues of the K-Controller/SIS. 
	                                     <br><br>
	                                     For detailed description of text strings, see the separate document
                                          <a href="operatorsettingparameters_EM_2040.pdf" target="_blank"><b>Runtime parameters set by operator</b></a>*/

};

#define IOP_VERSION 0
typedef struct EMdgmIOP_def dgm_IOP, *pdgm_IOP;


/************************************
    #IB - BIST Error Datagrams   
 ************************************/
/*!
 *  \brief #IB - Results from online built in test (BIST). Definition used for three different BIST datagrams, i.e.
 *  #IBE (BIST Error report), #IBR (BIST reply) or #IBS (BIST short reply).
 *  */
struct EMdgmIB_def 
{
	EMdgmHeader header;
	uint16_t numBytesCmnPart;  /*!< Size in bytes of body part struct. Used for denoting size of rest of the datagram.*/
	uint8_t BISTInfo;         /*!< 0 = last subset of the message  <br> 1 = more messages to come*/
	uint8_t BISTStyle;        /*!< 0 = plain text <br> 1 = use style sheet*/
	uint8_t BISTNumber;       /*!< The BIST number executed. */
	int8_t  BISTStatus;       /*!< 0 = BIST executed with no errors <br> positive number = warning <br> negative number = error */
	uint8_t BISTText;         /*!< Result of the BIST. Starts with a synopsis of the result, followed by detailed descriptions.  */                 
};

#define BIST_VERSION 0
typedef struct EMdgmIB_def dgm_IB, *pdgm_IB;

#ifndef _VXW
#pragma pack()
#endif
#endif

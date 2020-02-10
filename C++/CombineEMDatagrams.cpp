// CombineEMDatagrams.cpp : Show how to combine datagram fragments into one large datablock.
//
#define _MDP_STANDALONE
#define _DATUM_DEFINED

#include <iostream>
#include <iomanip> 
#include <winsock2.h>
#include "MessageReceive.h"
#include "EMdgmFormat.h"

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 65535	//Max length of buffer
#define PORT 8888	//The port on which to listen for incoming data

char * getMRZPingInfo(void * tgm);
char * getMRZSectorInfo(void * tgm);
char *getMRZRxInfo(void* tgm);
char *getMRZSoundings(void* tgm);
void handleMRZ(char* mrz);


int main()
{
	WSADATA       wsd;
	SOCKADDR_IN   from;
	SOCKET        s;
	int           ret, iVal = 0;
	int			  sz = sizeof(iVal);
	char          rcvbuf[BUFLEN];
	int           fromsz;

	// Load Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		std::cerr << "WSAStartup() failed with error code " << WSAGetLastError() << "\n";
		return -1;
	}
	else
		std::cout << "WSAStartup() should be OK!\n";

	// Create a datagram socket
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
	{
		std::cerr << "socket() failed with error code " << WSAGetLastError() << "\n";
		return -1;
	}
	else
		std::cout << "socket() is OK!\n";

	// Set the timeout value
	iVal = 100000;
	ret = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&iVal, sz);
	if (ret == SOCKET_ERROR)
	{
		std::cerr << "setsockopt() failed with error code " << WSAGetLastError() << "\n";
		return -1;
	}
	else
		std::cout << "setsockopt() is OK!\n";

	// Retrieve the value just to be sure
	ret = getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&iVal, &sz);
	if (ret == SOCKET_ERROR)
	{
		std::cerr << "getsockopt() failed with error code " << WSAGetLastError() << "\n";
		return -1;
	}
	else
		std::cout << "getsockopt() is fine!\n";
	std::cout << "Timeout value == " << iVal << "\n";

	// To test the receive function we need to bind first
	from.sin_family = AF_INET;
	from.sin_addr.s_addr = htonl(INADDR_ANY);
	from.sin_port = htons(PORT);

	ret = bind(s, (SOCKADDR *)&from, sizeof(from));
	if (ret == SOCKET_ERROR)
	{
		std::cerr << "bind() failed: " << WSAGetLastError() << "\n";
		return -1;
	}

	// Start a thread for reception of results from datagram assembly processing.
	auto consumer = std::thread([&] // Lambda function passing reference to all variables in scope.
	{
		Semaphore            hAssembledMessAvail; // Semaphore to be set when an assembled datagram is ready
		std::recursive_mutex hAssembledBuffMutex; // Mutex used to guard return buffer.
		std::string          strAssembledDgm;     // Buffer for return of assembled datagrams (guarded by previous mutex).

		// Initialize the asynchronous mechanism used to return assembled datagrams from the assembly (i.e. defragment) processing.
		CMessageReceive::instance()->setDgmReturn(&hAssembledMessAvail, &hAssembledBuffMutex, &strAssembledDgm); // Actually making the Message Receive instance here via 'instance()' call.
		do
		{
			hAssembledMessAvail.wait(); // Block here until an assembled datagram is received.
			hAssembledBuffMutex.lock(); // Lock access to the receive buffer.

			// Reaching this position means that an assembled datagram is ready.
			if (strAssembledDgm.size() > 0)
			{
				// Datagram is completed, i.e. defragmentation successful.
				char* data = (char*)strAssembledDgm.data();

				// Check and handle datagrams
				if (data[4] == '#' && data[5] == 'M' && data[6] == 'W' && data[7] == 'C')
				{
					std::cout << "MWC\n";
				}
				else if (data[4] == '#' && data[5] == 'M' && data[6] == 'R' && data[7] == 'Z')
				{
					std::cout << "MRZ\n";
					handleMRZ(data);
				}
				//else if(data[4] == '#' && data[5] == 'M' && data[6] == 'S' && data[7] == 'T')
				//{
				//   // Handle #MST
				//}

			}
			hAssembledBuffMutex.unlock();  // Release buffer
		} while (true);
	});
	consumer.detach(); // Let it run on its own.

	for (;;) {
		memset(rcvbuf, 0, sizeof(rcvbuf));
		fromsz = sizeof(from);
		ret = recvfrom(s, rcvbuf, BUFLEN, 0, (SOCKADDR *)&from, &fromsz);
		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAETIMEDOUT)
				std::cerr << "recvfrom() failed with error code 10060 (WSAETIMEDOUT)\n";
			else
				std::cerr << "recvfrom() failed with error code " << WSAGetLastError() << "\n";
		}
		else {
			//			std::cout << rcvbuf;  // This can be used to debug the socket
			char *buff = new char[ret];// deleted (free) in PutInQ
			memcpy(buff, rcvbuf, ret);
			CMessageReceive::instance()->PutInQ((char*)buff, ret); // Note, buff is deleted by receiver
		}
	}

	if (closesocket(s) == 0)
		std::cout << "closesocket() should be fine!\n";
	else
		std::cerr << "closesocket() failed with error code " << WSAGetLastError() << "\n";

	if (WSACleanup() == 0)
		std::cout << "WSACleanup() is OK!\n";
	else
		std::cerr << "WSACleanup() failed with error code " << WSAGetLastError() << "\n";

	return 0;
}

//f+/////////////////////////////////////////////////////////////////////////
//
//  getMRZPingInfo
/*!
//  Get start ptr of MRZ ping info.
//
//  \param  void
//
//  \return Pointer to the start of rx info.
//
//  \author Ole-Jacob Enderud Jensen
//
//  \date   12.10.2018
*/
//f-/////////////////////////////////////////////////////////////////////////

char * getMRZPingInfo(void * tgm)
{
	char *pData = NULL;

	// if this record is not empty
	if (tgm != NULL)
	{
		pEMdgmMRZ dgm = (pEMdgmMRZ)tgm;
		int sizeCommon = dgm->cmnPart.numBytesCmnPart;

		// Find rxInfo. Must take account of number of TX sectors.
		int numTxSectors = dgm->pingInfo.numTxSectors;

		//Get PingInfo
		char* pPI = (char*)(&(dgm->partition.dgmNum));
		pPI += 2;
		pPI += sizeCommon;

		pEMdgmMRZ_pingInfo pPingInfo = (pEMdgmMRZ_pingInfo)pPI;

		pData = pPI;

		// end of datagram for safety testing
		char *pSOD = (char*)dgm;
		char *pEOD = pSOD + dgm->header.numBytesDgm; // first byte past end of data

													 // safety check - verify pointer is valid
		if (pData >= pEOD)
		{
			pData = NULL;
		}
	}

	return pData;
}

//f+/////////////////////////////////////////////////////////////////////////
//
//  getMRZSectorInfo
/*!
//  Get start ptr of MRZ rx info.
//
//  \param  void
//
//  \return Pointer to the start of rx info.
//
//  \author Ole-Jacob Enderud Jensen
//
//  \date   12.10.2018
*/
//f-/////////////////////////////////////////////////////////////////////////

char * getMRZSectorInfo(void * tgm)
{
	char *pData = NULL;

	// if this record is not empty
	if (tgm != NULL)
	{
		pEMdgmMRZ dgm = (pEMdgmMRZ)tgm;
		int sizeCommon = dgm->cmnPart.numBytesCmnPart;

		// Find rxInfo. Must take account of number of TX sectors.
		int numTxSectors = dgm->pingInfo.numTxSectors;

		//Get PingInfo
		char* pPI = (char*)(&(dgm->partition.dgmNum));
		pPI += 2;
		pPI += sizeCommon;

		pEMdgmMRZ_pingInfo pPingInfo = (pEMdgmMRZ_pingInfo)pPI;

		//Move to end of pingInfo
		char* pch = pPI + pPingInfo->numBytesInfoData; //At start of txsectors
		int nSkip = 0;
		pch += nSkip;

		//Get sector
		int nBytesTx = pPingInfo->numBytesPerTxSector;
		pEMdgmMRZ_txSectorInfo pSect = (pEMdgmMRZ_txSectorInfo)pch;

		pData = pch;

		// end of datagram for safety testing
		char *pSOD = (char*)dgm;
		char *pEOD = pSOD + dgm->header.numBytesDgm; // first byte past end of data

													 // safety check - verify pointer is valid
		if (pData >= pEOD)
		{
			pData = NULL;
		}
	}

	return pData;
}

//f+/////////////////////////////////////////////////////////////////////////
//
//  getMRZRxInfo
/*!
//  Get start ptr of MRZ rx info.
//
//  \param  void
//
//  \return Pointer to the start of rx info.
//
//  \author Ole-Jacob Enderud Jensen
//
//  \date   12.10.2018
*/
//f-/////////////////////////////////////////////////////////////////////////

char *getMRZRxInfo(void* tgm)
{
	char *pData = NULL;

	// if this record is not empty
	if (tgm != NULL)
	{
		pEMdgmMRZ dgm = (pEMdgmMRZ)tgm;
		int sizeCommon = dgm->cmnPart.numBytesCmnPart;

		// Find rxInfo. Must take account of number of TX sectors.
		int numTxSectors = dgm->pingInfo.numTxSectors;

		//Get PingInfo
		char* pPI = (char*)(&(dgm->partition.dgmNum));
		pPI += 2;
		pPI += sizeCommon;

		pEMdgmMRZ_pingInfo pPingInfo = (pEMdgmMRZ_pingInfo)pPI;

		//Move to end of pingInfo
		char* pch = pPI + pPingInfo->numBytesInfoData; //At start of txsectors
		int nSkip = 0;
		pch += nSkip;

		//Get sector
		int nBytesTx = pPingInfo->numBytesPerTxSector;
		pEMdgmMRZ_txSectorInfo pSect = (pEMdgmMRZ_txSectorInfo)pch;

		//Move to start of rxInfo
		pch = (char*)pSect;
		pch += (nBytesTx * numTxSectors);
		EMdgmMRZ_rxInfo* pRxInfo = (EMdgmMRZ_rxInfo*)pch;

		pData = pch;

		// end of datagram for safety testing
		char *pSOD = (char*)dgm;
		char *pEOD = pSOD + dgm->header.numBytesDgm; // first byte past end of data

													 // safety check - verify pointer is valid
		if (pData >= pEOD)
		{
			pData = NULL;
		}
	}

	return pData;
}

//f+/////////////////////////////////////////////////////////////////////////
//
//  getMRZSoundings
/*!
//  Get start ptr of MRZ beam soundings.
//
//  \param  void
//
//  \return Pointer to the start of rx info.
//
//  \author Ole-Jacob Enderud Jensen
//
//  \date   12.10.2018
*/
//f-/////////////////////////////////////////////////////////////////////////

char *getMRZSoundings(void* tgm)
{
	char *pData = NULL;

	// if this record is not empty
	if (tgm != NULL)
	{
		pEMdgmMRZ dgm = (pEMdgmMRZ)tgm;
		int sizeCommon = dgm->cmnPart.numBytesCmnPart;

		// Find rxInfo. Must take account of number of TX sectors.
		int numTxSectors = dgm->pingInfo.numTxSectors;

		//Get PingInfo
		char* pPI = (char*)(&(dgm->partition.dgmNum));
		pPI += 2;
		pPI += sizeCommon;

		pEMdgmMRZ_pingInfo pPingInfo = (pEMdgmMRZ_pingInfo)pPI;

		//Move to end of pingInfo
		char* pch = pPI + pPingInfo->numBytesInfoData; //At start of txsectors
		int nSkip = 0;
		pch += nSkip;

		//Get sector
		int nBytesTx = pPingInfo->numBytesPerTxSector;
		pEMdgmMRZ_txSectorInfo pSect = (pEMdgmMRZ_txSectorInfo)pch;

		//Move to start of rxInfo
		pch = (char*)pSect;
		pch += (nBytesTx * numTxSectors);
		EMdgmMRZ_rxInfo* pRxInfo = (EMdgmMRZ_rxInfo*)pch;
		int nSizeRXInfo = pRxInfo->numBytesRxInfo;

		// Now, find soundings.
		int numExtraDet = pRxInfo->numExtraDetectionClasses;
		pch += nSizeRXInfo;
		pch += numExtraDet * sizeof(EMdgmMRZ_extraDetClassInfo);
		EMdgmMRZ_sounding* pSoundings = (EMdgmMRZ_sounding*)pch;

		pData = pch;

		// end of datagram for safety testing
		char *pSOD = (char*)dgm;
		char *pEOD = pSOD + dgm->header.numBytesDgm; // first byte past end of data

													 // safety check - verify pointer is valid
		if (pData >= pEOD)
		{
			pData = NULL;
		}
	}

	return pData;
}

// Handle MRZ in this demonstration
void handleMRZ(char* mrz)
{
	long long tmptime;

	pEMdgmMRZ depths = (pEMdgmMRZ)mrz;
	// Make time into millisec
	tmptime = depths->header.time_sec;
	tmptime *= 1000;
	tmptime += (depths->header.time_nanosec / 1000000);

	//Structs above rxinfo can grow in newer datagram version. Use getMRZRxInfo to move to the correct location
	pEMdgmMRZ_rxInfo rxInfo = (pEMdgmMRZ_rxInfo)getMRZRxInfo((char*)depths);
	char* pData = (char*)(&(depths->sectorInfo[depths->pingInfo.numTxSectors]));
	unsigned short numExtraDet = ((pEMdgmMRZ_rxInfo)(pData))->numExtraDetectionClasses;

	//Structs above sounding can grow in newer datagram version. Use getMRZSoundings to move to the correct location
	pEMdgmMRZ_sounding depthList = (pEMdgmMRZ_sounding)getMRZSoundings((char*)depths);

	//Structs above pinginfo can grow in newer datagram version. Use getMRZPingInfo to move to the correct location
	pEMdgmMRZ_pingInfo pPingInfo = (pEMdgmMRZ_pingInfo)getMRZPingInfo((char*)depths);
	double blat = pPingInfo->latitude_deg;
	double blon = pPingInfo->longitude_deg;

	for (int i = 0; i < rxInfo->numSoundingsMaxMain; i++) {
		double dlat = (depthList + i)->deltaLatitude_deg;
		double dlon = (depthList + i)->deltaLongitude_deg;
		double depthRefPoint = (depthList + i)->z_reRefPoint_m;
		double lat = blat + dlat;
		double lon = blon + dlon;
		double depthReWaterline = depthRefPoint - depths->pingInfo.z_waterLevelReRefPoint_m;
		std::cout << std::fixed << std::setprecision(8) << lat << " ";
		std::cout << std::fixed << std::setprecision(8) << lon << " ";
		std::cout << std::fixed << std::setprecision(2) << depthReWaterline << " ";
		std::cout << tmptime << "\n";
	}
}
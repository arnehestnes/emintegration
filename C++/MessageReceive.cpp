
#include "MessageReceive.h"
#include <iterator>


//- Private global definitions ----------------------------------------------------------
std::shared_ptr<CMessageReceive> CMessageReceive::m_poInstance = nullptr;
std::recursive_mutex CMessageReceive::m_hConstrMutex;


//f+//////////////////////////////////////////////////////////////////////////
//
//  Name:	CMessageReceive (Default constructor)
/*!
//  Standard constructor
//
//  \param  NA
//
//  \return NA
//    
//  \author RN
//
//  \date   Sep. 2019
//
*/
// Copyright: Kongsberg Maritime
//f-//////////////////////////////////////////////////////////////////////////
CMessageReceive::CMessageReceive()
{
   //Beep(1200, 2000);

   // Set flag to allow input processing. 
   m_bKillMe = false;

   // Start main thread reading from input queue.
   std::thread ThreadReadQ([=] { JobReadQ();});
   ThreadReadQ.detach(); // Let it run on its own

   // Start timer thread ** CURRENTLY NOT USED **
   //std::thread ThreadTimer([=] { TimerHandler();});
   //ThreadTimer.detach(); // Let it run on its own 
}


//f+//////////////////////////////////////////////////////////////////////////
//
//  Name:	~CMessageReceive (destructor)
/*!
//  Clean up
//
//  \param  N/A
//
//  \return N/A
//    
//  \author RN
//
//  \date   Sep. 2019
*/
//f-//////////////////////////////////////////////////////////////////////////
CMessageReceive::~CMessageReceive()
{
   // Stop read/work thread.
   m_bKillMe = true;
   //Beep(600, 2000);
   Sleep(1500); // Just in case, allow threads to terminate before continuing and closing down
}


//f+/////////////////////////////////////////////////////////////////////////////////////
//
//  instance()
/*!
//  Function for creating a "singleton" (i.e. a single instance of the message receive class).
//  If no existing instance it present, create it. Return the pointer to this instance.
//  It can be used in the following way from the external parent running in a 
//  separate thread:
//  CMessageReceive::instance()->PutInQ(datagramstr, priority);
//
//  \param  NA
//
//  \return The single(ton) instance.
//
//  \author RN
//
//  \date   Sep. 2019
*/
//f-/////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<CMessageReceive> CMessageReceive::instance()
{
   // This lock is essentially only to avoid interference during the single moment
   // when the instance is created (in case several threads are using this function
   // simultaneously!).
   m_hConstrMutex.lock(); 
   
   // Create the single instance (object) if necessary
   if (nullptr == CMessageReceive::m_poInstance)
   {
      CMessageReceive::m_poInstance = std::make_shared<CMessageReceive>(); // Default constructor is used. 
   }
   m_hConstrMutex.unlock();

   return CMessageReceive::m_poInstance;
}


//f+/////////////////////////////////////////////////////////////////////////////////////
//
//  setDgmReturn
/*!
//  Set the mechanism used for returning the fully assembled #MRZ, #MCW ... 
//  datagrams to parent for further handling.
//
//  \param  phAssembledMessAvail Ptr to semaphore to be used to indicate complete datagram ready in buffer.
//
//  \param  phAssembledBuffMutex Ptr to semaphore to be used to guard return buffer.
//
//  \param  pstrAssemblDgm Ptr to buffer to be used for returning complete datagrams.
//
//  \return NA
//
//  \author RN
//
//  \date   Sep. 2019
*/
//f-/////////////////////////////////////////////////////////////////////////////////////
void CMessageReceive::setDgmReturn(Semaphore *phAssembledMessAvail, std::recursive_mutex *phAssembledBuffMutex, std::string *pstrAssemblDgm)
{
   if (nullptr != CMessageReceive::m_poInstance)
   {
      m_phAssembledMessAvail = phAssembledMessAvail;
      m_phAssembledBuffMutex = phAssembledBuffMutex;
      m_pstrAssemblDgm       = pstrAssemblDgm;
   }
}


//f+//////////////////////////////////////////////////////////////////////////
//
//  Name:	PutInQ
/*!
//  Put a copy of the incoming datagram in the asynchronous queue system to be 
//  handled when ready. The queue is guarded by a mutex to inhibit simultaneous
//  access (writing and/or reading).
//
//  \param  szInDgm  Ptr to char string with possible binary data. Note string is deleted after use.
//
//  \param  iDgmSize  Size of charstring.
//
//  \param  bHigPri Flag for high priority, (put first in queue). Default is low pri (last in queue).
//
//  \return N/A
//    
//  \author RN
//
//  \date   Sep. 2019.
//
*/
//f-//////////////////////////////////////////////////////////////////////////
void CMessageReceive::PutInQ(char *szInDgm, int iDgmSize, bool bHigPri)
{ 
   if (iDgmSize > 0)
   {
      // Make a std::string copy and delete the original char input buffer.
      std::string *pInDatagram = new std::string(szInDgm, iDgmSize);
      delete[] szInDgm; 

      // Get access to the input queue for storing of received datagram.
      hExternalQMutex.lock();
      // The (external) thread got ownership of the mutex.

      // Handle input according to priority.
      if (bHigPri)
      {
         // Put buffer first in input queue.
         m_ExternalQ.push_front(pInDatagram);
      }
      else
      {
         // Put buffer last in input queue.
         m_ExternalQ.push_back(pInDatagram);
      }

      //Release access to input queue
      hExternalQMutex.unlock(); 

      // Indicate datagram added in input queue.
      // This will trigger the 'JobReadQ' activity.
      m_hEntryInQ.notify();
   }
}



//f+//////////////////////////////////////////////////////////////////////////
//
//  Name:	JobReadQ
/*!
//  Wait for, and read, datagram or datagram fragments from the input queue. 
//  Process the datagram fragment involving assembly of a complete datagram 
//  if all fragmets are received.
//  The queue is guarded by a mutex to inhibit simultaneous access (reading and/or writing).
//  This function is run in a separate work thread which is started in the constructor.
//
//  \param  N/A
//
//  \return N/A
//    
//  \author Ronny Nergard
//
//  \date   Sep. 2019.
//
*/
//f-//////////////////////////////////////////////////////////////////////////
void CMessageReceive::JobReadQ()
{
   // Loop until stopped.
   do
   {
      // Wait for an entry in the queue (using seemaphor avoids polling and sleeping).
      m_hEntryInQ.wait();

      // One or more entries arrived in input queue.
      // Get mutex to avoid conflicting access to input queue.
      hExternalQMutex.lock();

      // The thread got ownership of the mutex.
      // Get first datagram/fragment in the queue.
      while (m_ExternalQ.size() > 0)
      {
         std::string *pstrCurrElem = m_ExternalQ.front();
         m_ExternalQ.pop_front();

         // Temporarily release access to queue to allow adding of new 
         // input datagram/fragment while processing current.
         hExternalQMutex.unlock();

         //***************************************************
         // This is where the received datagram/fragments are decoded/assembled.
         ProcessDatagram(pstrCurrElem);
         //***************************************************
         
         // Wait for access again to process next datagram/fragment in the queue or finish!
         hExternalQMutex.lock();
      }

      // No more input datagram/fragments in the queue. Release ownership of the mutex object
      hExternalQMutex.unlock();

   } while(!m_bKillMe); // Repeat until stopped. 
}


//f+//////////////////////////////////////////////////////////////////////////
//
//  Name:	ProcessDatagram
/*!
//  This is the startpoint of the assembly of the received fragmented datagram  
//  fragments. 
//  Note that this operation is performed asynchronously, in a separate thread,
//  decoupled from the external reception of datagram. 
//
//  Note that the current datagram format used for fragmented datagram elements
//  needs to be updated to handle dual head systems. When this update is implemented
//  ping count and fan index becomes part of the parameters used to identify each
//  datagram fragmen!
//
//  \param  pstrCurrFrag Ptr to a datagram/fragment constituting part of the complete datagram.
//
//  \return N/A
//    
//  \author RN
//
//  \date   Sep. 2019
//
*/
//f-//////////////////////////////////////////////////////////////////////////
void CMessageReceive::ProcessDatagram(std::string *pstrCurrFrag)
{
   // Store datagram/fragment into a sorted map. First generate a key for this purpose.
   std::string         strDgmType   = getTypeFromDgm(pstrCurrFrag);
   uint64_t            iCurrentTime = getNanoSecFromDgm(pstrCurrFrag);  // Note nano sec since Epoch 01.01.1970
   uint16_t            iPingCnt = 0; //getPingCntFromDgm(pstrCurrFrag); // Must be re-introduced when fragmented datagram format is updated.
   uint8_t             iFan     = 0; //getFanFromDgm(pstrCurrFrag);     // Must be re-introduced when fragmented datagram format is updated.
   EMdgmMpartition_def stPartitionId = getPartitionIdFromDgm(pstrCurrFrag);
   
   uint64_t iMilliseconds_since_epoch =
      std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);

   // Current key
   MessIdentParam MessId(strDgmType.c_str(), iCurrentTime, iPingCnt, iFan, stPartitionId.dgmNum, stPartitionId.numOfDgms, iMilliseconds_since_epoch);

   // The received fragments are stored in sequence.
   m_AssemblyMap.insert(std::make_pair(MessId, pstrCurrFrag));

   // Check if all fragments have arrived for the received MessId. 
   // Find the first fragment (i.e. datagram number 1) and the last fragment stored in the current sequence.
   DatagramFragList::iterator itLowerBound, itUpperBound, itAssemblyMap;

   // Find lower bound for current datagram fragment.
   MessIdentParam MessIdLower(strDgmType.c_str(), iCurrentTime, iPingCnt, iFan, 1, stPartitionId.numOfDgms);
   itLowerBound = m_AssemblyMap.lower_bound(MessIdLower);

   // Find upper bound for current datagram fragment.
   MessIdentParam MessIdUpper(strDgmType.c_str(), iCurrentTime, iPingCnt, iFan, stPartitionId.numOfDgms, stPartitionId.numOfDgms);
   itUpperBound = m_AssemblyMap.upper_bound(MessIdUpper);

  
   // Initially set a flag to indicate that the set of datagram fragments is not complete.
   bool bComplete = false;

   // Check if range is valid.
   if(itLowerBound != m_AssemblyMap.end())
   {
      // Now, do the check!
      int iDgmNum = 1; // First datagram fragment to look for.
      for(itAssemblyMap = itLowerBound; itAssemblyMap != itUpperBound; itAssemblyMap++, iDgmNum++)
      {
         if(itAssemblyMap->first.m_iDgmNum == iDgmNum && iDgmNum < itAssemblyMap->first.m_iNumOfDgms)
            continue; // Element found, continue testing for next in sequence.
         else if(itAssemblyMap->first.m_iDgmNum == iDgmNum && iDgmNum == itAssemblyMap->first.m_iNumOfDgms)
         {
            bComplete = true; // All fragments in datagram are received. Stop testing.
            break;
         }
         else
         {
            bComplete = false; // Fragment number checked for is not found, i.e. datagram is not complete. Stop this test and wait for more fragments.
            break;
         }
      }
   }

   if(bComplete)
   {
      // All datagram fragments have been received. Assemble this datagram.
      // First, move all relevant fragments to a separate map.
      DatagramFragList CompletedMap;
      for(itAssemblyMap = itLowerBound; itAssemblyMap != itUpperBound;)
      {
         std::swap(m_AssemblyMap[itAssemblyMap->first], CompletedMap[itAssemblyMap->first]);
         //CompletedMap[itAssemblyMap->first] = std::move(m_AssemblyMap.at(itAssemblyMap->first));
         itAssemblyMap = m_AssemblyMap.erase(itAssemblyMap);
      }
   
      // Do the datagram assembly.
      CMessageAssembly DgmAssembly;
      std::string strAssembledDgm;
      DgmAssembly.fnProcess(CompletedMap, strAssembledDgm);

      // The result is now ready and can be transferred to parent receipient    
      m_phAssembledBuffMutex->lock();
      *m_pstrAssemblDgm = strAssembledDgm; // The assembled datagram is copied into the return buffer.
      m_phAssembledBuffMutex->unlock();
      m_phAssembledMessAvail->notify();
   }

   // Finally, clean up the map containing the received and sorted datagram fragments
   // and remove any fragments that have become too old.
   // This is currently done in the simplest possible way!
   CMessageReceive::fnCleanStaleFragments(m_AssemblyMap);
}


//f+//////////////////////////////////////////////////////////////////////////
//
/*!
//  This function will remove any too old datagram fragments from the sorted 
//  list (map) of received fragments.
//  Currently this is perfored using the current (i.e. last received) datagram
//  time and then compare it to the storage time for each fragment in the list.
//  A difference of more than MAX_DGM_AGE results in the fragment being removed.
//
//  \param  FragList Reference to list containing a complete set of sorted datagram fragments.
//
//  \param strAssembledDgm Referense to variable used to return the complete assembled datagram.
//
//  \return Number of erased/removed datagram fragments.
//
//  \author RN
//
//  \date   Sep. 2019
//
*/
//f-//////////////////////////////////////////////////////////////////////////
int CMessageReceive::fnCleanStaleFragments(DatagramFragList &AssemblyMap)
{
   int iEraseCnt = 0;
   // Get current time.
   uint64_t iMilliseconds_since_epoch = 
      std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);

   // Check time for all fragments stored.
   DatagramFragList::iterator itFragList;
   for(itFragList = AssemblyMap.begin(); itFragList != AssemblyMap.end();)
   {
      if(iMilliseconds_since_epoch - itFragList->first.m_iAge > MAX_DGM_AGE_MS)
      {
         // Current fragment too old, remove.
         delete itFragList->second;                  // First, delete the actual stored datagram buffer.
         itFragList = AssemblyMap.erase(itFragList); // Remove this map element.
         iEraseCnt++;
      }
      else
         itFragList++;
   }
   return iEraseCnt;
}


//*************************************************************************
//** Below follows a number of utility functions.
//** These functions are global and not part of the CMessageReceive class
//** or any other (utility) class.
//*************************************************************************
std::string getTypeFromDgm(std::string *pstrDgm)
{
   std::string strType;
   pEMdgmHeader pHeader = (pEMdgmHeader)pstrDgm->data();
   if (pHeader)
   {
       strType = std::string(reinterpret_cast<const char *> (pHeader->dgmType), sizeof(pHeader->dgmType) / sizeof(pHeader->dgmType[0]));
   }
   return strType;
}

uint16_t getPingCntFromDgm(std::string *pstrDgm)
{
   uint16_t iPingCnt = 0; // Default

   std::string strType = getTypeFromDgm(pstrDgm);

   // Assume that #MRZ and #MWC might differ in *initial* format layout (currently, Sep. 2019 equal!).
   if(strType.compare("#MRZ") == 0)
   {
      pEMdgmMbody pBody = (pEMdgmMbody)getCommonPartFromDgm(pstrDgm);
      iPingCnt = pBody->pingCnt;
   }
   else if(strType.compare("#MWC") == 0)
   {
      pEMdgmMbody pBody = (pEMdgmMbody)getCommonPartFromDgm(pstrDgm);
      iPingCnt = pBody->pingCnt;
   }
   return iPingCnt;
}

uint8_t getFanFromDgm(std::string *pstrDgm)
{
   uint8_t iFanIndex = 0; // Default

   std::string strType = getTypeFromDgm(pstrDgm);

   // Assume that #MRZ and #MWC might differ in *initial* format layout (currently equal, Sep. 2019!).
   if(strType.compare("#MRZ") == 0)
   {
      pEMdgmMbody pBody = (pEMdgmMbody)getCommonPartFromDgm(pstrDgm);
      iFanIndex = pBody->rxFanIndex;
   }
   else if(strType.compare("#MWC") == 0)
   {
      pEMdgmMbody pBody = (pEMdgmMbody)getCommonPartFromDgm(pstrDgm);
      iFanIndex = pBody->rxFanIndex;
   }
   return iFanIndex;
}

uint64_t getMSecFromDgm(std::string *pstrDgm)
{
   pEMdgmHeader pHeader = (pEMdgmHeader)pstrDgm->data();

   if (pHeader)
   {
      double nano = pHeader->time_nanosec;
      double sec = pHeader->time_sec;

      if (!std::isnan(nano) && !std::isnan(sec))  // Both nano and sec must be a number.
         return static_cast<uint64_t>((nano / 1000000) + (sec * 1000));
      else
         return 0;
   }
   return 0;
}

uint64_t getNanoSecFromDgm(std::string *pstrDgm)
{
   pEMdgmHeader pHeader = (pEMdgmHeader)pstrDgm->data();

   if (pHeader)
   {
      double nano = pHeader->time_nanosec;
      double sec = pHeader->time_sec;

      if (!std::isnan(nano) && !std::isnan(sec))  // Both nano and sec must be a number.
         return static_cast<uint64_t>((nano)+(sec * 1000000000));
      else
         return 0;
   }
   return 0;
}

EMdgmMpartition_def getPartitionIdFromDgm(std::string *pstrDgm)
{  
   EMdgmMpartition_def stPartitionId ={0, 0};

   if(pstrDgm != nullptr)
   {
      std::string strType = getTypeFromDgm(pstrDgm);

      // Assume that #MRZ and #MWC might differ in *initial* format layout (currently equal, Sep. 2019!).
      if(strType.compare("#MRZ") == 0)
      {
         pEMdgmMRZ pOrigDepth = (pEMdgmMRZ)pstrDgm->data();
         stPartitionId = pOrigDepth->partition;
      }
      else if(strType.compare("#MWC") == 0)
      {
         pEMdgmMWC pWCol = (pEMdgmMWC)pstrDgm->data();
         stPartitionId = pWCol->partition;
      }
   }
   return stPartitionId;
}



char* getCommonPartFromDgm(std::string *pstrDgm)
{  
   // 'Common part' Currently relevant for #MRZ and #MWC only.
   pEMdgmHeader pHeader = (pEMdgmHeader)pstrDgm->data();
   char*        pData   = nullptr;

   if (pHeader)
   {
      std::string strType = getTypeFromDgm(pstrDgm);
      
      // Assume that #MRZ and #MWC might differ in *initial* format layout (currently equal, Sep. 2019!).
      if(strType.compare("#MRZ") == 0)
      {
         pEMdgmMRZ pDepth = (pEMdgmMRZ)pstrDgm->data();
         char *pCommon = (char*)&pDepth->cmnPart;
         pData = pCommon;
      }
      else if(strType.compare("#MWC") == 0)
      {
         pEMdgmMWC pWCol = (pEMdgmMWC)pstrDgm->data();
         char *pCommon = (char*)&pWCol->cmnPart;
         pData = pCommon;
      }

      // Get adress range of datagram for simple validity testing
      const char* pSOD = (reinterpret_cast<const char*> (pHeader));
      const char *pEOD = pSOD + pHeader->numBytesDgm; // First byte past end of datagram

      // Simple validity check
      if (pData < pSOD || pData >= pEOD)
      {
         pData = nullptr; // Invalid.
      }
   }
   // Note: This return pointer is associated with the buffer containd in the 
   // string object pointed to by pstrDgm input argument.
   return pData; 
}



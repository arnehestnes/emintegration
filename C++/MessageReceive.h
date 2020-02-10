

#ifndef MESSAGERECEIVE_H
#define MESSAGERECEIVE_H


#include <list>
#include <Shlwapi.h> // Currently for Sleep() only!

//#include <NDFDefs.h>
#include "EMdgmFormat.h"
#include "MessageDefragDef.h"
#include "MessageAssembly.h"

// Some global utility functions.
std::string getTypeFromDgm(std::string *pstrCurrFrag);    //!< Get type of datagram, e.g. #MRZ, #MWC.
uint16_t    getPingCntFromDgm(std::string *pstrCurrFrag); //!< Get ping count from datagram (only relevant for #MRZ and #MWC)
uint64_t    getNanoSecFromDgm(std::string *pstrCurrFrag); //!< Get no of nano sec. since Epoch from datagram.
uint64_t    getMSecFromDgm(std::string *pstrCurrFrag);    //!< Get no of milli sec. since Epoch from datagram.
uint8_t     getFanFromDgm(std::string *pstrCurrFrag);     //!< Get fan index from datagram (only relevant for #MRZ and #MWC)
char*       getCommonPartFromDgm(std::string *pstrCurrFrag);//!< Get the pointer to the datagram common part (only relevant for #MRZ and #MWC).
EMdgmMpartition_def getPartitionIdFromDgm(std::string *pstrCurrFrag); //!< Get the partition data from datagram (only relevant for #MRZ and #MWC). 


class CMessageReceive
{
private:
   // Private members

   static std::shared_ptr<CMessageReceive> m_poInstance; //!< Pointer making sure that only one instance of the message receive function is started (singleton).

   std::recursive_mutex hExternalQMutex;   //!< Mutex guardig access to input queue of datagram fragments.
   Semaphore            m_hEntryInQ;       //!< Indicates if a datagram fragment has arrived.
   std::mutex           hTimerMutex;       //!< Mutex guardig access to list of timer items.

   std::list<std::string*> m_ExternalQ;    //!< Input queue of received datagram fragments waiting to be processed.
   std::list<TimerItem*>   m_TimerQ;       //!< List of timer items being counted down.

   std::thread *thrReadQ;                  //!< Work thread reading from input queue and processing received datagram fragments.
   bool m_bKillMe;                         //!< Set to true to stop input processing and terminate.
 
   DatagramFragList m_AssemblyMap;         //!< Queue for sorted #MRZ and #MWC datagram fragments before handling (assembly).

   // Items for used reporting assembled datagrams parent level.
   // Note these items are provided by the parent level using the public 'setDgmReturn' function.
   Semaphore            *m_phAssembledMessAvail; //!< Ptr to semaphore to be set when an assembled datagram is ready
   std::recursive_mutex *m_phAssembledBuffMutex; //!< Ptr to mutex used to guard return buffer.
   std::string          *m_pstrAssemblDgm;       //!< Ptr to buffer for return of assembled datagrams (guarded by previous mutex).

public:
   
   CMessageReceive(); // Default constructor
   ~CMessageReceive();

   static std::shared_ptr<CMessageReceive> instance(); //!< Function creating the only instance of the message receive function and securing access.
   static std::recursive_mutex m_hConstrMutex;         //!< Mutex securing only one call to the constructor (e.g. in case of input present at startup).


   //! Enter fragments into an asynchronous input queue system.
   void PutInQ(char *szInDgm, int iDgmSize, bool bHigPri = false); 
   
   //! Set semaphore used to indicate result ready, i.e. datagram assempled.
   void setDgmReturn(Semaphore *hAssembledMessAvail, std::recursive_mutex *hAssembledBuffMutex, std::string *strAssemblDgm);

private:
   // Private functions
   
   //! Read input queue and start handling of received fragments. This function contains the main processing entry point.
   void JobReadQ();

   //! The main processing function i.e. entry point.
   void ProcessDatagram(std::string *pstrCurrElem);
   
   //! Remove any datagram fragments that are too old.
   int  fnCleanStaleFragments(DatagramFragList &AssemblyMap);


};

#endif
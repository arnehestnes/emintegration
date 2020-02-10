
#include "MessageAssembly.h"


//f+//////////////////////////////////////////////////////////////////////////
//
//  Name:	CMessageAssembly()
/*!
//  Default constructor
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
//f-//////////////////////////////////////////////////////////////////////////
CMessageAssembly::CMessageAssembly()
{
   m_receivedCnt  = 0;
   m_processedCnt = 0;
   m_parts        = 0;
   m_dropped      = 0;
}


//f+//////////////////////////////////////////////////////////////////////////
//
//  Name:	fnProcess
/*!
//  This function will receive a complete list (map) of all datagram fragments
//  constituting a complete datagram. The fragments are assembled 
//  (i.e. defragmented) into a complete datagram which is returned.
//
//  \param  FragList Reference to list containing a complete set of sorted datagram fragments.
//
//  \param strAssembledDgm Referense to variable used to return the complete assembled datagram.
//
//  \return Returned datagram valid or not.
//
//  \author RN
//
//  \date   Sep. 2019
//
*/
//f-//////////////////////////////////////////////////////////////////////////
bool CMessageAssembly::fnProcess(DatagramFragList &FragList, std::string &strAssembledDgm)
{
   DatagramFragList::iterator itFragList;
   std::string strFrag;

   int iDgmNum = 1;
   bool bFirst = false;
   bool bValid = true; // Assume success!

   int iDgmCnt, iExpectedDgmCnt=0;
   for(itFragList = FragList.begin(), iDgmCnt= 0; itFragList != FragList.end(); itFragList++, iDgmCnt++)
   {
      if(itFragList->first.m_iDgmNum == 1)
      {
         // First datagram fragment.
         bFirst = true;
         strAssembledDgm = *itFragList->second;
         delete itFragList->second; // Delete the original datagram input buffer;
         strAssembledDgm.erase(strAssembledDgm.size() - 4, 4); // Remove length ending of datagram fragment  = 4 bytes.
         iExpectedDgmCnt = itFragList->first.m_iNumOfDgms;
      }
      else
      {
         // Only add fragments if first fragment has been found.
         if(bFirst)
         {
            strFrag = *itFragList->second;
            delete itFragList->second; // Delete the original datagram input buffer;
            strFrag.erase(0, sizeof(EMdgmHeader) + sizeof(EMdgmMpartition));// Removing Header + partition of part datagram = 24 Bytes.
                                                                            // This must be updated when part datagrams are changed.
            
            strFrag.erase(strFrag.size() - 4, 4); // Remove length ending of datagram fragment = 4 bytes. 
            strAssembledDgm += strFrag;
         }
         else
         {
            // Error, datagram assembly is aborted as the first fragment was not found first. 
            bValid = false;
            break;
         }
      }
   }

   if(bValid)
   {
      // Check if the expected number of fragments have been processed.
      if(iDgmCnt == iExpectedDgmCnt)
      {
         // Ok!
         // Complete the datagram assemply by setting a new datagram sequence number (i.e. 1/1) 
         // and a new datagram size at start and end of datagram.
         const pMheaderRec pstrAssembledDgm = (const pMheaderRec)(strAssembledDgm.data());
         pstrAssembledDgm->partition.numOfDgms = 1;
         pstrAssembledDgm->partition.dgmNum    = 1;
         uint32_t iLen = strAssembledDgm.size() + 4; // Calculating new size. + 4 bytes to include length ending into total-size.
         std::string strLen = std::string((char*)(&iLen), 4);
         pstrAssembledDgm->header.numBytesDgm = iLen;
         strAssembledDgm += strLen;
      }
   }
   else
   {
      // Error, not able to assemble datagram.
      strAssembledDgm = ""; // Make sure returned string is empty.
   }
   return bValid; 
}



#ifndef MESSAGEDEFRAGDEF_H
#define MESSAGEDEFRAGDEF_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>


const uint64_t MAX_DGM_AGE_MS = 2000; // Max age in msec of received datagram fragment stored in sorted list.  


// Simple implementation of a semaphore using a C++ condition variable.
// (C++ 11 don't have semaphores) 
class Semaphore 
{
public:
   Semaphore (int iCount = 0)
      : m_iCount(iCount) {}

   inline void notify()
   {
      std::unique_lock<std::mutex> lock(m_mtx);
      m_iCount++;
      m_cv.notify_one();
   }

   inline void wait()
   {
      std::unique_lock<std::mutex> lock(m_mtx);

      while(m_iCount == 0)
      {
         m_cv.wait(lock);
      }
      m_iCount--;
   }

private:
   std::mutex m_mtx;
   std::condition_variable m_cv;
   int m_iCount;
};


typedef struct tagTimerItem
{
   int         iCnt;           //!< Timeout in sec.
   std::string strReturnEv;    //!< Name of return event.
   std::string strReturnData;  //!< User data to be returned.

                               // Default Constructor
   tagTimerItem(int iDuration, std::string strEv, std::string strData)
   {
      iCnt = abs (iDuration);                          //!< Timing always positive.

      if (strEv.length() > 0)
         strReturnEv = strEv; 
      else 
         strReturnEv = "Unknown_Timer";                //!< Unknown timer

      if (strReturnEv.compare("Unknown_Timer") == 0)
         strReturnData = "No settings for this timer"; //!< Just a comment
      else
         strReturnData = strData;                      //!< User data if any.
   };

   // Default Constructor
   tagTimerItem()
   {
      iCnt          = 0;                            //!< No timing, expires immediately (maximum 1 sec.)
      strReturnEv   = "Unknown_Timer";              //!< Unknown timer
      strReturnData = "No settings for this timer"; //!< Just a comment
   };

   // Copy ctor
   tagTimerItem(tagTimerItem &Orig)
   {
      iCnt          =  Orig.iCnt; 
      strReturnEv   =  Orig.strReturnEv; 
      strReturnData =  Orig.strReturnData; 
   }

   // Assignment
   void operator=(const tagTimerItem &Right)
   {
      iCnt          =  Right.iCnt; 
      strReturnEv   =  Right.strReturnEv; 
      strReturnData =  Right.strReturnData; 
   }

   // Postfix decrement operator
   tagTimerItem operator-- (int i)
   {
      if (i != 0)
         iCnt = iCnt - i;
      else
         iCnt--;
      return *this;
   }

   // Decrement timer counter directly and return new counter value
   int operator-= (int i)
   {
      iCnt-=i;
      return iCnt;
   }
}TimerItem, *pTimerItem;



//! Structure used as key in sorted map of #MRZ and #MWC datagram fragments.
typedef struct tagMessIdent
{
   char     m_cDgmType[4];  //!< Name of datagram, e.g. "#MRZ".
   uint64_t m_iCurrentTime; //<! Time in nano sec. since Epoch 1970-01-01 fetched from datagram fragment header.
   uint16_t m_iPingCnt;     //!< Ping count
   uint8_t  m_iRxFanIndex;  //!< Fan index (0, 1, 2, 3 where 0, 1 is for swath 0 and 3, 4 is for swath 1)
   uint16_t m_iDgmNum;      //!< Fragment number.
   uint16_t m_iNumOfDgms;   //!< Total number of fragments in Datagram.
   uint64_t m_iAge;         //<! Time in milli sec. since Epoch 1970-01-01 set when stored.

                                        
   //! Default constructor
   tagMessIdent()
   {
      memset(m_cDgmType, '\0', 4);
      m_iCurrentTime = 0;
      m_iPingCnt     = 0;
      m_iRxFanIndex  = 0;
      m_iDgmNum      = 0;
      m_iNumOfDgms   = 0;
      m_iAge         = 0;
   };

   //! Full constructor
   tagMessIdent(const char *pDgmType, uint64_t iCurrentTime, uint16_t iPingCnt = 0, uint8_t iRxFanIndex = 0, uint16_t iDgmNum = 0, uint16_t iNumOfDgms = 0, uint64_t iAge = 0)
   {
      memcpy(m_cDgmType, pDgmType, 4);
      m_iCurrentTime = iCurrentTime;
      m_iPingCnt     = iPingCnt;
      m_iRxFanIndex  = iRxFanIndex;
      m_iDgmNum      = iDgmNum;
      m_iNumOfDgms   = iNumOfDgms;
      m_iAge         = iAge;
   };


   //! Copy constructor
   tagMessIdent(const tagMessIdent &Orig)
   {
      memcpy(m_cDgmType, Orig.m_cDgmType, 4);
      m_iCurrentTime = Orig.m_iCurrentTime;
      m_iPingCnt     = Orig.m_iPingCnt;   
      m_iRxFanIndex  = Orig.m_iRxFanIndex;
      m_iDgmNum      = Orig.m_iDgmNum;
      m_iNumOfDgms   = Orig.m_iNumOfDgms;
      m_iAge         = Orig.m_iAge;

   }

   //! Assignment operator.
   void operator=(const tagMessIdent &Right)
   {
      memcpy(m_cDgmType, Right.m_cDgmType, 4);
      m_iCurrentTime = Right.m_iCurrentTime;
      m_iPingCnt     = Right.m_iPingCnt;
      m_iRxFanIndex  = Right.m_iRxFanIndex;
      m_iDgmNum      = Right.m_iDgmNum;
      m_iNumOfDgms   = Right.m_iNumOfDgms; 
      m_iAge         = Right.m_iAge;   }


   //! Comparison
   bool operator== (const tagMessIdent &Right) 
   {
      if (memcmp(m_cDgmType, Right.m_cDgmType, 4) == 0 &&
          m_iCurrentTime == Right.m_iCurrentTime &&
          m_iPingCnt     == Right.m_iPingCnt &&
          m_iRxFanIndex  == Right.m_iRxFanIndex &&
          m_iDgmNum      == Right.m_iDgmNum &&
          m_iNumOfDgms   == Right.m_iNumOfDgms)
         // Note Age is not included
         return true;  // Equal
      else
         return false; // Different
   }

   //! Less than comparison
   bool operator < (const tagMessIdent &Right)
   {
      // Note Age is not included in comparison

      int itypeCmp = memcmp(m_cDgmType, Right.m_cDgmType, 4); // Test datagram type string
      if(itypeCmp < 0)
         return true;  // Less than
      else if(itypeCmp > 0)
         return false;  // Larger than
      else if(itypeCmp == 0)
      {
         // Same type and testing must be continued. Test time
         if(m_iCurrentTime < Right.m_iCurrentTime)
            return true;  // Less than
         else if(m_iCurrentTime > Right.m_iCurrentTime)
            return false;  // Larger than
         else if(m_iCurrentTime == Right.m_iCurrentTime)
         {
            // Same time and testing must be continued. Test Ping no.
            if(m_iPingCnt < Right.m_iPingCnt)
               return true;  // Less than
            else if(m_iPingCnt > Right.m_iPingCnt)
               return false;  // Larger than
            else if(m_iPingCnt == Right.m_iPingCnt)
            {
               // Same ping count and testing must be continued. Test fan index.

               if(m_iRxFanIndex < Right.m_iRxFanIndex)
                  return true;  // Less than
               else if(m_iRxFanIndex > Right.m_iRxFanIndex)
                  return false;  // Larger than
               else if(m_iRxFanIndex == Right.m_iRxFanIndex)
               {
                  // Same fan index and testing must be continued. Test fragment no.
                  if(m_iDgmNum < Right.m_iDgmNum)
                     return true;  // Less than
                  else if(m_iDgmNum > Right.m_iDgmNum)
                     return false;  // Larger than
                  else if(m_iDgmNum == Right.m_iDgmNum)
                  {
                     // Same fragment no and testing must be concluded.
                     return false; // I.e. two equals should never happen.
                  }
               }
            }
         }
      }
   }
}MessIdentParam, *MessIdentParamPnt;


//! Class used for comparing MessIdentParam keys when sorting into map.
/*!
//  This is a user defined compare for a map containing MessIdentParam
//  as keys. 
*/
class MessIdentParamCmp /*: public std::binary_function<MessIdentParam, MessIdentParam, bool>*/ 
{
public:   
   MessIdentParamCmp(){};
   bool operator()(const MessIdentParam &x, const MessIdentParam &y) const
   {
      // Note Age is not included in comparison

      int itypeCmp = memcmp(x.m_cDgmType, y.m_cDgmType, 4); // Test datagram type string
      if(itypeCmp < 0)
         return true;  // Less than
      else if(itypeCmp > 0)
         return false;  // Larger than
      else if(itypeCmp == 0)
      {
         // Same type and testing must be continued. Test time
         if(x.m_iCurrentTime < y.m_iCurrentTime)
            return true;  // Less than
         else if(x.m_iCurrentTime > y.m_iCurrentTime)
            return false;  // Larger than
         else if(x.m_iCurrentTime == y.m_iCurrentTime)
         {
            // Same time and testing must be continued. Test Ping no.
            if(x.m_iPingCnt < y.m_iPingCnt)
               return true;  // Less than
            else if(x.m_iPingCnt > y.m_iPingCnt)
               return false;  // Larger than
            else if(x.m_iPingCnt == y.m_iPingCnt)
            {
               // Same ping count and testing must be continued. Test fan index.

               if(x.m_iRxFanIndex < y.m_iRxFanIndex)
                  return true;  // Less than
               else if(x.m_iRxFanIndex > y.m_iRxFanIndex)
                  return false;  // Larger than
               else if(x.m_iRxFanIndex == y.m_iRxFanIndex)
               {
                  // Same fan index and testing must be continued. Test fragment no.
                  if(x.m_iDgmNum < y.m_iDgmNum)
                     return true;  // Less than
                  else if(x.m_iDgmNum > y.m_iDgmNum)
                     return false;  // Larger than
                  else if(x.m_iDgmNum == y.m_iDgmNum)
                  {
                     // Same fragment no and testing must be concluded.
                     return false; // I.e. two equals should never happen, except when empty map.
                  }
               }
            }
         }
      }
   }
};


//! Type for map container sorting #MRZ and #MWC datagram fragments.
typedef std::map<MessIdentParam, std::string*, MessIdentParamCmp> DatagramFragList;

#endif
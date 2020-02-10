#ifndef MESSAGEASSEMBLY_H
#define MESSAGEASSEMBLY_H

#include <chrono>
#include "MessageDefragDef.h"
//#include <NDFDefs.h>
#include "EMdgmFormat.h"

// Help struct for accessing partition info in #MRZ, #MWC (copied from NDFDefs.h)
typedef struct{
   EMdgmHeader header;
   EMdgmMpartition partition;
   EMdgmScommon common;
} MheaderRec, *pMheaderRec;

//! Message defragment class. For assembling fragmented datagrams (e.g. #MRZ, #MWC).
/*!
// This class is implemented for the purpose of reconstructing fragmented M-datagrams.
//
*/
class CMessageAssembly
{

public:
   //! Default constructor
   explicit CMessageAssembly();
   
   ~CMessageAssembly() {};

   //! Prosess the set of complete datagram fragment, i.e. assemble the datagram fragments into a complete datagram.
   bool fnProcess(DatagramFragList &FragList, std::string &strAssembledDgm);

private:
   bool m_bValid;

   // Statistics (currently not used)
   unsigned int m_receivedCnt;   //!< Total received packages.
   unsigned int m_processedCnt;  //!< Total processed packages.
   unsigned int m_parts;         //!< Number of parts pr. processing sample.
   unsigned int m_base;          //!< Number of base pr. processing sample.
   unsigned int m_dropped;       //!< Number of dropped packages.

};

#endif
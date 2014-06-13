/*
 *  Slim5OutputParser.h
 *  control
 *
 *  Created by Riccardo Di Sipio on 13/02/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */


#include <iostream>
#include <fstream>

//ROOT histogramming
# include <TTree.h>
# include "TObject.h"
# include <TString.h>
# include <TClass.h>
# include "TH1F.h"
# include "TH1I.h"
# include "TH2I.h"
# include "TF1.h"


#define WORD_SIZE           4

#include "bit_operations.h"
#include "EventManager.h"

enum _VERBOSITY {
  kSILENT = 0,
  kMEDIUM = 1,
  kDEBUG  = 2
};

#define IF_VERBOSE(level)  if(m_verbosity >= level)  
#define IF_DEBUG           IF_VERBOSE(kDEBUG)

class Slim5OutputParser
{
public:
                            Slim5OutputParser();
                            Slim5OutputParser(ifstream& file);
        virtual             ~Slim5OutputParser();
    
	virtual void        setStartingBlock(_BLOCK start_from_block);
        virtual bool        FindEvent();
        virtual _EVENT&     GetEvent(); 
        virtual void        Dump(ostream& out) { return m_event.dump(out); };
        virtual void        setVerbosity(_VERBOSITY level);
	virtual bool        IsEndOfFile() const { return m_reached_end_of_file;}
	virtual void        dumpErrors(ostream& out);
	virtual TH1F*       getHistErrors() { return m_hist_errors; };
        virtual void 	    resetErrorStats();
	virtual bool        attachFile(ifstream& file);
	inline  bool        fileIsAttached()           { return m_file != 0 ? true : false;  }

protected:
        ifstream*           m_file;
        unsigned long       m_currentBlock;
        unsigned long       m_filePosition;                  //position of the "get" pointer
        EventManager        m_event;
        _EVENT*             m_lastEvent;
	_VERBOSITY          m_verbosity;
	int                 m_lastHitBlockLine;
        bool                m_reached_end_of_file;
        
        //Error handling
	TH1F*               m_hist_errors;

	//functions
	virtual bool        initialize();

        class errImpossibleNewEvent : public exception
        {
            virtual const char* what() const throw()
            {
                return "@ Impossible to create new event.";
            }
        } impossibleNewEvent;
        
        class errNoROD : public exception
        {
            virtual const char* what() const throw()
            {
                return "@ ROD header not found.";
            }
        } RODHeaderNotFound;
        
        class errOrdinal : public exception
        {
            virtual const char* what() const throw()
            {
                return "@ Event ordinal number doesn\'t match number of events found.";
            }
        } wrongOrdinal;
        
        class errFlags : public exception
        {
            virtual const char* what() const throw()
        {
                return "@ Error: event flags are different from ones of the previous event.";
        }
        } wrongFlags;
        
        class errBCO : public exception
        {
            virtual const char* what() const throw()
        {
                return "@ BCO counter doesn\'t follow the BCO counter of the previous event.";
        }
        } wrongBCO;

	class errBX : public exception
        {
            virtual const char* what() const throw()
        {
                return "@ BX counter doesn\'t follow the BX counter of the previous event.";
        }
        } wrongBX;
        
        class errLvl1ID : public exception
        {
            virtual const char* what() const throw()
        {
                return "@ Lvl1ID counter doesn\'t follow the LVL1ID counter of the previous event..";
        }
        } wrongLvl1ID;
        
        class errTimestamp : public exception
        {
            virtual const char* what() const throw()
        {
                return "@ Wrong Timestamp.";
        }
        } wrongTimestamp;
        
        class errTimestampNonConsistent : public exception
        {
            virtual const char* what() const throw()
        {
                return "@ Non consistent timestamps.";
        }
        } wrongTimestampNonConsistent;
        
        
        class errLessTriggerInfo : public exception
        {
            virtual const char* what() const throw()
        {
                return "@ Less trigger informations than expected.";
        }
        } lessTrigger;
        
        class errMoreTrigger : public exception
        {
            virtual const char* what() const throw()
        {
                return "@ More trigger informations than expected.";
        }
        } moreTrigger;
        
        class errInputLine : public exception
        {
            virtual const char* what() const throw()
        {
                return "@ Selected input line doesn\'t exist.";
        }
        } wrongInputLine;
        
        
        class warnLastWord : public exception
        {
            virtual const char* what() const throw()
        {
                return "@ Layer last word reached but bit 23 is not set.";
        }
        } warnLastWord;
        
        
        
        class errHitBlockWords : public exception
        {
            virtual const char* what() const throw()
        {
                return "@ Hit block for selected line has fewer words than expected.";
        }
        } wrongHitBlockWords;
        
        
        class errLength : public exception
        {
            virtual const char* what() const throw()
        {
                return "@ Event length is different from calculated length.";
        }
        } wrongLength;
        
        class errChecksum : public exception
        {
            virtual const char* what() const throw()
            {
                return "@ Wrong checksum.";
            }
        } wrongChecksum;

        class errSourceID : public exception
        {
            virtual const char* what() const throw()
            {
                return "@ Wrong Source ID ";
            }
        } wrongSourceID;

        
        /*
         class err : public exception
         {
             virtual const char* what() const throw()
         {
                 return "@ .";
         }
         } ;
         */
        
};



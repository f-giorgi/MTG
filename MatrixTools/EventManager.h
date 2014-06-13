/*
 *  EventManager.h
 *  
 *
 *  Created by Riccardo Di Sipio on 06/02/08.
 *
 */

#ifndef _EVENTMANAGER_H
#define _EVENTMANAGER_H

#ifndef LAYERS
#define LAYERS 8
#endif

#define MAXBUFFERSIZE 3

#include <vector>
#include <deque>
#include <exception>
#include <stdexcept>
#include <iostream>
//#include <ostream>
#include <iomanip>



using namespace std;

#include "bit_operations.h"
#include "error_management.h"

#define STARTING_INDEX              -1

#ifdef _DEBUG
#define ASSERT_INDEX()              if( m_currentEvent == STARTING_INDEX || (unsigned int) m_currentEvent >= m_events.size() )      throw wrongIndex; 
#define ASSERT_INDEX_EXISTS(i)      if( m_currentEvent < 0 || i >= m_events.size() )     throw wrongIndex;
#define ASSERT_EVENT_HAS_STARTED()  if(!hasStarted())                                           throw eventNotStarted; 
#define ASSERT_ROB_HAS_STARTED()    if(!isInsideROB())                                          throw robNotStarted; 
#define ASSERT_ROD_HAS_STARTED()    if(!isInsideROD())                                          throw rodNotStarted; 
#endif /* _DEBUG */


#define SEPARATOR  "\t/--------------------------------/ "

typedef struct { 
    _CHUNK raw;
    unsigned int z() { return GET_HITS_Z(raw); };
    unsigned int w() { return GET_HITS_W(raw); };
    unsigned int x() { return GET_HITS_X(raw); };
    unsigned int y() { return GET_HITS_Y(raw); };
} _HIT;

typedef struct {
    _BLOCK   hit_block_position;
    unsigned int number_of_words;
    unsigned int number_of_hits;
    vector<_HIT> hits;
} _LAYER;


enum _Status { 
    empty       = 0, // 0x000, 
    started     = 1, // 0x001, 
    end_trigger = 2, // 0x010, 
    complete    = 3, // 0x011 
    corrupted   = 4 //  0x100
} ;

const std::string StatusString[8] = {"Empty", 
				     "Started", 
				     "Trigger Ended but not completed", 
				     "Completed", 
				     "Corrupted", "", "",
				     "Has warnings"};

enum _triggerType {
    NONE        = 0, //  0x0000,
    AM          = 8, //  0x1000,
    INT         = 4, //  0x0100,
    AM_INT      = 12, // 0x1100,
    EXT         = 1   // 0x0001
} ;

const std::string TriggerString[13] = {"None", 
				       "EXT", "", "", 
				       "INT", "", "", "",
				       "AM", "", "", "", 
				       "AM+INT"};

typedef struct {
    _Status             status; 
    _triggerType        type;   //1000 AM, 0100 int, 0001 ext, 1100 AM+int
    _BLOCK              starting_block, ending_block;    
} _TRIGGER;


enum _sourceID {
    SLAVE  = 0,
    MASTER = 1
};


typedef struct {
    _BLOCK raw;
} _AM_DATA;


typedef struct {
    _Status             status;
    _BLOCK              starting_block, ending_block;
    unsigned int        timestamp;
    _sourceID           source_identifier;
    unsigned int        format_version;
    unsigned int        flags;
    unsigned int        BX;     //BX counter
    unsigned int        BCO;    //BCO counter
    unsigned int        lvl1id;    //progressive counter 
    _TRIGGER            trigger;
    _BLOCK              trigger_type;
    _BLOCK              event_type;
    unsigned int        N_AM;      //trackwords    
    deque<_AM_DATA>     AM_data;
    vector<_LAYER>      layers;
  //unsigned int        tot_hits;
    unsigned int        tot_hits() {
      unsigned int th = 0; 
      for(unsigned int l=0; l < layers.size(); l++)
	  {
	    th += layers[l].number_of_hits;
	  }
      return th;
    }
} _ROD;


typedef struct {
    _BLOCK              starting_block, ending_block;
    _Status             status;
    unsigned int        event_type;
    _ROD                rod;
} _ROB;


typedef struct {
  _CHUNK                raw;
  unsigned int          timestamp;
} _SCINTFLAG;


//Holds informations about a single event (2 ROB)
typedef struct {
    int                 run_number;
    _Status             status; 
    unsigned int        warnings;
    unsigned long       expected_length;
    unsigned int        length;
    unsigned long       ordinal;
    _BLOCK              starting_block, ending_block;
    bool                AM_enabled;
    unsigned int        timestamp;
    vector<_ROB>        ROBs;
    int                 number_of_robs() { return ROBs.size(); };
    vector<_ERROR>      errors_found;
    bool                scint_flag;
    deque<_SCINTFLAG>   scint_flags;
} _EVENT; 

typedef deque<_EVENT> eventBuffer;



//////////////////////////////////////////////////////////////////////////////
class PIXEL {
 public:
  static unsigned int GetPxCol(int rawhit)   { return PIXEL_GetX(rawhit);};
  static unsigned int GetPxRow(int rawhit)   { return PIXEL_GetY(rawhit);};
  static unsigned int GetPxIdx(int rawhit)   { return (GetPxCol(rawhit)*32 + GetPxRow(rawhit));};
  static unsigned int GetHitTS(int rawhit)   { return PIXEL_GetTS(rawhit);};
  static unsigned int GetParity(int rawhit)  { return PIXEL_GetParity(rawhit);};
  static unsigned int GetChipN(int rawhit)   { return PIXEL_GetChipN(rawhit);};
  static unsigned int GetDacStep(int rawhit) { return PIXEL_GetDAC(rawhit);};

  static unsigned int GetTrgCnt(int rawhit)      { return PIXEL_GetTrgCnt(rawhit);};
  static unsigned int GetMissCalTrg(int rawhit)  { return PIXEL_GetMissCalTrg(rawhit);};
  static unsigned int GetMissCalSoS(int rawhit)  { return PIXEL_GetMissCalSoS(rawhit);};


  static bool isEoE(int rawhit)              { return PIXEL_IsEoE(rawhit);};
  static bool isDummyWord(int rawhit )       { return PIXEL_IsDummyWord(rawhit);};

  static int EvalParity(int rawhit){
      int parity=0;
      int myhit=rawhit;
      for (int i=0; i<24 ; i++){
     	parity = parity ^ (unsigned(myhit)&1); 
	myhit  = myhit>>1;
	
/* 	if( (unsigned(myhit)&1) == 0){ */
/*    	  parity++; */
/* 	  //cout<<"+"; */
/* 	  myhit=myhit>>1; */
/* 	}else{  */
/* 	  parity--; */
/* 	  //cout<<"-"; */
/* 	  myhit=myhit>>1; */
/* 	} */
      }
      //cout<<(hex)<<myhit<<endl;
      return parity;
    }; 
};




///////////////////////////////////////////////////
class MAPS {
 public:
    static unsigned int GetPxCol(int rawhit){
        unsigned int PxlACol = GET_PX_ACOL(rawhit);
        unsigned int PxlRCol = GET_PX_RCOL(rawhit);
        unsigned int PxlCol  = PxlACol * 4 + PxlRCol;
        return PxlCol;
    };
    static unsigned int GetPxRow(int rawhit){ return  GET_PX_ROW(rawhit); };
    static unsigned int GetPxIdx(int rawhit){
        return  (GET_PX_ACOL(rawhit)<<7)+
            ((GET_PX_ROW(rawhit)&0x1c)<<2)+(GET_PX_RCOL(rawhit)<<2)
            +(GET_PX_ROW(rawhit)&3);
    };
    static unsigned int GetHitTS(int rawhit){
      unsigned int ts= GET_HIT_TS(rawhit);
      return ts;
    };
    static unsigned int GetParity(int rawhit){
      unsigned int Parity= GET_PARITY(rawhit);
      return Parity;
    };
    static unsigned int GetChipN(int rawhit){
      unsigned int ChipN=GET_CHIPN(rawhit);
      return ChipN;
    };
    static unsigned int GetDacStep(int rawhit){
      unsigned int DacStep=GET_DACSTEP(rawhit);
      return DacStep;
    };
    static unsigned int GetWrapBits(int rawhit){
      unsigned int  WrapBits= GET_COL_WRAP(rawhit);
      return WrapBits;
    }; 
    static int EvalParity(int rawhit){
      int parity=0;
      int myhit=rawhit;
      for (int i=0; i<24 ; i++){
     	if( (unsigned(myhit)&1) == 0){
   	  parity++;
	  //cout<<"+";
	  myhit=myhit>>1;
	}else{ 
	  parity--;
	  //cout<<"-";
	  myhit=myhit>>1;
	}
      }
      //cout<<(hex)<<myhit<<endl;
      return parity;
    }; 
    
};


class FSSR2 {
 public:
    static unsigned int GetFSSRAdc(int rawhit){ return GET_ADC(rawhit);};
    static unsigned int GetFSSRChip(int rawhit){ return GET_CHIP(rawhit);};
    static unsigned int GetFSSRSet(int rawhit){
        static int setVal[32]={256,256,256,256,256,256,256,256,
                               256,256,  0,  1,  4,  5,  3,  2,
                               256,256, 12, 13,  8,  9, 11, 10,
                               256,256, 15, 14,  7,  6,256,256};
        return setVal[GET_SET(rawhit)];};
    static unsigned int GetFSSRStrip(int rawhit){
        static int stripVal[16]={256,256,256,256,256,  0,  2,  1,
                                 256,  6,  4,  5,256,  7,  3,256};
        return stripVal[GET_STRIP(rawhit)];};
    static unsigned int GetFSSRStripID(int rawhit){
        return GetFSSRChip(rawhit)*128+GetFSSRSet(rawhit)*8
            +GetFSSRStrip(rawhit); };
};


//////////////////////////////////////////////////////////////////////////////


class EventManager {
public:
                                        EventManager();
    //                                    EventManager(int numberOfLayers);
    virtual                             ~EventManager();

    virtual void                        reset();

    virtual bool                        newEvent();
    virtual bool                        startEvent(_BLOCK block);
    virtual bool                        startNewEvent(_BLOCK block);
    virtual bool                        hasStarted();
    virtual bool                        hasBeenClosed();
    virtual void                        setCorrupted()                      { m_events[m_currentEvent].status = corrupted; };
    virtual void                        setWarning()                      { m_events[m_currentEvent].warnings++; };
    virtual bool                        endEvent(_BLOCK block);
    virtual _EVENT&                     getEvent(const int index);
    virtual _EVENT&                     getEvent()                          { return getEvent(m_currentEvent); } ;
    virtual bool                        CheckNRobs() { return m_nrobs==m_events[m_currentEvent].ROBs.size();};
    virtual bool                        addROB(_BLOCK blk);
    virtual bool                        isInsideROB();
    virtual _BLOCK                      getROBstartingBlock();
    virtual bool                        endROB(_BLOCK block);
    
    virtual bool                        startROD(_BLOCK block);
    virtual bool                        isInsideROD();
    virtual _BLOCK                      getRODstartingBlock();
    virtual _BLOCK                      getRODendingBlock();

    virtual void                        setRODformatVersion(_CHUNK version);
    virtual void                        setSourceID(_CHUNK vID);
    virtual void                        endROD(_BLOCK block);

    virtual bool                        startTrigger(_BLOCK block);
    virtual _BLOCK                      getTriggerStartingBlock();
    virtual bool                        triggerHasStarted();
    virtual _triggerType                getTriggerType();
    virtual _triggerType                setTriggerType(_CHUNK chunk);
    virtual bool                        endTrigger();

    virtual void			addAMdata(_CHUNK am_raw);
    
    virtual unsigned long               getExpectedLength();
    
    virtual bool                        setPropertiesFromHeader(_CHUNK header);
    virtual void                        setRunNumber(_CHUNK word);
    virtual bool                        setNumberOfTracks(unsigned int ntracks);
    virtual unsigned int                getNumberOfTracks();
    virtual unsigned int                getFlags();
    virtual bool                        checkFlagsConsistency(unsigned int first, unsigned int second);
    virtual bool                        checkFlagsConsistency()             {
                                            return m_currentEvent>0 ?  checkFlagsConsistency(m_currentEvent, m_currentEvent-1): true; };
    virtual bool                        isEnabledAM();
    virtual bool                        isEnabledScint();
    virtual bool                        isMasterEdro();
    
    virtual inline unsigned long        getNumberOfStoredEvents()           { return m_events.size(); };
    virtual unsigned long               getOrdinal();
    virtual _BLOCK                      getStartingBlock(const unsigned int index);
    virtual _BLOCK                      getStartingBlock()                  { return getStartingBlock(m_currentEvent); } ;
    virtual _BLOCK                      getEndingBlock(const unsigned int index);
    virtual _BLOCK                      getEndingBlock()                    { return getEndingBlock(m_currentEvent); } ;
    virtual unsigned int                getTimestamp();
    virtual bool                        checkTimestampConsistency();
    
    virtual bool                        setBX(_BLOCK bx);
    virtual _BLOCK                      getBX(const unsigned int index);
    virtual _BLOCK                      getBX()                            { return getBX(m_currentEvent); };
    virtual bool                        BXhasGrown();
    
    virtual bool                        setBCO(_BLOCK bco);
    virtual _BLOCK                      getBCO(const unsigned int index);
    virtual _BLOCK                      getBCO()                            { return getBCO(m_currentEvent); };
    virtual bool                        BCOhasGrown();
    
    virtual bool                        setLvl1ID(_CHUNK lvl1id);
    virtual _BLOCK                      getLvl1ID(const unsigned int index);
    virtual _BLOCK                      getLvl1ID()                            { return getLvl1ID(m_currentEvent); };
    virtual bool                        Lvl1IDhasGrown();
    
    virtual _LAYER&                     getLayer(const unsigned int index);
    virtual vector<int>                 setHits(const unsigned int hit_counter, _BLOCK block);
    virtual unsigned int                getTotHits();

    virtual void                        addScintFlag(_CHUNK flag_raw);    

    virtual void                        dump(ostream& out) {return dump(m_currentEvent, out); };
    virtual void                        dump(const int index, ostream& out);
    virtual void                        dumpAllEvents(ostream& out);

    virtual void                        addError(_ERROR& err);
    virtual void                        addError(char* description,  _BLOCK position, _CHUNK raw_block);
    virtual void                        addError(KIND_OF_ERROR kind, _BLOCK position, _CHUNK raw_block);
    virtual void                        dumpErrorDB(ostream& out);
    virtual void			resetErrorDB();
protected:
    //variables
    eventBuffer                         m_events;
    int                                 m_currentEvent; // <= MAX
    unsigned long                       m_totevents;    // virtually unbounded, practically can VERY large
    unsigned int                        m_nrobs;
    
    //error managing
    virtual void                        initErrorDB();
    errorDB                             m_errorDB;

    //functions
    virtual void                        resetEvent(_EVENT& event);
    
    //Error handling
    

    class errIndex : public exception
    {
        virtual const char* what() const throw()
        {
            return "@ Invalid event index!";
        }
    } wrongIndex;
    
    class errStart : public exception
    {
        virtual const char* what() const throw()
        {
            return "@ Event not yet started!";
        }
    } eventNotStarted;
    
    class errAlreadyStarted : public exception
    {
        virtual const char* what() const throw()
    {
            return "@ Event already started!";
    }
    } eventAlreadyStarted;
    
    class errROBStart : public exception
    {
        virtual const char* what() const throw()
        {
            return "@ ROB not yet started!";
        }
    } robNotStarted;
    
    class errRODStart : public exception
    {
        virtual const char* what() const throw()
    {
            return "@ ROD not yet started!";
    }
    } rodNotStarted;
    
    
};

#endif /* _EVENTMANAGER_H */

/*
 *  EventManager.cpp
 *  
 *
 *  Created by Riccardo Di Sipio on 06/02/08.
 *
 */

#include "EventManager.h"

EventManager::EventManager()
{
    m_currentEvent = STARTING_INDEX;
    m_totevents = 0;
    m_nrobs = 0;

    initErrorDB();
}


EventManager::~EventManager()
{
}


//////////////////////////////////////////////////


void EventManager::reset()
{
  m_events.clear();

  m_currentEvent = STARTING_INDEX;;
  m_totevents    = 0;
  m_nrobs        = 0;

  resetErrorDB(); 
}


//////////////////////////////////////////////////


void EventManager::resetEvent(_EVENT& event)
{
    event.run_number                = 0;
    event.status                    = empty;
    event.warnings                  = 0;
    event.expected_length           = 0;
    event.ordinal                   = 0;
    event.starting_block            = 0;
    event.ending_block              = 0;
}


//////////////////////////////////////////////////


bool EventManager::newEvent()
{
    bool succeded = true;
    _EVENT event;
    
    resetEvent(event);
    m_events.push_back(event);
    if( m_events.size() > MAXBUFFERSIZE )  {
      m_events.pop_front();
      m_currentEvent =  m_events.size() - 1;
    }
    else m_currentEvent++;
        
    m_totevents++;
    return succeded;
}


//////////////////////////////////////////////////


_EVENT& EventManager::getEvent(const int index)
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    return m_events[index];
}


//////////////////////////////////////////////////


bool EventManager::startEvent(_BLOCK block)
{
    //if(!currentEventIsValid() ) return false;
#ifdef _DEBUG
    ASSERT_INDEX()
#endif

    bool succeded = true;
    
    if( m_events[m_currentEvent].status != started) 
    {
            m_events[m_currentEvent].status = started;
            m_events[m_currentEvent].starting_block = block;
    }
    else
    {
            succeded = false;
    }
    return succeded;
}


//////////////////////////////////////////////////


bool EventManager::startNewEvent(_BLOCK block)         
{ 
    if( hasStarted() ) throw eventAlreadyStarted;
    newEvent(); 
    return startEvent(block); 
}


//////////////////////////////////////////////////


bool EventManager::endEvent(_BLOCK block)
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_EVENT_HAS_STARTED()
#endif
    
    _EVENT* p_evt = &m_events[m_currentEvent];

    p_evt->ending_block = block;
    p_evt->length = p_evt->ending_block - p_evt->starting_block ; 
    if( m_nrobs==0 ) m_nrobs = p_evt->ROBs.size();
    if(p_evt->status != corrupted  ) p_evt->status = complete;
    return true;
}


//////////////////////////////////////////////////


bool EventManager::hasStarted()
{
    if( m_currentEvent < 0 ) return false;

#ifdef _DEBUG   
    ASSERT_INDEX()
#endif
    
    bool status = true;
    if(m_events[m_currentEvent].status != started ) status = false;
    
    return status;
}


//////////////////////////////////////////////////


bool EventManager::hasBeenClosed()
{
    if( m_currentEvent < 0 ) return false;

#ifdef _DEBUG    
    ASSERT_INDEX()
#endif
        
    bool status = true;
    if(m_events[m_currentEvent].status != complete) status = false;
    
    return status;
}


//////////////////////////////////////////////////


_BLOCK EventManager::getStartingBlock(const unsigned int index)
{
#ifdef _DEBUG
    ASSERT_INDEX_EXISTS(index)
    //ASSERT_EVENT_HAS_STARTED()
#endif

    return m_events[index].starting_block;
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////
// ROB Stuff

bool EventManager::addROB( _BLOCK blk)
{
    _ROB rob; 
    rob.status                      = started;

    rob.starting_block              = blk;
    rob.ending_block                = 0;
    
    _ROD rod;
    rod.trigger.type                = NONE;  
    rod.trigger.status              = empty;
    rod.trigger.starting_block      = (_BLOCK)0;
    rod.trigger.ending_block        = (_BLOCK)0;
    
    rod.BX                          = (_BLOCK)0;
    rod.BCO                         = (_BLOCK)0;
    rod.lvl1id                      = (_BLOCK)0; 
    rod.source_identifier           = MASTER;
    rod.flags                       = 0;        
    rod.N_AM                        = 0;
    rod.timestamp                   = 0;
    
    rod.layers.clear();
    for(unsigned int l = 0; l <= LAYERS; l++)
    {
        _LAYER emptyLayer;
	emptyLayer.number_of_hits      = 0;
	emptyLayer.number_of_words     = 0;

        rod.layers.push_back(emptyLayer);
    }
    rob.rod = rod;

    m_events[m_currentEvent].ROBs.push_back( rob );
    
    return true;
}

//////////////////////////////////////////////////


bool EventManager::isInsideROB()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    bool res = false;
    
    if( m_events[m_currentEvent].ROBs.size()>0 &&
	m_events[m_currentEvent].ROBs.back().status == started ) res = true;
    
    return res;
}


//////////////////////////////////////////////////


bool EventManager::endROB(_BLOCK block)
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif

    _ROB * p_rob = &m_events[m_currentEvent].ROBs.back();
    if(isInsideROB() )  {
      p_rob->status       = complete;
      p_rob->ending_block = block;
    }
    return true;  // give back something
}


//////////////////////////////////////////////////


_BLOCK EventManager::getROBstartingBlock()
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_ROB_HAS_STARTED()
#endif 
   
    return m_events[m_currentEvent].ROBs.back().starting_block;
}

//////////////////////////////////////////////////



bool EventManager::startROD(_BLOCK block)
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_ROB_HAS_STARTED()
#endif

    _ROD rod;
    rod.starting_block = block;
    rod.status         = started;
    rod.BX             = 0;
    rod.BCO            = 0;
    rod.lvl1id         = 0;
    rod.timestamp      = 0;
    rod.N_AM           = 0;
    rod.trigger.type   = NONE;
    rod.trigger.status = empty;


    //reset layer informations
    for(unsigned int lay = 0; lay < LAYERS; lay++) 
    {
        _LAYER layer;
        layer.hit_block_position = 0;
        layer.number_of_words    = 0;
        
        rod.layers.push_back(layer);
    }
    
    m_events[m_currentEvent].ROBs.back().rod = rod;
    
    return true;
}


//////////////////////////////////////////////////


bool EventManager::isInsideROD()
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_ROB_HAS_STARTED()
#endif

    bool res = false;
    
    if( m_events[m_currentEvent].ROBs.back().rod.status == started ) res = true;
    
    return res;
}


//////////////////////////////////////////////////


_BLOCK EventManager::getRODstartingBlock()
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_ROB_HAS_STARTED()
    ASSERT_ROD_HAS_STARTED()
#endif

    return m_events[m_currentEvent].ROBs.back().rod.starting_block;
}


//////////////////////////////////////////////////


_BLOCK EventManager::getRODendingBlock()
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_ROB_HAS_STARTED()
#endif
    
    return m_events[m_currentEvent].ROBs.back().rod.ending_block;
}


//////////////////////////////////////////////////


void EventManager::setRODformatVersion(_CHUNK version)
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_ROB_HAS_STARTED()
    ASSERT_ROD_HAS_STARTED()
#endif
    
    m_events[m_currentEvent].ROBs.back().rod.format_version = version;
}


//////////////////////////////////////////////////


void EventManager::endROD(_BLOCK block)
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_ROB_HAS_STARTED()
    ASSERT_ROD_HAS_STARTED()
#endif
    
    _EVENT * p_evt = &m_events[m_currentEvent];
    _ROD * p_rod   = &p_evt->ROBs.back().rod;

    p_rod->ending_block = block;
    p_rod->status = complete;
    p_rod->trigger.status = complete;
    
    //now we can define the expected length of the event
    m_events[m_currentEvent].expected_length = 8 +  p_rod->tot_hits()  
        +  p_evt->AM_enabled*(p_rod->N_AM)  
        +  p_evt->scint_flag * 4 //should be 0 or 4
        +  1 ;
    
}


//////////////////////////////////////////////////


void EventManager::setSourceID(_CHUNK vID)
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_ROB_HAS_STARTED()
    ASSERT_ROD_HAS_STARTED()
#endif
    
    // MASTER_ID = 0xeda03001
    if(vID == MASTER_ID) 
        m_events[m_currentEvent].ROBs.back().rod.source_identifier = MASTER;
    else
        m_events[m_currentEvent].ROBs.back().rod.source_identifier = SLAVE;

}



//////////////////////////////////////////////////


_BLOCK EventManager::getEndingBlock(const unsigned int index)
{
#ifdef _DEBUG
    ASSERT_INDEX_EXISTS(index)
    ASSERT_EVENT_HAS_STARTED()
#endif
    
    return m_events[index].ending_block;
}


//////////////////////////////////////////////////


unsigned int EventManager::getTimestamp()
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_EVENT_HAS_STARTED()
#endif
    
    return m_events[m_currentEvent].timestamp;
}


//////////////////////////////////////////////////


bool EventManager::checkTimestampConsistency()
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_EVENT_HAS_STARTED()
    ASSERT_ROB_HAS_STARTED()
    ASSERT_ROD_HAS_STARTED()
#endif

    bool successful = true;
    
    int nRobs = m_events[m_currentEvent].ROBs.size();
    unsigned int ts1, ts2 = 0;
    
    if( nRobs > 1) {
        try {
        ts1 = m_events[m_currentEvent].ROBs.front().rod.timestamp;
        ts2 = m_events[m_currentEvent].ROBs.back().rod.timestamp;
        } 
        catch(std::bad_alloc& e) {
            printf("\nerror");
        }
        if( ts1 != ts2 ) successful = false;
    }
    
    return successful;
}


//////////////////////////////////////////////////


bool EventManager::setPropertiesFromHeader(_CHUNK header)
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_EVENT_HAS_STARTED()
#endif
    
    _EVENT* event = &m_events[m_currentEvent];
    //_ROD * p_rod = &m_events[m_currentEvent].ROBs.back().rod;
    
    m_events[m_currentEvent].ordinal     = GET_EVT_COUNTER(header);
    //p_rod->flags       = GET_EVT_FLAGS(header);        //ATTENZIONE
    //p_rod->master_edro = GET_MASTER_EDRO(header); //changed
    event->scint_flag  = GET_SCINT_FLAG(header);
    event->timestamp   = GET_TIMESTAMP(header);
    event->AM_enabled  = GET_AM_ENABLED(header);
    
    return true;
}


//////////////////////////////////////////////////


_triggerType EventManager::setTriggerType(_CHUNK chunk)
{
//    bool res = true;
    _ROD * p_rod = &m_events[m_currentEvent].ROBs.back().rod;
    
    //set timestamp
    m_events[m_currentEvent].timestamp = GET_TIMESTAMP(chunk);
    p_rod->timestamp = GET_TIMESTAMP(chunk);
    p_rod->trigger.type = NONE;
    //which trigger?
    if( TRIG_IS_EXT(chunk) )                           p_rod->trigger.type = EXT;
    if( TRIG_IS_AM(chunk)  )                           p_rod->trigger.type = AM;
    if( TRIG_IS_INT(chunk) )                           p_rod->trigger.type = INT;
    if( TRIG_IS_AM(chunk) && TRIG_IS_INT(chunk) )      p_rod->trigger.type = AM_INT;
    
    return p_rod->trigger.type;
}

//////////////////////////////////////////////////


void EventManager::setRunNumber(_CHUNK word)
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif   
 
    m_events[m_currentEvent].run_number = word;
}


//////////////////////////////////////////////////


bool EventManager::startTrigger(_BLOCK block)
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    m_events[m_currentEvent].ROBs.back().rod.trigger.status = started;
    m_events[m_currentEvent].ROBs.back().rod.trigger.starting_block = block;
    return true;
}

//////////////////////////////////////////////////


bool EventManager::triggerHasStarted()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    bool status = false;
    if( m_events[m_currentEvent].ROBs.back().rod.trigger.status == started) status = true;
    
    return status;
}


//////////////////////////////////////////////////


_triggerType EventManager::getTriggerType()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    return m_events[m_currentEvent].ROBs.back().rod.trigger.type;
}


//////////////////////////////////////////////////


_BLOCK EventManager::getTriggerStartingBlock()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    return m_events[m_currentEvent].ROBs.back().rod.trigger.starting_block;
}

//////////////////////////////////////////////////


bool EventManager::endTrigger()
{
#ifdef _DEBUG
     ASSERT_INDEX()
     ASSERT_EVENT_HAS_STARTED()
#endif
    
     m_events[m_currentEvent].ROBs.back().rod.trigger.status = end_trigger;
     
     return true;
}


//////////////////////////////////////////////////


bool EventManager::setBX(_BLOCK bx)
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_ROB_HAS_STARTED()
    ASSERT_ROD_HAS_STARTED()
#endif
    
    m_events[m_currentEvent].ROBs.back().rod.BX = bx;
    
    return true;
}



_BLOCK EventManager::getBX(unsigned int index)
{
#ifdef _DEBUG
    ASSERT_INDEX_EXISTS(index)
#endif
    
    return m_events[index].ROBs.back().rod.BX;
}




bool EventManager::BXhasGrown()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    if(m_currentEvent <= 0) return true;
#ifdef _DEBUG
    ASSERT_INDEX_EXISTS((unsigned int)(m_currentEvent-1))
#endif

    unsigned idx = m_events[m_currentEvent].ROBs.size()-1; // index of the last ROB
    
    return ( m_events[m_currentEvent-1].ROBs.size()>idx && m_events[m_currentEvent].ROBs[idx].rod.BX >= m_events[m_currentEvent-1].ROBs[idx].rod.BX);
}


//////////////////////////////////////////////////


bool EventManager::setBCO(_BLOCK bco)
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_ROB_HAS_STARTED()
    ASSERT_ROD_HAS_STARTED()
#endif

    m_events[m_currentEvent].ROBs.back().rod.BCO = bco;
    
    return true;
}



_BLOCK EventManager::getBCO(unsigned int index)
{
#ifdef _DEBUG
    ASSERT_INDEX_EXISTS(index)
#endif
        
    return m_events[index].ROBs.back().rod.BCO;
}




bool EventManager::BCOhasGrown()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif

    if(m_currentEvent <= 0) return true;
#ifdef _DEBUG
    ASSERT_INDEX_EXISTS((unsigned int)(m_currentEvent-1))
#endif
    unsigned int i = m_events[m_currentEvent].ROBs.size();
    return ( m_events[m_currentEvent].ROBs[i-1].rod.BCO >= m_events[m_currentEvent-1].ROBs[i-1].rod.BCO);
}


//////////////////////////////////////////////////



_BLOCK EventManager::getLvl1ID(unsigned int index)
{
#ifdef _DEBUG
    ASSERT_INDEX_EXISTS(index);  
#endif
    
    return m_events[index].ROBs.back().rod.lvl1id;
}



bool EventManager::Lvl1IDhasGrown()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif

    if(m_currentEvent <= 0) return true;
    //ASSERT_INDEX_EXISTS(m_currentEvent-1)
    
    return ( m_events[m_currentEvent].ROBs.back().rod.lvl1id -1 == m_events[m_currentEvent-1].ROBs.back().rod.lvl1id);
}



bool EventManager::setLvl1ID(_CHUNK lvl1id)
{
#ifdef _DEBUG
    ASSERT_INDEX()
    ASSERT_ROB_HAS_STARTED()
    ASSERT_ROD_HAS_STARTED()
#endif
    
    m_events[m_currentEvent].ROBs.back().rod.lvl1id = lvl1id;
    
    
    return true;
}


//////////////////////////////////////////////////


unsigned long EventManager::getExpectedLength()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    return m_events[m_currentEvent].expected_length;
}


//////////////////////////////////////////////////


unsigned long EventManager::getOrdinal()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    return m_events[m_currentEvent].ordinal;
}


//////////////////////////////////////////////////


unsigned int  EventManager::getFlags()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    return m_events[m_currentEvent].ROBs.back().rod.flags;
}


//////////////////////////////////////////////////


bool EventManager::checkFlagsConsistency(unsigned int first, unsigned int second)
{
#ifdef _DEBUG
    ASSERT_INDEX_EXISTS(first)
//    if(second == -1) return true;
    ASSERT_INDEX_EXISTS(second)
#endif
    
    return ( m_events[first].ROBs.back().rod.flags == m_events[second].ROBs.back().rod.flags );
}

//////////////////////////////////////////////////


bool EventManager::isMasterEdro()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    if(m_events[m_currentEvent].ROBs.back().rod.source_identifier == MASTER) return true;
    else return false;
}


//////////////////////////////////////////////////


bool EventManager::isEnabledAM()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif

    return m_events[m_currentEvent].AM_enabled;
}


//////////////////////////////////////////////////


void EventManager::addAMdata(_CHUNK am_raw)
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
     _ROD* p_rod = &m_events[m_currentEvent].ROBs.back().rod;

    _AM_DATA am;
    am.raw = am_raw;

    p_rod->AM_data.push_back( am );

    return;
}
//////////////////////////////////////////////////


bool EventManager::isEnabledScint()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    return m_events[m_currentEvent].scint_flag;
}


//////////////////////////////////////////////////



bool EventManager::setNumberOfTracks(unsigned int  ntracks)
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    m_events[m_currentEvent].ROBs.back().rod.N_AM = ntracks;
    
    return true;
}


//////////////////////////////////////////////////


unsigned int EventManager::getNumberOfTracks()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    return m_events[m_currentEvent].ROBs.back().rod.N_AM;
}


//////////////////////////////////////////////////


_LAYER& EventManager::getLayer(unsigned int index)
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif

    if(index <= m_events[m_currentEvent].ROBs.back().rod.layers.size() )
        return m_events[m_currentEvent].ROBs.back().rod.layers[index];
    else 
    {
        throw wrongIndex;
    }
}


//////////////////////////////////////////////////


vector<int> EventManager::setHits(unsigned int hit_counter, _BLOCK block)
{
    // hit_counter should be 1 or 2
    if( hit_counter < 1 || hit_counter > 2 ) throw wrongIndex;

#ifdef _DEBUG
    ASSERT_INDEX()
#endif

    vector<int> hits4layer;
//??    _EVENT* event = &m_events[m_currentEvent];
    
    //if( hit_counter == 1 ) event->ROBs.back().rod.tot_hits = 8; //?

    _BLOCK mask = 0x0;
    int hc_shift = 4 * (hit_counter - 1); //there are 8 layers. hitcounter 1 -> layers 0-3; hitcounter 2 -> layers 4-7
        
    for(unsigned int l = 0; l < 4; l++){
        mask = 0xff<<(8 * l);
	int nhits = (block & mask) >> (8*l);
	//event->ROBs.back().rod.layers[hc_shift + l].number_of_hits = nhits; 
	getLayer(hc_shift + l).number_of_hits = nhits;
	hits4layer.push_back( 1 + nhits ); //at least 1 hitword per layer

        //event->ROBs.back().rod.tot_hits += event->ROBs.back().rod.layers[hc_shift + l].number_of_hits;
	//event->ROBs.back().rod.tot_hits += getLayer(hc_shift + l).number_of_hits;
    }
    //hits4layer.push_back( event->ROBs.back().rod.tot_hits() );
    
    return hits4layer;
}


//////////////////////////////////////////////////


unsigned int EventManager::getTotHits()
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    return m_events[m_currentEvent].ROBs.back().rod.tot_hits();
}


//////////////////////////////////////////////////


void EventManager::addScintFlag(_CHUNK flag_raw)
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
     _EVENT* p_evt = &m_events[m_currentEvent];

    _SCINTFLAG sf;
    sf.raw       = flag_raw;
    sf.timestamp = GET_SCINT_TIMESTAMP( flag_raw );

    p_evt->scint_flags.push_back( sf );

    return;
}


//////////////////////////////////////////////////


void EventManager::dump(const int index, ostream& out)
{
#ifdef _DEBUG
    ASSERT_INDEX()
#endif
    
    _ROD dummy;
    dummy.lvl1id         = 666;
    dummy.BCO            = 666;
    dummy.BX             = 666;
    dummy.timestamp      = 0;
    dummy.trigger.type   = NONE;
    dummy.trigger.status = corrupted;

    _EVENT* evt = &m_events[index];
    _ROD *  myrod;
    if( evt->ROBs.size()>0 ) myrod = &evt->ROBs.front().rod;
    else myrod = &dummy;


    out << "Dump of event "     << myrod->lvl1id                                                        << endl
        << "\tBlocks "          << evt->starting_block << " to " << evt->ending_block                   << endl
        << "\tStatus: "         << "(" << evt->status << ") " << StatusString[evt->status]              << endl
        << "\tWarnings: "       << evt->warnings                                                        << endl
        << "\tTrigger: type: "  << TriggerString[myrod->trigger.type]        
        << ", status: "         << StatusString[myrod->trigger.status]                                  << endl
        << "\tTimestamp: "      << evt->timestamp <<"\t";
    char c='(';
    for( unsigned int ii=0; ii<evt->ROBs.size(); ++ii){
      cout<<c<<evt->ROBs[ii].rod.timestamp;
      c=',';
    }
    cout<<") "<<endl                                                       << endl
        << "\tBCO: "            << myrod->BCO 
        << ", Lvl1ID: "         << myrod->lvl1id  
        << ", BX: "             << myrod->BX 
        << endl;
    out << "\tROBs found: " << evt->ROBs.size() << endl;

    for(unsigned int rob = 0; rob < evt->ROBs.size(); rob++) {
      out << "\t\tNumber of hits (ROB " << rob << "): " << evt->ROBs[rob].rod.tot_hits()            
	    << " (N_AM: "   << evt->ROBs[rob].rod.N_AM << ")" << endl;
    }
    cout << endl << SEPARATOR << endl << endl;
}

void EventManager::dumpAllEvents(ostream& out)
{
    for(unsigned int i = 0; i < m_events.size(); i++)
    {
        dump(i, out);
	out << endl;
    }
}


//////////////////////////////////////////////////

/*
  Remainder: EventManagers stores no more than a certain number of events, 
  but stats covers all the processed events.
*/

void EventManager::initErrorDB()
{
  //m_errorDB.reserve(kNUMBER_OF_KINDS);
  //printf("Error DB: @ %d errors known\n", kNUMBER_OF_KINDS);
  for(int e = 0; e < kNUMBER_OF_KINDS; e++)
  {
    _ERROR err;

    err.id          = (KIND_OF_ERROR)e;
    err.description = errorStrings[e].c_str();
    err.position    = 0;
    err.raw_block   = 0;
    err.counts      = 0;
    m_errorDB.push_back(err);
  }
}

void EventManager::resetErrorDB()
{
  m_totevents = 0;
  for (unsigned int i = 0; i < m_errorDB.size(); i++)
    {
       m_errorDB.at(i).counts = 0;
    }
}


void EventManager::addError(char* description, _BLOCK position = 0, _CHUNK raw_block = 0)
{
  _ERROR err;
  err.id          = (KIND_OF_ERROR)0;
  err.description = description;
  err.position    = position;
  err.raw_block   = raw_block;

  addError(err);
  printf("Generic error added\n");
}

void EventManager::addError(KIND_OF_ERROR kind, _BLOCK position = 0, _CHUNK raw_block = 0)
{
  _ERROR err;
  err.id          = kind;
  err.description = errorStrings[kind].c_str();
  err.position    = position;
  err.raw_block   = raw_block;

  addError(err);
}

void EventManager::addError(_ERROR& err)
{
#ifdef _DEBUG
    ASSERT_INDEX() 
#endif

    //  printf("@ Adding error %s\n", errorStrings[err.id].c_str() );
    if( err.id < 0 || err.id >= kNUMBER_OF_KINDS  )
      {
	throw std::out_of_range(" @ Error id out of range");
      }

    //    setCorrupted();
    m_events[m_currentEvent].errors_found.push_back(err);

    _ERROR* p_e = &m_errorDB.at(err.id); //update counter
    if(!p_e) throw out_of_range(" @ NULL pointer");
    else p_e->counts++;
}


void EventManager::dumpErrorDB(ostream& out)
{
  if(m_totevents == 0) return;

  //out << "Errors found:" << endl;
  unsigned long n_errs = 0;
  

  if( m_errorDB.size() > 0 ) 
  {
    // Calculate total N of errors
    for(unsigned int eid = 0; eid < kNUMBER_OF_KINDS; eid++)
    {
      n_errs += m_errorDB[eid].counts;  //total number
    }

    //double error_freq = (double)n_errs / (double)m_totevents;

    for(unsigned int eid = 0; eid < kNUMBER_OF_KINDS; eid++)
    {
      out.width(80);
      out.precision(3);
      unsigned long errfound = m_errorDB[eid].counts;

      //calculate %
      double perc_err = 0;
      //double perc_evt = 0;

      if(n_errs != 0) {
	perc_err = 100.0 * errfound / n_errs;
	//perc_evt = 100.0 * errfound / m_totevents;
      }

      out << errorStrings[eid] << "(code " << eid << ")" << ": " << errfound << "\t(" << perc_err  << "\% of errors)" << endl;
	//<< ",\toccurred in "  << perc_evt << "% of evts )" << endl;
    }
    out << "Total errors: " << n_errs << " in " << m_totevents <<" events" << endl;
  }
}

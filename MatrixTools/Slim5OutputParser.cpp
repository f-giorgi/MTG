/*
 *  Slim5OutputParser.cpp
 *  control
 *
 *  Created by Riccardo Di Sipio on 13/02/08.
 *
 */



/*
 Event ->    ROB -> ROD
 ROB -> ROD
 
 .
 .
 .
 
 */

#include "Slim5OutputParser.h"


#ifndef _BIT_FUNCTIONS_

char *getBufferAsBinaryString(const unsigned long input)
{
	int pos=0;
	char result;
	static char bitstring[256];
	memset(bitstring, 0, 256);
	for(int i=31;i>=0;i--)
	{
		if (((input >> i) & 1)) result = '1';
		else result = '0';
		bitstring[pos] = result;
		if ((i>0) && ((i)%4)==0)
		{
			pos++;
			bitstring[pos] = ' ';
		}
		pos++;
	}
	return bitstring;
}

#endif /* _BIT_FUNCTIONS_ */


Slim5OutputParser::Slim5OutputParser()
{
  initialize();
}


Slim5OutputParser::Slim5OutputParser(ifstream& file)
{
  attachFile(file);
  initialize();  
}


bool Slim5OutputParser::initialize()
{
    m_currentBlock                      = 0;
    m_filePosition                      = 0;
    m_verbosity                         = kSILENT;
    m_lastHitBlockLine                  = -1;
    m_reached_end_of_file               = false;

    m_hist_errors = new TH1F("errors_found", "Errors found", kNUMBER_OF_KINDS, -0.5, kNUMBER_OF_KINDS-0.5);
  
    return true;
}


Slim5OutputParser::~Slim5OutputParser()
{
  delete m_hist_errors;
}


//////////////////////////////////////////////////



bool Slim5OutputParser::attachFile(ifstream& file)
{
  m_file = &file; 
  m_event.reset(); 
 
  return fileIsAttached();
}


//////////////////////////////////////////////////


void Slim5OutputParser::setVerbosity(_VERBOSITY level)
{
    m_verbosity = level;
}


//////////////////////////////////////////////////

void Slim5OutputParser::setStartingBlock(_BLOCK start_from_block)
{
  m_currentBlock = start_from_block;
}


//////////////////////////////////////////////////


void Slim5OutputParser::dumpErrors(ostream& out)
{
  out << "Error Stats:" << endl;
  m_event.dumpErrorDB(out);
  out << endl;
}


//////////////////////////////////////////////////


bool Slim5OutputParser::FindEvent()
{
  if(!fileIsAttached()) return false;

    bool eventFound = false;
    bool reached_end_of_event = false;
    bool next_is_checksum     = false;
    //First of all, check if current event is corrupted
    /*if(m_event.getNumberOfStoredEvents() > 0 && !m_event.hasBeenClosed() ) 
    {
      m_event.addError("Event not closed.", 0, 0);
      }*/
    char storage[50];
    char *hex_chunk;
    //char blockN[128];
    _BLOCK current_chunk = 0;
    int jump = 5;
    
    while(!reached_end_of_event)
    {
        hex_chunk = storage;
        hex_chunk[0] = 0;
        _CHUNK checksum = 0;
        m_reached_end_of_file = true;
        try{
            //shift get pointer and read the block     
            m_file->seekg( m_currentBlock*WORD_SIZE );
	    m_file->read(reinterpret_cast < char * > (&current_chunk), sizeof(current_chunk) );
        }
        catch(std::exception& e){
            //end of file? file unreadable?
            //            cout << e.what() << endl;
            reached_end_of_event = true;
            m_reached_end_of_file = true;
            continue;
        }
        m_reached_end_of_file = false;   
        // convert read buffer into a string
        //const unsigned int current_chunk = strtoul(buf, NULL, 16);     
        snprintf(hex_chunk,10, "%08lx", current_chunk) ;
        
        IF_DEBUG cout << m_currentBlock << ") " << getBufferAsBinaryString(current_chunk) << "\t" << hex_chunk << "\t"; //" = " << current_chunk << endl;
        
	if( START_EVT(current_chunk) ) 
	{
	  if( !m_event.hasStarted()  )
	    {
	      IF_DEBUG cout << "Event header\t";
	      m_event.startNewEvent(m_currentBlock);
	      /* Is it really useful now?
	      if(!m_event.startNewEvent(m_currentBlock) ) 
		{
		  m_event.addError(kIMPOSSIBLE_NEW_EVENT, m_currentBlock, current_chunk);
		  throw impossibleNewEvent;
		  }
	      */
	    } // evt has already started
	  else if( m_event.isInsideROB() )
	    {
	      // bad marker? event not closed?
	      m_event.addError(kIMPOSSIBLE_NEW_EVENT, m_currentBlock, current_chunk);
	      m_hist_errors->Fill(kIMPOSSIBLE_NEW_EVENT); 
	      m_currentBlock++;
	      throw impossibleNewEvent;
	    }
	}// endif: start event marker found
        else 
	  {
	  if( m_event.hasStarted() )
	    {
	      IF_DEBUG cout << "+ ";
	      if( m_currentBlock == m_event.getStartingBlock() + EVT_HEADER_SIZE )
		{
		  IF_DEBUG cout << "Run Number\t" << current_chunk;
		  m_event.setRunNumber(current_chunk);
		}
            
            
            //////////////////////////////////////////////////
            
            
            /*if( m_currentBlock == m_event.getStartingBlock() + EVT_HEADER_SIZE+2 ) 
            {
                //IF_DEBUG cout << "Reading header properties..";
                //m_event.setPropertiesFromHeader(current_chunk);
                // if(!m_event.checkFlagsConsistency()) throw wrongFlags;   //ATTENZIONE
            }*/
            
            
            //////////////////////////////////////////////////
            
            //ROB stuff
            
            if( ROB_HEADER(current_chunk) && !next_is_checksum )
            {
                IF_DEBUG cout << "ROB header\t";
                m_event.addROB(m_currentBlock);
                
                /* skip ROB header */
                //m_filePosition += ROB_HEADER_SIZE * WORD_SIZE;
                //m_currentBlock += ROB_HEADER_SIZE - 1; 
            }
            
            //////////////////////////////////////////////////
            
            else if( m_event.isInsideROB() && m_currentBlock != m_event.getROBstartingBlock() )
            {
                IF_DEBUG cout << "* ";
                //ROD header
                if( ROD_HEADER(current_chunk) && !next_is_checksum )
                {
                    IF_DEBUG cout << "ROD header\t";
                    m_event.startROD(m_currentBlock);
                }
                
                else if( m_event.isInsideROD() && m_currentBlock > m_event.getRODstartingBlock() )
                {
                    checksum = 0;
                    
                    // Format Version
                    if(m_currentBlock == m_event.getRODstartingBlock() + 2)
                    {
                        IF_DEBUG cout << "Format version\t" << current_chunk;
                        m_event.setRODformatVersion(current_chunk);
                    }
                    
                    // Source ID
                    else if(m_currentBlock == m_event.getRODstartingBlock() + 3 + jump)
		      {
                        // MASTER_ID = 0xeda03001 //SLAVE_ID = 0xeda03000
			{
			  if (current_chunk == MASTER_ID)  {
			    IF_DEBUG cout << "Source ID: Master";
			  }else if(current_chunk == SLAVE_ID) {
			    IF_DEBUG cout << "Source ID: Slave";
			  } else{
			    IF_DEBUG cout <<" Bad source ID ";
			    m_currentBlock++;
			    throw wrongSourceID;
			  }
			}

			m_event.setSourceID(current_chunk);
		      }
                    
                    //////////////////////////////////////////////////
                    
                    // BX counter       ( when event is on s-link)
                    else if(m_currentBlock == m_event.getRODstartingBlock() + 4+ jump)
                    {  
                        IF_DEBUG cout << "BX: " << current_chunk;
                        m_event.setBX(current_chunk);
                        if( !m_event.BXhasGrown() ) {
			    m_event.addError(kBX, m_currentBlock, current_chunk );
			    m_hist_errors->Fill(kBX); 
			    //			    m_currentBlock++;
  	 		    IF_DEBUG cout << "   @w " <<errorStrings[kBX];
			    m_event.setWarning();

			    //                            throw wrongBX;
                        }
                    }
                    
                    // Lvl1ID counter     (event sequential counter) ***** Level 1 ID
                    else if(m_currentBlock == m_event.getRODstartingBlock() + 5+ jump) 
                    {  
                        IF_DEBUG cout << "Lvl1ID: " << current_chunk;
                        m_event.setLvl1ID(current_chunk);
                        if( !m_event.Lvl1IDhasGrown() ) {
                            m_event.addError(kLVL1ID,  m_currentBlock, current_chunk );
			    m_hist_errors->Fill(kLVL1ID);
			    //			    m_currentBlock++;
			    //                            throw wrongLvl1ID;
  	 		    IF_DEBUG cout << "   @w " <<errorStrings[kLVL1ID];
			    m_event.setWarning();

                        }
                    }                
                    
                    // BCO counter      ( when trigger info has arrived )
                    else if(m_currentBlock == m_event.getRODstartingBlock() + 6+ jump)
                    {  
                        IF_DEBUG cout << "BCO: " << current_chunk;
                        m_event.setBCO(current_chunk);
                        if( !m_event.BCOhasGrown() ) {
                            m_event.addError(kBCO,  m_currentBlock, current_chunk );
			    m_hist_errors->Fill(kBCO);
			    //			    m_currentBlock++;
			    //                            throw wrongBCO;
  	 		    IF_DEBUG cout << "   @w " <<errorStrings[kBCO];
			    m_event.setWarning();

                        }
                    }
                    
                    // trigger type
                    else if(m_currentBlock == m_event.getRODstartingBlock() + 7+ jump)
                    {
                        int tt = m_event.setTriggerType( current_chunk );

                        IF_DEBUG cout << "Trigger type: " << TriggerString[tt];
			IF_DEBUG cout << "\tTS: " << m_event.getTimestamp();

                        //Check if ROD N-1 has the same timestamp
                        if(!m_event.checkTimestampConsistency() ) {
                            m_event.addError(kTIMESTAMPS_NON_CONSISTENT, m_currentBlock, current_chunk  );
			    m_hist_errors->Fill(kTIMESTAMPS_NON_CONSISTENT);
  	 		    IF_DEBUG cout << "   @w " <<errorStrings[kTIMESTAMPS_NON_CONSISTENT];
			    m_event.setWarning();
			    //                            throw wrongTimestampNonConsistent;
                        }                                                
                    }
                    
                    // event type
                    else if(m_currentBlock == m_event.getRODstartingBlock() + 8+ jump)
                    {
                        IF_DEBUG cout << "Event type: " << current_chunk;
                    }
                    
                    //////////////////////////////////////////////////
                    // DATA
                    
                    // Hit counter1
                    else if(m_currentBlock == m_event.getRODstartingBlock() + 9+ jump) 
                    {
                        IF_DEBUG cout << "N Hit Words 0-3:\t(";
                        vector<int> hitsfound = m_event.setHits(1, current_chunk);
                        IF_DEBUG { 
                            for(int l = 0; l < 4; l++){
                                cout << hitsfound[l] << ", ";
                            }
                            cout << ") "; 
                        }
                    }
                    
                    //////////////////////////////////////////////////
                    
                    // Hit counter2
                    else if(m_currentBlock == m_event.getRODstartingBlock() + 10+ jump) 
                    {
                        IF_DEBUG cout << "N Hit Words 4-7:\t(";
                        vector<int> hitsfound = m_event.setHits(2, current_chunk);
                        IF_DEBUG {
                            for(int l = 0; l < 4; l++){
                                cout << hitsfound[l] << ", "; 
                            }
                            cout << ")" ;
                        }
                    }
                    
                    //////////////////////////////////////////////////
                    
                    // AM trigger data
                    else if(IS_STARTTRIGGERDATA(current_chunk) && !END_EVT(current_chunk) && (m_currentBlock > m_event.getRODstartingBlock() + 10)) 
                    {
                        m_event.startTrigger(m_currentBlock); //opened
                        IF_DEBUG cout << "Start trigger";
                        unsigned long currentTS = GET_TIMESTAMP(current_chunk);
                        if( currentTS != m_event.getTimestamp() ) {
                            m_event.addError(kTIMESTAMP, m_currentBlock, current_chunk  );
			    m_hist_errors->Fill(kTIMESTAMP);
			    m_event.setWarning();
  	 		    IF_DEBUG cout << "  @w "<<errorStrings[kTIMESTAMP]<<endl;
			    //                            throw wrongTimestamp;
                        }
                        
                        m_event.setNumberOfTracks( GET_TRACKWORDS(current_chunk) );  //number of expected tracks
			IF_DEBUG cout << "\t" << m_event.getNumberOfTracks() << " trks";
                        if( m_event.getNumberOfTracks() == 0 )
                        {
                            m_event.endTrigger(); //opened + closed
                            IF_DEBUG cout << " End Trigger";
                        }  
                    } // end AM trigger data
                    
                    //////////////////////////////////////////////////
                    
                    // AM data
                    else if(  m_event.triggerHasStarted() 
			      && m_currentBlock <= m_event.getTriggerStartingBlock() + m_event.getNumberOfTracks() ) 
                    { 

                        //trigger opened but not closed 
                        if(IS_TRIGGERDATA(current_chunk) && !IS_TRIGGERLASTWORD(current_chunk))
			  {
			    IF_DEBUG cout << "- AM Trigger data";
			    m_event.addAMdata(current_chunk);

			    unsigned long currentTS = GET_HITS_TIMESTAMP(current_chunk);
			    if( currentTS != m_event.getTimestamp() ){
			      m_event.addError(kTIMESTAMP, m_currentBlock, current_chunk  );
			      m_hist_errors->Fill(kTIMESTAMP);
			      m_event.setWarning();

			      IF_DEBUG cout << "  @w "<<errorStrings[kTIMESTAMP];
			      IF_DEBUG cout << "\t curr=" << currentTS << " vs ts=" << m_event.getTimestamp();
			      IF_DEBUG cout << endl;
			      //IF_DEBUG cout << getBufferAsBinaryString(LASTWORD) << endl;
			      throw wrongTimestamp;
			    }
			  }
                        else
			  //if(IS_TRIGGERLASTWORD(current_chunk)) // lsb up (odd number)
                        {
			  //unsigned long currentTS = GET_TIMESTAMP(current_chunk);
                            IF_DEBUG cout << "End trigger ";
                                              
                            if(m_currentBlock < m_event.getTriggerStartingBlock() + m_event.getNumberOfTracks()-1 ) {
                                m_event.addError(kTRIGGER_LESS, m_currentBlock, current_chunk  );
				m_hist_errors->Fill(kTRIGGER_LESS);
				//m_event.setWarning();
                                //throw lessTrigger;
				IF_DEBUG cout << " @w Less trigger informations than expected";
                            }
                            else if(m_currentBlock > m_event.getTriggerStartingBlock() + m_event.getNumberOfTracks() ) 
			      {
                                m_event.addError(kTRIGGER_MORE, m_currentBlock, current_chunk  );
				m_hist_errors->Fill(kTRIGGER_MORE);
				//m_event.setWarning();
                                //throw moreTrigger;
				IF_DEBUG cout << " @w More trigger informations than expected";
                            }
			    m_event.endTrigger();
			    //else {
			    //  m_event.endTrigger();
			    //}
                        }
                        
                    } // end AM data
                    
                    //////////////////////////////////////////////////
                    
                    // Hit blocks	
                    // up to 8 hb, one per layer	
                    else if( IS_HITBLOCK(current_chunk) 
			     && m_currentBlock <= 8 + m_event.getTriggerStartingBlock() + m_event.getNumberOfTracks() + m_event.getTotHits() )
                    { 
			int line           = GET_INPUTLINE(current_chunk); //line 0-7;
			m_lastHitBlockLine = line;

                        IF_DEBUG cout << "Hit Block for line " << line ;
                        if( line>7 ) {
                            m_event.addError(kINPUT_LINE, m_currentBlock, current_chunk );
			    m_hist_errors->Fill(kINPUT_LINE);
			    m_currentBlock++;
                            throw wrongInputLine;
                        }
                        
                        if (    GET_HITS_TIMESTAMP(current_chunk) != 0 //temporary solution! 
                                && GET_HITS_TIMESTAMP(current_chunk) != m_event.getTimestamp() ) {
			    m_event.addError(kTIMESTAMP, m_currentBlock, current_chunk );
			    m_hist_errors->Fill(kTIMESTAMP);
			    //m_currentBlock++;
                            //throw wrongTimestamp;
                        }
                        
                        if(m_event.getLayer(line).hit_block_position == 0) 
                        { 
                            //starting block has not been set
                            m_event.getLayer(line).hit_block_position = m_currentBlock; 
                            m_event.getLayer(line).number_of_words    = GET_HITBLOCK_WORDS(current_chunk);	 
                        }
                        
                        _HIT hit;
			hit.raw = current_chunk;
                        m_event.getLayer(line).hits.push_back(hit);
			
                        
                        
                        if(IS_HITBLOC_LASTWORD(current_chunk)
                           && m_currentBlock <  m_event.getLayer(line).hit_block_position 
			                        + m_event.getLayer(line).number_of_words 
                           ) {
			  //m_event.setCorrupted();
			    m_event.addError(kHITBLOCK, m_currentBlock, current_chunk );
			    m_hist_errors->Fill(kHITBLOCK);
			    m_event.setWarning();
                            IF_DEBUG cout << " @ Hit block for selected line has fewer words than expected.";
                            //                                throw wrongHitBlockWords;
			}
                    }
                    
		    
		    else if(!IS_MARKER_ROD_ENDDATA(current_chunk) 
			    && m_lastHitBlockLine >= 7 
			    && m_currentBlock <= m_event.getRODstartingBlock() + m_event.getExpectedLength() )
		    {
		      IF_DEBUG cout << "- Scint flag\t ts=" << GET_SCINT_TIMESTAMP(current_chunk); 
		      m_event.addScintFlag(current_chunk);
		    }
                    else if( IS_MARKER_ROD_ENDDATA(current_chunk) )
                    {
                        IF_DEBUG cout << "End ROD";
                        m_event.endROD(m_currentBlock);
			next_is_checksum = true;
			m_lastHitBlockLine = -1;
                    }
		    

                }
                
                checksum ^= current_chunk;
                
                //////////////////////////////////////////////////
                //Checksum control
		if( m_currentBlock == m_event.getRODendingBlock() + 1)
		  {
		    next_is_checksum = false;
		    IF_DEBUG cout << "End ROB; ";
		    m_event.endROB(m_currentBlock); //?
		    if(checksum != current_chunk) 
		      {
			IF_DEBUG cout << " Wrong checksum (" << checksum << ")" ;
			m_event.addError(kCHECKSUM, m_currentBlock, current_chunk);
			m_hist_errors->Fill(kCHECKSUM);
			m_currentBlock++;  
			throw wrongChecksum;
		      }
		    else
		      {
			IF_DEBUG cout << " Checksum ok";
		      }
                    
		  }
                
                //////////////////////////////////
                
            }
            
            //////////////////////////////////////////////////
            
            // Next event or run end	
            else if( current_chunk == EVT_NEXT || current_chunk == RUN_END )
	      {
                IF_DEBUG cout << "End of Event";
		if( current_chunk == RUN_END ) IF_DEBUG cout << "End of Run";
                m_event.endEvent(m_currentBlock);
                reached_end_of_event = true;
                eventFound = true;
		if( !m_event.CheckNRobs() ){
                  m_event.addError(kWRONGNROBS, m_currentBlock, current_chunk);
		  m_hist_errors->Fill(kWRONGNROBS);
		  
		}
                //if( (m_currentBlock - m_event.getStartingBlock() + 2 ) != m_event.getExpectedLength() ) throw wrongLength;
		
	      }
	    } //endif: event has started
	  }// end already-started-event processing
        
        //next block is coming...
        m_filePosition += WORD_SIZE;
        m_currentBlock++;
       	//        delete [] hex_chunk ;
        IF_DEBUG cout << endl;
    }//end main while
    
    m_lastEvent = &m_event.getEvent();
 
    return eventFound;
}


//////////////////////////////////////////////////



_EVENT& Slim5OutputParser::GetEvent()
{
  return *m_lastEvent;
}



void Slim5OutputParser::resetErrorStats()
{
  m_hist_errors->Reset();
  m_event.resetErrorDB();
}

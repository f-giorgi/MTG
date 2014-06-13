#include "parser.h"

uint32_t parser::getBClat(){
  if (TimeRes > 0){
    return trig_latency/int(TimeRes*1.0e+6);
  }
  cout<<"ERROR(parser): TimeRes is 0 when calling getBClat()"<<endl;
  return 0;
}

uint32_t parser::getMaskedValue(uint32_t mask, uint32_t val){
    uint32_t idx = 0;
    uint32_t msk = mask;
    //nothing selected
    if (mask == 0) return 0;
    while ((msk & 0x1) == 0){
      msk >>= 1;
      idx ++; //index of first bit in word (0 -> N-1)
    }
    return  ((mask & val)>>idx);
}




parser::parser(ifstream & file){
  attachOutputFile(file);
  Init();
  if(verbosity==kMEDIUM || verbosity==kDEBUG) cout<<"INFO(parser): New parser created"<<endl;
}

parser::parser(){
  Init();			
  if(verbosity==kMEDIUM || verbosity==kDEBUG) cout<<"INFO(parser): New parser created"<<endl;
}

void parser::Init(){
  verbosity = kSILENT;
  setDataType(simuSQARE200x256);
  setChipProperties(simuSQARE200x256);
  idx=0;	
  Time=0;
  triggered = false;
  trig_latency=0;
  TimeRes = 0.0;
  hits_found=0;
  lastTS=0;
  lastSM_ID=0;
}

void parser::listDataTypes(){
  
  cout<<" simuSORTEX200x256    = 1" <<endl;
  cout<<" simuSQARE200x256     = 2" <<endl;
  cout<<" FE4DtestPISA         = 3" <<endl; 
  cout<<" APSEL4DmicroBOLOGNA  = 4" <<endl;
  cout<<" simuSQARE2x16x32     = 5" <<endl;
  cout<<" simuGenList          = 6" <<endl;
  cout<<" DAQLayerDump         = 7" <<endl;
  cout<<" StripFEBuffer        = 8" <<endl;
  cout<<" simuSQARE2x48x128    = 9" <<endl;
  cout<<" simuSQARE2x48x128raw = 10"<<endl;
  cout<<" simuSQARE2x16x128    = 11"<<endl;
  cout<<" simuSQARE2x16x128raw = 12"<<endl;

}

unsigned int parser::from_gray_conv(unsigned int TS){
  unsigned int result				=0;
  unsigned int bit_shifter	=0;	
  unsigned int mask					=0x80;
  bit_shifter = TS & mask;
  result = result | bit_shifter;	
  //cout<<TS<<"  "<<result<<"  "<<bit_shifter<<"  "<<mask<<endl;
  for (int i=6; i>=0; i--){
    mask = mask>>1;
    bit_shifter	= (bit_shifter>>1)^(TS & mask);
    //		bit_shifter=((TS & mask)>>1)^(TS&(mask>>1))	;	
    
    result = result | bit_shifter;	
    //cout<<TS<<"  "<<result<<"  "<<bit_shifter<<"  "<<mask<<endl;
  }
  return result;
}


bool parser::and_array(vector<bool> & x){
  unsigned int count = 0;
  for (unsigned int i=0; i<x.size(); i++){
    if (x[i]==true){
      count++;
    }
  }
  if (count==x.size()) return true; else return false;
}


bool parser::attachOutputFile(ifstream  &file){
  outFile = &file;
  return OutputfileIsAttached();
}

bool parser::attachTrigFile(ifstream  &file){
  trigFile = &file;
  return TrigfileIsAttached();
}

void parser::setDataType(_DATATYPE DataType){
  dataType = DataType;
  setChipProperties(DataType);  //try to auto-fill the chip properties from the data-type specified
}

void parser::setVerbosity(_VERBOSITY level){
  verbosity=level;
  if (verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"INFO(parser): verbosity level set to "<<level<<endl;
}

void parser::setTrigLatency(unsigned int lat){
  if (triggered){
    trig_latency = lat;
    if (verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"INFO(parser): trigger latency set to "<<lat<<endl;
  } else {
    trig_latency = 0;
    cout<<"WARNING(parser): request to set trigger latency when parser is in not-triggered mode. Latency is set to 0"<<endl;
  }
}

/* void parser::setBClat(int x){ */
/*   BClat = x; */
/*   if (verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"INFO(parser): BC latency set to "<<x<<endl; */
/* } */

void parser::setTimeRes(double x){
  TimeRes = x;
  if (verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"INFO(parser): Time Res set to "<<x<<endl;
}


void parser::setTriggeredFlag(int x){
  if (x == 0) {
    trig_latency = 0;
    triggered = false;
  }else{
    triggered = true;
  }
  if (verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"INFO(parser): triggered flag set to "<<triggered<<endl;
}


void parser::fileRewind(){
  if (OutputfileIsAttached() && outFile->is_open()){
    outFile->clear();
    outFile->seekg(0,ios::beg);
  if (verbosity == kMEDIUM || verbosity==kDEBUG)   cout<<"INFO(parser)file Rewind, pointer position: "<<outFile->tellg()<<endl;
  }
} 

bool parser::Read_SQ_Header(ifstream & file, _SQUAREHEADER & SQ_header){
  if(!(file>>SQ_header.submatr_addr)) {
     if(verbosity==kDEBUG) cout<<"SQ_header.submatr_addr error"<<SQ_header.submatr_addr<<endl;
    return false;
  }
  if(!(file>>SQ_header.dummy)) {
    if(verbosity==kDEBUG) cout<<"SQ_header.dummy error"<<SQ_header.dummy<<endl;
    return false;
  }	
  file>>hex;
  if(!(file>>SQ_header.TS)){ 
    if(verbosity==kDEBUG) cout<<"SQ_header.TS error"<<SQ_header.TS<<endl;
    file>>dec;
    return false;
  }
  file>>dec;	
  //if(verbosity==kDEBUG) cout<<" SQ_header.submatr_addr "<<SQ_header.submatr_addr<<" SQ_header.dummy "<<SQ_header.dummy<<" SQ_header.TS "<<SQ_header.TS<<" file get pointer "<<outFile->tellg()<<endl;
  
  return true;
}

bool parser::Read_SQ_Hit(ifstream & file, _SQUAREHIT&  SQ_hit){
  if(!(file>>SQ_hit.submatr_addr)) {
    if(verbosity==kDEBUG) cout<<"SQ_hit.submatr_addr error"<<SQ_hit.submatr_addr<<endl;		
    return false;
  }
  if(!(file>>SQ_hit.B2_addr)) {
    if(verbosity==kDEBUG) cout<<"SQ_hit.B2_addr error"<<SQ_hit.B2_addr<<endl;
    return false;
  }
  if(!(file>>SQ_hit.X_address)){ 
    if(verbosity==kDEBUG) cout<<"SQ_hit.X_address error"<<SQ_hit.X_address<<endl;
    return false;
  }
  if(!(file>>SQ_hit.Y_address)){
    if(verbosity==kDEBUG) cout<<"SQ_hit.Y_address error"<<SQ_hit.Y_address<<endl;
    return false;
  }
	
  //if(verbosity==kDEBUG) cout<<" SQ_hit.submatr_addr "<<SQ_hit.submatr_addr<<" SQ_hit.B2_addr "<<SQ_hit.B2_addr<<" SQ_hit.X_address "<<SQ_hit.X_address<<" SQ_hit.Y_address"<<SQ_hit.Y_address<<" file get pointer "<<outFile->tellg()<<endl;
	
	
  return true;
}

bool parser::Read_RAW_SQ_TS(uint32_t data, _DATATYPE type,  _SQUAREHEADER & SQ_header){
  if (type == simuSQARE2x48x128raw){
    SQ_header.submatr_addr = getMaskedValue(SQARE2x48x128_SUBMAT_ID, data);
    SQ_header.parity       = getMaskedValue(SQARE2x48x128_PARITY_MASK, data);   
    SQ_header.hitRejectB1  = getMaskedValue(SQARE2x48x128_HRB1_MASK, data);
    SQ_header.sweepJump    = getMaskedValue(SQARE2x48x128_SJ_MASK, data);
    SQ_header.hitRejectB2  = getMaskedValue(SQARE2x48x128_HRB2_MASK, data);
    SQ_header.TS           = getMaskedValue(SQARE2x48x128_TS, data); 
    if (verbosity==kDEBUG) cout<<"INFO(parser): header TS is (dec): "<<SQ_header.TS <<" mat ID : "<<SQ_header.submatr_addr<<endl;
    return true;
  } else if (type == simuSQARE2x16x128raw){
    SQ_header.submatr_addr = getMaskedValue(SQARE2x16x128_SUBMAT_ID, data);
    SQ_header.parity       = getMaskedValue(SQARE2x16x128_PARITY_MASK, data);   
    SQ_header.hitRejectB1  = getMaskedValue(SQARE2x16x128_HRB1_MASK, data);
    SQ_header.sweepJump    = getMaskedValue(SQARE2x16x128_SJ_MASK, data);
    SQ_header.hitRejectB2  = getMaskedValue(SQARE2x16x128_HRB2_MASK, data);
    SQ_header.TS           = getMaskedValue(SQARE2x16x128_TS, data); 
    if (verbosity==kDEBUG) cout<<"INFO(parser): header TS is (dec): "<<SQ_header.TS <<" mat ID : "<<SQ_header.submatr_addr<<endl;
    return true;
    
  }
  return false;
}

bool parser::Read_RAW_SQ_HITS(uint32_t data, _DATATYPE type, int submatID, vector<_HIT> &  hits){
  _HIT hit;
  uint32_t N_hit = 0;
  uint32_t bitAddress = 0;
  uint32_t pattern = 0;
  if (type == simuSQARE2x48x128raw){
    if (getMaskedValue(SQARE2x48x128_TSorHIT_MASK, data)){
      cout<<"ERROR(parser): tried to read a SQ_TS as a Hit"<<endl;
      return false;
    }
    pattern = getMaskedValue(SQARE2x48x128_Z_PATTERN, data);
    while (pattern){
      if (bitAddress>32){
        cout<<"ERROR(parser): pattern in RAW hit is too long"<<endl;
        return false;
      }
      if (pattern & 0x1){
        N_hit ++;
        hit.X_address    = submatID*48 + getMaskedValue(SQARE2x48x128_X_ADDR , data);
        hit.Y_address    = getMaskedValue(SQARE2x48x128_SPARS_ADDR ,data)*32 + getMaskedValue(SQARE2x48x128_YZONE_ADDR ,data)*4 + bitAddress;
        hits.push_back(hit);
        if (verbosity==kDEBUG) cout<<"INFO(parser): HitX: "<<hit.X_address <<" HitY : "<<hit.Y_address<<endl;
      }
      pattern >>= 1;
      bitAddress ++;
    }
    if (N_hit == 0) {
      cout<<"ERROR(parser): hit with no active bits in pattern"<<endl; 
      return false;
    } else return true;
  } else   if (type == simuSQARE2x16x128raw){
    if (getMaskedValue(SQARE2x16x128_TSorHIT_MASK, data)){
      cout<<"ERROR(parser): tried to read a SQ_TS as a Hit"<<endl;
      return false;
    }
    pattern = getMaskedValue(SQARE2x16x128_Z_PATTERN, data);
    while (pattern){
      if (bitAddress>32){
        cout<<"ERROR(parser): pattern in RAW hit is too long"<<endl;
        return false;
      }
      if (pattern & 0x1){
        N_hit ++;
        hit.X_address    = submatID*16 + getMaskedValue(SQARE2x16x128_X_ADDR , data);
        hit.Y_address    = getMaskedValue(SQARE2x16x128_SPARS_ADDR ,data)*32 + getMaskedValue(SQARE2x16x128_YZONE_ADDR ,data)*4 + bitAddress;
        hits.push_back(hit);
        if (verbosity==kDEBUG) cout<<"INFO(parser): HitX: "<<hit.X_address <<" HitY : "<<hit.Y_address<<endl;
      }
      pattern >>= 1;
      bitAddress ++;
    }
    if (N_hit == 0) {
      cout<<"ERROR(parser): hit with no active bits in pattern"<<endl; 
      return false;
    } else return true;
  }


  cout<<"ERROR(parser): trying to read a RAW SQARE hit from an unusual data type "<<endl;
  return false;
}



bool parser::Check_SQ_TS(unsigned int N_submat, int TS, vector<int> & TS_buff){
  //se e' la prima del giro la metto e torno, non perdo tempo.
  uint32_t buff_size = TS_buff.size();
  if(buff_size == 0){
    TS_buff.clear();
    TS_buff.push_back(TS);
    return true;
  } 
  if(buff_size<N_submat){
    for(uint32_t i =0; i<buff_size; i++){
      if(TS != TS_buff[i]){
        if(verbosity== kMEDIUM || kDEBUG) cout<<"ERROR(parser): found misordered Time stamps"<<endl;
        return false;
      }			
    }
    TS_buff.push_back(TS);				
  }else if (buff_size==N_submat){ 
    //controllo consecutivita'
    if(TS != ((TS_buff[N_submat-1]+1)%256) && triggered==false){
      if(verbosity== kMEDIUM || kDEBUG) cout<<"WARNING(parser): consecutivity not respected. Ignore if calibration run."<<endl;
      //return false;
    }
    TS_buff.clear();					
    TS_buff.push_back(TS);							
		
  }
  return true;
}



//fai il controllo linea per linea, altrimenti se ne accorge alla fine....
bool parser::CheckOutputFile(_DATATYPE DataType){
  if(verbosity==kMEDIUM || verbosity==kDEBUG) cout<<"INFO(parser): start checking outFile..."<<endl;
  //bool read_ok = false;
  int idx=0;
  uint32_t data=0;
  _SQUAREHEADER SQ_header;
  _SQUAREHIT 		SQ_hit;
  vector<_HIT> hits;
  hits.clear();

  vector<int> TS_buff;
  TS_buff.clear();
  
  if(!OutputfileIsAttached()){
    cout<<"ERROR(parser): outFile is not attached"<<endl;
    return false;
  }
  
  if(!(outFile->is_open())){
    cout<<"ERROR(parser): could not open outFile"<<endl;
    return false;
  }

  if(verbosity==kDEBUG) cout<<"INFO(parser): Data File Rewind"<<endl;
  fileRewind();
  
  if(verbosity==kDEBUG) cout<<"INFO(parser): Entering main switch "<<endl;
  switch (DataType) {
    case simuSORTEX200x256:
      break;		
      
      ////////////////////////////////////////////////////////////////////////////////////////
    case simuSQARE200x256:
      TS_buff.clear();
      //check for data integrity, and TS check (all and ordered)
      while(!(outFile->eof())){
       *outFile>>idx;
       // if (read_ok){
        if (outFile->good()){
          if(idx==-1) {
            //leggo l'header e controllo che sia intero
            if(!Read_SQ_Header(*outFile, SQ_header)) {
              if(verbosity==kMEDIUM || verbosity==kDEBUG) cout<<"ERROR(parser): output file is not terminated properly (header) TS: "<<SQ_header.TS<<endl;
              return false; 
            }
            if(!Check_SQ_TS(4, from_gray_conv(SQ_header.TS), TS_buff)){
              if(verbosity==kMEDIUM || verbosity==kDEBUG) cout<<"ERROR(parser): TS check gone BAD"<<endl;
              return false;
            }
          }else{
            if(verbosity==kDEBUG) cout<<"Analyzing hit IDX "<<idx<<endl;
            //leggo la hit e controllo che sia intera
            if(!Read_SQ_Hit(*outFile, SQ_hit)) {
              if(verbosity==kMEDIUM || verbosity==kDEBUG) cout<<"ERROR(parser): output file is not terminated properly (hit) idx:"<<idx<<endl;
              return false; 
            }					
          }
        } else if(outFile->eof()){
            if(verbosity==kDEBUG) cout<<"INFO(parser): End of Output file reached "<<endl;
        } else {
            cout<<"ERROR(parser): Output File check FAILED  "<<endl;
            return false;
        }
      }
      break;
      ///////////////////////////////////////////////////////////////////////////////////////////
    case APSEL4DmicroBOLOGNA:
      break;
    case FE4DtestPISA:
      break;
    case simuSQARE2x16x32:
    case simuSQARE2x48x128:
    case simuSQARE2x16x128:
     TS_buff.clear();
      //check for data integrity, and TS check (all and ordered)
      while(!(outFile->eof())){
        if(verbosity==kDEBUG) cout<<"INFO(parser): Get first item in file"<<endl;
        *outFile>>idx;
        if (outFile->good()){
          if(idx==-1) {
            //leggo l'header e controllo che sia intero
            if(!Read_SQ_Header(*outFile, SQ_header)) {
              cout<<"ERROR(parser): output file is not terminated properly (header) TS: "<<SQ_header.TS<<endl;
              return false; 
            }
            if(!Check_SQ_TS(2, from_gray_conv(SQ_header.TS), TS_buff)){
              cout<<"ERROR(parser): TS check gone BAD"<<endl;
              return false;
            }
          }else{
            if(verbosity==kDEBUG) cout<<"Analyzing hit IDX "<<idx<<endl;
            //leggo la hit e controllo che sia intera
            if(!Read_SQ_Hit(*outFile, SQ_hit)) {
              cout<<"ERROR(parser): output file is not terminated properly (hit) idx:"<<idx<<endl;
              return false; 
            }					
          }
        } else if(outFile->eof()){
            if(verbosity==kDEBUG) cout<<"INFO(parser): End of Output file reached "<<endl;
        } else {
            cout<<"ERROR(parser): Output File check FAILED  "<<endl;
            return false;
        }

      }
      break;
    case simuSQARE2x48x128raw:     
      TS_buff.clear();
      if(verbosity==kDEBUG) cout<<"INFO(parser): checking a simuSQARE2x48x128raw file "<<endl;
      while(!(outFile->eof())){
        *outFile>>hex>>data;
        if(outFile->good()){
          if(verbosity==kDEBUG) cout<<"INFO(parser): read data: "<<hex<< data <<dec<< endl; 
          //if it is a TIME STAMP
          if (getMaskedValue(SQARE2x48x128_TSorHIT_MASK, data)){
            if(! Read_RAW_SQ_TS(data, DataType, SQ_header)) {
              cout<<"ERROR(parser): output file is not terminated properly (header) TS: "<<SQ_header.TS<<endl;
              return false;            
            }
            if(!Check_SQ_TS(2, from_gray_conv(SQ_header.TS), TS_buff)){
              cout<<"ERROR(parser): TS check gone BAD"<<endl;
              return false;
            }
          }else{
            //leggo la hit e controllo che sia intera
            if(!Read_RAW_SQ_HITS(data, DataType,0, hits)) { //since I don't mind the real hit coordinates, I put sm 0 by default
              cout<<"ERROR(parser): output file is not terminated properly (hit) idx:"<<idx<<endl;
              return false; 
            }					           
          }
        } else if(outFile->eof()){
            if(verbosity==kDEBUG) cout<<"INFO(parser): End of Output file reached "<<endl;
        } else {
            cout<<"ERROR(parser): Output File check FAILED  "<<endl;
            return false;
        }
      }
      break;
    case simuSQARE2x16x128raw:     
      TS_buff.clear();
      if(verbosity==kDEBUG) cout<<"INFO(parser): checking a simuSQARE2x16x128raw file "<<endl;
      while(!(outFile->eof())){
        *outFile>>hex>>data;
        if(outFile->good()){
          if(verbosity==kDEBUG) cout<<"INFO(parser): read data: "<<hex<< data <<dec<< endl; 
          //if it is a TIME STAMP
          if (getMaskedValue(SQARE2x16x128_TSorHIT_MASK, data)){
            if(! Read_RAW_SQ_TS(data, DataType, SQ_header)) {
              cout<<"ERROR(parser): output file is not terminated properly (header) TS: "<<SQ_header.TS<<endl;
              return false;            
            }
            if(!Check_SQ_TS(2, from_gray_conv(SQ_header.TS), TS_buff)){
              cout<<"ERROR(parser): TS check gone BAD"<<endl;
              return false;
            }
          }else{
            //leggo la hit e controllo che sia intera
            if(!Read_RAW_SQ_HITS(data, DataType,0, hits)) { //since I don't mind the real hit coordinates, I put sm 0 by default
              cout<<"ERROR(parser): output file is not terminated properly (hit) idx:"<<idx<<endl;
              return false; 
            }					           
          }
        } else if(outFile->eof()){
            if(verbosity==kDEBUG) cout<<"INFO(parser): End of Output file reached "<<endl;
        } else {
            cout<<"ERROR(parser): Output File check FAILED  "<<endl;
            return false;
        }
      }
      break;
    case simuGenList:
      break;
    case DAQLayerDump:
      break;
    case StripFEBuffer:
      break;
    default:
      return false;
  }
  
  //at the end of check, reset the outFile error bits (eof) and the read pointer
  fileRewind();
  return true;
}






bool parser::findEvent(_EVENT & evt){
  bool eoe = false;  //end of event
  bool newEventFound = false;
  bool newHitFound = false;
  bool read_ok = false;
  long getPointerPosition=0;
  long getPointerPosition2=0;
  int SOSrecevied=0;
  int curTS=-1;
  //uint32_t phase = 8000;
  //int deltaTS;
  int TS=0;
  int sh_n = 0;
  evt.Reset();
  _HIT readHit;
  uint32_t BClat;
  //_EVENT readEvent;
  int idx_out, submatr_addr_out, spars_addr_out;
  int N_word;
  int word;
  //bool firstRead = true;
  unsigned int raw;
  _TRIG_DATA_LINE TDline; 
  _BUF_DATA_LINE BDline;
  _GEN_DATA_LINE GDline;
  _SQUAREHEADER SQ_header;
  vector<_HIT> hits;
  //simulation time must be saved in long unisgned variable due to its rage
  //unsigned long int inBufferTime;
  //unsigned long int sim_time;
  string t_res="";
  int trig;
  //unsigned long int trig_time;
  string trig_res;
 	
  //if (verbosity==kDEBUG)cout<<"BRKPNT 0"<<endl;
  if (OutputfileIsAttached()){
    //if (verbosity==kDEBUG)cout<<"BRKPNT 1"<<endl;
    switch (dataType){
      case simuSORTEX200x256: return false;
        cout<<"ERROR(parser):  simuSORTEX200x256 --parser not implemented yet"<<endl;
        break;
      case simuSQARE200x256:	
				
        //ricordati di riempire Time e abs time in questa architettura
        while(!eoe){
          //if (verbosity==kDEBUG)cout<<"BRKPNT 2"<<endl;
          if(!outFile->eof()){
            //if (verbosity==kDEBUG)cout<<"BRKPNT 3"<<endl;
            //salvo la posizione corrente del getPointer
            getPointerPosition = outFile->tellg();
            read_ok = *outFile>>idx_out;									
            if (read_ok){
              if (verbosity==kDEBUG)cout<< getPointerPosition<<"  ";
              if (verbosity==kDEBUG)cout<<idx_out;									
              if (idx_out==-1){
                if (newEventFound) { //it is next event header
                  eoe=true;
                  //ho letto il successivo header, torno indietro di una lettura.
                  if (verbosity==kDEBUG)cout<<" "<<getPointerPosition-outFile->tellg()<<" new header found, moving back getPointer "<<endl;						
                  outFile->seekg((getPointerPosition-outFile->tellg()),ios::cur);			
                }else	{ //e' uno dei 4 SOS attesi
                  *outFile>>submatr_addr_out;
                  *outFile>>spars_addr_out;
                  *outFile>>hex>>TS>>dec;
                  if (verbosity==kDEBUG)cout<<" "<<from_gray_conv(TS)<<endl;						
                  if (SOSrecevied<3){	
                    SOSrecevied++;
                  }else{
                    if (verbosity==kDEBUG)cout<<"Parsed Event. TSgray= 0x"<<hex<<setw(2)<<setfill('0')<<TS
                      <<" TS= 0x"<<setw(2)<<setfill('0')<<from_gray_conv(TS)<<dec
                      <<" TSdec= "<<dec<<from_gray_conv(TS)<<endl;												
                    //si suppone che il check del file abbia dato esito positivo. (in questo caso i TS siano immediatamente consecutivi
                    evt.TimeStamp = from_gray_conv(TS);
                    newEventFound = true;
                  }
                }
              }else{
                *outFile>>submatr_addr_out;
                *outFile>>spars_addr_out;
                *outFile>>readHit.X_address;
                *outFile>>readHit.Y_address;
                evt.hits.push_back(readHit);
                if (verbosity==kDEBUG)cout<<" "<<readHit.X_address<<" "<<readHit.Y_address<<endl;						
              }
            }									
          } else eoe=true;
        }							
        return newEventFound;
        //ricordati di ritornare qualcosa
        break;
    case simuSQARE2x48x128raw:
         while(!eoe){
          if(!outFile->eof()){
            //salvo la posizione corrente del getPointer
            getPointerPosition = outFile->tellg();									
            if (*outFile>>word){
              if (verbosity==kDEBUG)cout<< getPointerPosition<<"  ";
              if (verbosity==kDEBUG)cout<<hex<<word<<dec;									
              if (getMaskedValue(SQARE2x48x128_TSorHIT_MASK, word) ){ //if its isa time stamp
                if (newEventFound) { //it is next event header
                  eoe=true;
                  //ho letto il successivo header, torno indietro di una lettura.
                  if (verbosity==kDEBUG)cout<<" "<<getPointerPosition-outFile->tellg()<<" new header found, moving back getPointer "<<endl;						
                  outFile->seekg((getPointerPosition-outFile->tellg()),ios::cur);			
                }else	{ //e' uno dei 2 SOS attesi
                  Read_RAW_SQ_TS(word, dataType, SQ_header);
                  lastSM_ID = SQ_header.submatr_addr;
                  SQ_header.submatr_addr == 0 ? sh_n = 0 : sh_n=8;
                  evt.flags |= SQ_header.hitRejectB1<<(2+sh_n) | SQ_header.sweepJump <<(1+ sh_n) | SQ_header.hitRejectB2<<sh_n;
                  if (verbosity==kDEBUG)cout<<" "<<from_gray_conv(SQ_header.TS)<<endl;						
                  if (SOSrecevied<1){	
                    SOSrecevied++;
                  }else{
                    if (verbosity==kDEBUG)cout<<"Parsed Event. TSgray= 0x"<<hex<<setw(2)<<setfill('0')<<SQ_header.TS
                      <<" TS= 0x"<<setw(2)<<setfill('0')<<from_gray_conv(SQ_header.TS)<<dec
                      <<" TSdec= "<<dec<<from_gray_conv(SQ_header.TS)<<endl;												
                    //si suppone che il check del file abbia dato esito positivo. (in questo caso i TS siano immediatamente consecutivi
                    evt.TimeStamp = from_gray_conv(SQ_header.TS);
                    newEventFound = true;
                  }
                }
              }else{
                Read_RAW_SQ_HITS(word, dataType,lastSM_ID, evt.hits);
                //while(!hits.empty())
                //evt.hits.push_back(hits.pop_back());
                
                //  if (verbosity==kDEBUG)cout<<" "<<readHit.X_address<<" "<<readHit.Y_address<<endl;						
              }
            }									
          } else eoe=true;
        }							
        return newEventFound;

      break;
    case simuSQARE2x16x128raw:
         while(!eoe){
          if(!outFile->eof()){
            //salvo la posizione corrente del getPointer
            getPointerPosition = outFile->tellg();									
            if (*outFile>>word){
              if (verbosity==kDEBUG)cout<< getPointerPosition<<"  ";
              if (verbosity==kDEBUG)cout<<hex<<word<<dec;									
              if (getMaskedValue(SQARE2x16x128_TSorHIT_MASK, word) ){ //if its isa time stamp
                if (newEventFound) { //it is next event header
                  eoe=true;
                  //ho letto il successivo header, torno indietro di una lettura.
                  if (verbosity==kDEBUG)cout<<" "<<getPointerPosition-outFile->tellg()<<" new header found, moving back getPointer "<<endl;						
                  outFile->seekg((getPointerPosition-outFile->tellg()),ios::cur);			
                }else	{ //e' uno dei 2 SOS attesi
                  Read_RAW_SQ_TS(word, dataType, SQ_header);
                  lastSM_ID = SQ_header.submatr_addr;
                  SQ_header.submatr_addr == 0 ? sh_n = 0 : sh_n=8;
                  evt.flags |= SQ_header.hitRejectB1<<(2+sh_n) | SQ_header.sweepJump <<(1+ sh_n) | SQ_header.hitRejectB2<<sh_n;
                  if (verbosity==kDEBUG)cout<<" "<<from_gray_conv(SQ_header.TS)<<endl;						
                  if (SOSrecevied<1){	
                    SOSrecevied++;
                  }else{
                    if (verbosity==kDEBUG)cout<<"Parsed Event. TSgray= 0x"<<hex<<setw(2)<<setfill('0')<<SQ_header.TS
                      <<" TS= 0x"<<setw(2)<<setfill('0')<<from_gray_conv(SQ_header.TS)<<dec
                      <<" TSdec= "<<dec<<from_gray_conv(SQ_header.TS)<<endl;												
                    //si suppone che il check del file abbia dato esito positivo. (in questo caso i TS siano immediatamente consecutivi
                    evt.TimeStamp = from_gray_conv(SQ_header.TS);
                    newEventFound = true;
                  }
                }
              }else{
                Read_RAW_SQ_HITS(word, dataType,lastSM_ID, evt.hits);
                //while(!hits.empty())
                //evt.hits.push_back(hits.pop_back());
                
                //  if (verbosity==kDEBUG)cout<<" "<<readHit.X_address<<" "<<readHit.Y_address<<endl;						
              }
            }									
          } else eoe=true;
        }							
        return newEventFound;

      break;
    case simuSQARE2x16x32:	
    case simuSQARE2x48x128:
    case simuSQARE2x16x128:
      //ricordati di riempire Time e abs time in questa architettura
        while(!eoe){
          //if (verbosity==kDEBUG)cout<<"BRKPNT 2"<<endl;
          if(!outFile->eof()){
            //if (verbosity==kDEBUG)cout<<"BRKPNT 3"<<endl;
            //salvo la posizione corrente del getPointer
            getPointerPosition = outFile->tellg();
            read_ok = *outFile>>idx_out;									
            if (read_ok){
              if (verbosity==kDEBUG)cout<< getPointerPosition<<"  ";
              if (verbosity==kDEBUG)cout<<idx_out;									
              if (idx_out==-1){
                if (newEventFound) { //it is next event header
                  eoe=true;
                  //ho letto il successivo header, torno indietro di una lettura.
                  if (verbosity==kDEBUG)cout<<" "<<getPointerPosition-outFile->tellg()<<" new header found, moving back getPointer "<<endl;						
                  outFile->seekg((getPointerPosition-outFile->tellg()),ios::cur);			
                }else	{ //e' uno dei 2 SOS attesi
                  *outFile>>submatr_addr_out;
                  *outFile>>spars_addr_out;
                  *outFile>>hex>>TS>>dec;
                  if (verbosity==kDEBUG)cout<<" "<<from_gray_conv(TS)<<endl;						
                  if (SOSrecevied<1){	
                    SOSrecevied++;
                  }else{
                    if (verbosity==kDEBUG)cout<<"Parsed Event. TSgray= 0x"<<hex<<setw(2)<<setfill('0')<<TS
                      <<" TS= 0x"<<setw(2)<<setfill('0')<<from_gray_conv(TS)<<dec
                      <<" TSdec= "<<dec<<from_gray_conv(TS)<<endl;												
                    //si suppone che il check del file abbia dato esito positivo. (in questo caso i TS siano immediatamente consecutivi
                    evt.TimeStamp = from_gray_conv(TS);
                    newEventFound = true;
                  }
                }
              }else{
                *outFile>>submatr_addr_out;
                *outFile>>spars_addr_out;
                *outFile>>readHit.X_address;
                *outFile>>readHit.Y_address;
                evt.hits.push_back(readHit);
                if (verbosity==kDEBUG)cout<<" "<<readHit.X_address<<" "<<readHit.Y_address<<endl;						
              }
            }									
          } else eoe=true;
        }							
        return newEventFound;
        //ricordati di ritornare qualcosa
        break;
      case FE4DtestPISA:			
        if (verbosity==kDEBUG)cout<<"DataType: FE4DtestPISA  --parser not implemented yet"<<endl;
        return false;
        break;
        
      case APSEL4DmicroBOLOGNA: 
        
        
        
        //if (verbosity==kDEBUG)cout<<"DataType: APSEL4DmicroBOLOGNA  --parser not implemented yet"<<endl;
        while(!eoe){
          if(!outFile->eof()){
            getPointerPosition = outFile->tellg();	
            read_ok = *outFile>>idx_out;									
            if (read_ok){
              //if (verbosity==kDEBUG)cout<< getPointerPosition<<"  ";
              //if (verbosity==kDEBUG)cout<<idx_out;									
              *outFile>>hex>>word>>dec;										
              if (word!=0){ 
                if(APSEL4DgetTS(word)!= curTS && newEventFound==false){
                  curTS=APSEL4DgetTS(word);
                  newEventFound=true;							
                  //set the event ID
                  //evt.idx=idx; 
                  //update the idx variable
                  idx++;						
                  //set the event TimeStamp
                  evt.TimeStamp = APSEL4DgetTS(word);
                  //find the Delta TS
                  //if (evt.TimeStamp < lastTS){
                  //	deltaTS = 256-(lastTS-evt.TimeStamp);
                  //}	else {
                  //	deltaTS = evt.TimeStamp-lastTS;
                  //}
                  //if (deltaTS>255) cout<<"Warning: deltaTS>255"<<endl;
                  //lastTS=evt.TimeStamp;		
                  //update the Time variable
                  //Time += deltaTS;
                  //set the event absolute time
                  //evt.AbsTime = Time;
                  //fill evt.hits with the first hit
                  readHit.X_address=APSEL4DgetX(word);
                  readHit.Y_address=APSEL4DgetY(word);
                  evt.hits.push_back(readHit);
                  //update the hits_found variable
                  hits_found++;										
                  
                }else 
                  if (APSEL4DgetTS(word)!= curTS && newEventFound==true){
                    eoe = true;
                    outFile->seekg((getPointerPosition-outFile->tellg()),ios::cur);		
                  }else {												
                    readHit.X_address=APSEL4DgetX(word);
                    readHit.Y_address=APSEL4DgetY(word);
                    evt.hits.push_back(readHit);	
                    //hits_found++;				
                  }
              }
            }										
          } else eoe = true;
        }
        
        return newEventFound;
        break;
        
    case simuGenList: 
      BClat = getBClat();
      getPointerPosition = outFile->tellg(); 
      getPointerPosition2 = getPointerPosition;
      if (!TrigfileIsAttached()) {
        cout<<"ERROR(parser): Trigger File is not attached when finding events"<<endl;
        return false;
      }

      TDline.trig=0;
      //get the next triggered TS
      while (!TDline.trig) {
        if(!ReadTDline(TDline)) {
          if (verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"WARNING(parser): Strip Buffer Event NOT found, could not read a new trigger"<<endl;
          return false;
        }
      }
      if(verbosity==kDEBUG) cout<<"INFO(parser): Trigger Found! TS = "<<TDline.TS<<" (0x"<<hex<<TDline.TS<<dec<<") "
                                <<" sim time "<<TDline.sim_time
                                <<" "<<TDline.tres
                                <<endl;

      TDline.TS-int(BClat)>=0 ? trig = TDline.TS-int(BClat) : trig = TDline.TS-int(BClat)+256; //triggered TS = TDline.TS-BClat taking into 
      evt.TimeStamp = trig;                                                     //account time counter modulo (assumed 8 bit TS!)

      //reach the buffer_data sim_time region where triggered hits were stored (with 1 us of time margin) (
      GDline.sim_time = 0;
      word = 0; //variable re-use. proper name would be "line"
      while (GDline.sim_time < (TDline.sim_time-trig_latency-(int(TimeRes*1.0e+06)))) {
        word ++;
        if(word ==1)getPointerPosition2 = outFile->tellg();
        if (!ReadGDline(GDline)){
          if(verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"WARNING(parser): Gen Data Event NOT found, could not read gen data file while "
                                                            <<"reaching proper simulation time to investigate for selected trigger (requested time was: "
                                                            <<TDline.sim_time-trig_latency-1e+6
                                                            <<")"<<endl;
          return true;
        }
      }

      //if file pointer was already at the sim_time region expected go back 1 line (problems with consecutive triggers)
      //if (word==1 ){
      outFile->seekg((getPointerPosition2-outFile->tellg()),ios::cur);
      //}

     	GDline.sim_time = 0;
      while (GDline.sim_time <= TDline.sim_time-trig_latency) {
	  
        if (!ReadGDline(GDline)){
          if (verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"WARNING(parser): Gen Data Event NOT found, could not read buf data while " 
                                                             <<"looking for TS "
                                                             <<trig<<" @ sim time:"
                                                             <<GDline.sim_time
                                                             <<" TDline.TS "
                                                             <<TDline.TS
                                                             <<" BClat "
                                                             <<BClat
                                                             <<endl;
          return true;
        }
        if (verbosity==kDEBUG) cout<<"INFO(parser): Word: "<<word<<" GDline.sim_time: "  <<GDline.sim_time
      
                                   << " "<< TDline.sim_time-trig_latency-(int(TimeRes*1.0e+06))
                                   << " "<< TDline.sim_time-trig_latency<<endl;
								 
								 
        if (GDline.sim_time > TDline.sim_time-trig_latency-(int(TimeRes*1.0e+06))  && GDline.sim_time <= TDline.sim_time-trig_latency){
          readHit.X_address = GDline.x;
          readHit.Y_address = GDline.y;
          evt.hits.push_back(readHit);
          newHitFound = true;
          getPointerPosition = outFile->tellg();
        } else  if (newHitFound) newEventFound = true;                 
      }
	
      //if hits for this trigger are found: restore the file read pointer to the last triggered hit
      //if hits fro this trigger are NOT found: restore the file read pointer the initial position
      outFile->seekg((getPointerPosition-outFile->tellg()),ios::cur);			
      //print info on the found event
      if(verbosity==kDEBUG && newHitFound) cout<<"INFO(parser): Triggered hits found in buffer data at sim time "<<GDline.sim_time<<endl; 	  
      return true;
      break;
   


        // extract trigger to investigate
/*         if(!trigFile->eof()){ */
/*           read_ok= *trigFile>>trig; */
/*           if (read_ok){ */
/*             if(verbosity==kDEBUG) cout<<"trig "<<trig<<endl; */
/*             *trigFile>>TS; */
/*             if(verbosity==kDEBUG) cout<<"TS "<<TS<<endl; */
/*             *trigFile>>trig_time; */
/*             if(verbosity==kDEBUG) cout<<"trig_time "<<trig_time<<endl; */
/*             *trigFile>>trig_res; */
/*             if(verbosity==kDEBUG) cout<<"unit "<<trig_res<<endl; */
/*                newEventFound = true; */
/*           } else newEventFound = false; */
/*         } else if(verbosity==kMEDIUM || verbosity==kDEBUG) cout<<"WARNING(parser): trig file reached the end"<<endl; */
        
	//set the triggered time stamp to event
	//evt.TimeStamp = TS;
        //????
	//if (trig==1) triggered = true; else triggered = false;
        
/*         while(!eoe){ */
/*           if(!outFile->eof()){ */
/*             getPointerPosition = outFile->tellg();	 */
/*             read_ok = *outFile>>idx_out;									 */
/*             if (read_ok){ */
/*               *outFile>>readHit.X_address; */
/*               if(verbosity==kDEBUG) cout<<"X "<<readHit.X_address<<endl; */
/*               *outFile>>readHit.Y_address; */
/*               if(verbosity==kDEBUG) cout<<"Y "<<readHit.Y_address<<endl; */
/*               *outFile>>sim_time; */
/*               if(verbosity==kDEBUG) cout<<"sim time "<<sim_time<<endl; */
/*               *outFile>>t_res; */
/*               if(verbosity==kDEBUG) cout<<"unit "<<t_res<<endl;  */
              
              
/* //		if (sim_time < trig_time-trig_latency+24499){ */
/* 		if (sim_time < trig_time-trig_latency){ */
/*                 evt.hits.push_back(readHit); */
/*                 if (verbosity==kDEBUG)cout<<"Including Hit: "<<readHit.X_address<<" "<<readHit.Y_address<<" in the event"<<endl;				 */
/*               } else { */
/*                 eoe = true; */
/*                 outFile->seekg((getPointerPosition-outFile->tellg()),ios::cur);		 */
/*                 //evt.TimeStamp = TS; */
/*               } */
/*             } else { */
/* 	      eoe=true; */
/*             } */
/*           } else { */
/* 	    //evt.TimeStamp = TS; */
/*             eoe = true; */
/*             //if(verbosity==kMEDIUM || verbosity==kDEBUG) cout<<"WARNING(parser): gen data file reached the end"<<endl; */
/*           } */
/*         } */
/*         return newEventFound; */
       
        
        // DAQ layer dump data (data extracted from DAQ data with "dump" program)    
      case DAQLayerDump:
        if(verbosity==kDEBUG) cout<<"Looking for a DAQLayerDump Event"<<endl; 
        if(!outFile->eof()){
          read_ok = *outFile>>hex>>raw>>dec;									
          if (read_ok){
            newEventFound=true;
            N_word = ((raw &0x0000ff00) >> 8);
            if(verbosity==kDEBUG) cout<<"N_word found "<<N_word<<endl; 	  
            for (int i = 0; i< N_word; i++){
              *outFile>>hex>>raw>>dec;
              //la prima hit/EoE che sta nel blocco setta il time stamp dell'evento
              if (i == 0) evt.TimeStamp = DAQLayerDumpGetTS(raw);
              //se e' un EoE non scrivo nessuna hit
              if (DAQLayerDumpIsEoE(raw)){
                if(verbosity==kDEBUG) cout<<"EoE:   TS = "<<DAQLayerDumpGetTS(raw)<<" Last Dac Set "<<DAQLayerDumpGetDAC(raw)<<endl; 
              }else {
                readHit.X_address = DAQLayerDumpGetX(raw);
                readHit.Y_address = DAQLayerDumpGetY(raw);
                evt.hits.push_back(readHit);
                if(verbosity==kDEBUG) cout<<"TS "<<evt.TimeStamp<<" X= "<<readHit.X_address<<" Y= "<<readHit.Y_address<<endl; 
              }
            }
          }
        }
        return newEventFound;
        break;
        
      case StripFEBuffer:
         BClat = getBClat();
         getPointerPosition = outFile->tellg();
         getPointerPosition2 = getPointerPosition;
         TDline.trig=0;
          //get the next triggered TS,
          while (!TDline.trig) {
            if(!ReadTDline(TDline)) {
              if (verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"WARNING(parser): Strip Buffer Event NOT found, could not read a new trigger"<<endl;
              return false;
            }
          }
          if(verbosity==kDEBUG) cout<<"INFO(parser): Trigger Found! TS = "<<TDline.TS<<" (0x"<<hex<<TDline.TS<<dec<<") "
                                                                 <<" sim time "<<TDline.sim_time
                                                                 <<" "<<TDline.tres
                                                                 <<endl;

          TDline.TS-int(BClat)>=0 ? trig = TDline.TS-int(BClat) : trig = TDline.TS-int(BClat)+256; //triggered TS = TDline.TS-BClat taking into 
          evt.TimeStamp = trig;                                                     //account time counter modulo (assumed 8 bit TS!)


          //reach the buffer_data sim_time region where triggered hits were stored (with 1 us of time margin)
          BDline.sim_time = 0;
	  word = 0; //variable re-use. proper name would be "line"
          while (BDline.sim_time<=(TDline.sim_time-trig_latency-1e+6)) {
            word ++;
	    if(word ==1)getPointerPosition2 = outFile->tellg(); 
	    if (!ReadBDline(BDline)){
              if(verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"WARNING(parser): Strip Buffer Event NOT found, could not read buf data file while " 
								     <<"reaching proper simulation time to investigate for selected trigger (requested time was: "
								     <<TDline.sim_time-trig_latency-1e+6
								     <<")"<<endl; 
              return true;
            }
          }
          
	  //if file pointer was already at the sim_time region expected go back 1 line (problems with consecutive triggers) 
	  if (word==1){
	    outFile->seekg((getPointerPosition2-outFile->tellg()),ios::cur);	
	  }
          
	  //look for the hits with the triggered Time Stamp
          BDline.TS = -1;
          readHit.Y_address = 0;
          
	  while (BDline.sim_time <= TDline.sim_time && !newEventFound) {
            if (!ReadBDline(BDline)){
             if (verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"WARNING(parser): Strip Buffer Event NOT found, could not read buf data while " 
								     <<"looking for TS "
								     <<trig<<" @ sim time:"
								     <<BDline.sim_time
								     <<" TDline.TS "
								     <<TDline.TS
								     <<" BClat "
								     <<BClat
								     <<endl;
              return true;
            }
	    if (BDline.TS == trig){
              readHit.X_address = BDline.strip;
              evt.hits.push_back(readHit);
              //evt.TimeStamp = BDline.TS;
              newHitFound = true;
              getPointerPosition = outFile->tellg();
            } else  if (newHitFound) newEventFound = true;                 
          }
	  
          //if hits for this trigger are found: restore the file read pointer to the last triggered hit
          //if hits fro this trigger are NOT found: restore the file read pointer the initial position
          outFile->seekg((getPointerPosition-outFile->tellg()),ios::cur);			
	  //print info on the found event
	  if(verbosity==kDEBUG && newHitFound) cout<<"INFO(parser): Triggered hits found in buffer data at sim time "<<BDline.sim_time<<endl; 	  
	  return true;
        
        
        
        /*if (!TrigfileIsAttached()) {
          if(verbosity==kMEDIUM || verbosity==kDEBUG) cout<<"ERROR(parser): Trigger File is not attached when finding events"<<endl;
          return false;
        }
        cout<<"Looking for a Strip Buffer  Event"<<endl; 
  			while(!eoe){      
          if(!outFile->eof()){
            getPointerPosition = outFile->tellg();
            read_ok = *outFile>>idx_out;
            if (read_ok){
              readHit.X_address = 0;
              *outFile>>readHit.Y_address;
              *outFile>>hex>>TS>>dec;
              *outFile>>inBufferTime;
              *outFile>>t_res;
              if(verbosity==kDEBUG) cout<<"  TS ="<<TS<<" Strip = "<<readHit.Y_address<<" stream fail= "<<outFile->fail()<<endl; 
              if (firstRead){
                curTS = TS;
                firstRead = false;
              }
              if (TS != curTS){
                evt.TimeStamp = from_gray_conv(curTS);
                newEventFound = true;
                eoe = true;
                //roporto il cursore indietro
                if (verbosity==kDEBUG)cout<<" "<<getPointerPosition<<" "<<outFile->tellg()<<" new header found, moving back getPointer "<<endl;
                outFile->seekg((getPointerPosition-outFile->tellg()),ios::cur);		 		
              } else {
                evt.hits.push_back(readHit);
              }
            }
          } else eoe = true;
        }  
        return newEventFound;*/
        break;
        
      default:
        cout<<"invalid data type set"<<endl;
        return false;
    }
  }else{ 
    return false;
    cout<<"Can't find event. File not attached"<<endl;
  }
  return false;
}

bool parser::fillMatrixData(matrixData *m_data){
  _EVENT myevent;
  myevent.Reset();
  m_data->Init();
  int deltaTS;
  int i = 0;	

  //set the matrix size (which depends on data type) in the matrixData structure.
  fillInMatrixSize(m_data);

  if (verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"INFO(parser): Filling Matrix Data data file"<<endl;
  
  while(findEvent(myevent)) {
    //SET THE EVENT ID
    myevent.idx = i;		 
		
    //SET THE EVENT ABS_TIME (long TS)
    if (myevent.TimeStamp < lastTS){
      deltaTS = 256-(lastTS-myevent.TimeStamp);
    }	else {
      deltaTS = myevent.TimeStamp-lastTS;
    }
    if (deltaTS>255) cout<<"WARNING(parser): deltaTS > 255 while filling matrix data"<<endl;
    //update the parser member: lastTS 
    lastTS = myevent.TimeStamp;		
    //update the parser member: Time
    Time += deltaTS;
    //set the event absolute time
    myevent.AbsTime=Time;						
    i++;
    //update the parser member: hits_found
    hits_found += myevent.hits.size();
		
    //ADD THE EVENT TO MATRIXDATA
    m_data->AddEvent(myevent);	
    if (verbosity==kDEBUG) cout<<"INFO(parser): added Event"<<endl;
    if (verbosity==kDEBUG) myevent.Print();	
		
    
  }  
  if(i==0){
    if (verbosity == kMEDIUM || verbosity==kDEBUG) cout<<"WARNING(parser): matrixData is empty"<<endl;
    return false;
  }else return true;
}

bool parser::ReadTDline(_TRIG_DATA_LINE &TDline){
  if (!TrigfileIsAttached()) {
    cout<<"ERROR(parser): Trigger File is not attached when reading a line"<<endl;
    return false;
  }  
  if(!trigFile->eof()){
    *trigFile>>TDline.trig;
    *trigFile>>TDline.TS;
    *trigFile>>TDline.sim_time;
    *trigFile>>TDline.tres;
  } else return false;
  
  //check if a line is corrupted
  if (trigFile->good())return true;
  else {
    //not good due to anything but end of file.
    if(!trigFile->eof())cout<<"ERROR(parser): Error while reading trig data line. trig = "<<TDline.trig
				 <<" TS = "<<TDline.TS
				 <<" sim_time = "<<TDline.sim_time
				 <<" "<<TDline.tres
				 <<endl;
    //end of file crossed
    else cout<<"WARNING(parser): Tried to read trig data line after eof "<<endl;

    return false;
  }
}

bool parser::ReadBDline(_BUF_DATA_LINE &BDline){
  int TS = 0;
  if (!OutputfileIsAttached()) {
    cout<<"ERROR(parser): Buf data file is not attached when reading a line"<<endl;
    return false;
  }  
  if(!outFile->eof()){
    *outFile>>BDline.idx;
    *outFile>>BDline.strip;
    *outFile>>hex>>TS>>dec; //read the gray-coded hex value.
    BDline.TS = from_gray_conv(TS); 
    *outFile>>BDline.sim_time;
    *outFile>>BDline.tres;
  } else return false;
  
  //check if a line is corrupted
  if (outFile->good())return true;
  else {
    //not good due to anything but end of file.
    if(!outFile->eof())cout<<"ERROR(parser): Error while reading buf data line. idx = "<<BDline.idx
				<<" strip = "<<BDline.strip
				<<" TS = "<<BDline.TS
				<<" sim_time = "<<BDline.sim_time
				<<" "<<BDline.tres
				<<endl;
    //end of file crossed
    else cout<<"WARNING(parser): Tried to read buf data line after eof "<<endl;
    return false;
  }
}

bool parser::ReadGDline(_GEN_DATA_LINE &GDline){
  //int TS = 0;
  if (!OutputfileIsAttached()) {
    cout<<"ERROR(parser): Buf data file is not attached when reading a line"<<endl;
    return false;
  }  
  if(!outFile->eof()){
    *outFile>>GDline.idx;
    *outFile>>GDline.x;
    *outFile>>GDline.y;
    *outFile>>GDline.sim_time;
    *outFile>>GDline.tres;
  } else return false;
  
  //check if a line is corrupted
  if (outFile->good())return true;
  else {
    //not good due to anything but end of file.
    if(!outFile->eof())cout<<"ERROR(parser): Error while reading gne data line. idx = "<<GDline.idx
				<<" x = "<<GDline.x
				<<" y = "<<GDline.y
				<<" sim_time = "<<GDline.sim_time
				<<" "<<GDline.tres
				<<endl;
    //end of file crossed
    else cout<<"WARNING(parser): Tried to read gen data line after eof "<<endl;
    return false;
  }
}

bool parser::setChipProperties(_DATATYPE type){
  return getChipProperties(type, chipProperties);
}


/*For certain data types the chip properties can be individuated and set automatically*/
bool parser::getChipProperties(_DATATYPE type, _CHIP_PROPERTIES & chipProp){
  bool good = false;

  if (type == simuSORTEX200x256){
      chipProp.chipName       = "finalSORTEX";
      chipProp.chipID         = 1;
      chipProp.submat_X_size  = 50;
      chipProp.submat_Y_size  = 256;
      chipProp.NX_submat      = 4;
      chipProp.NY_submat      = 1;
      good = true;
  }else if (type == simuSQARE200x256){
      chipProp.chipName       = "finalSQARE";
      chipProp.chipID         = 2;
      chipProp.submat_X_size  = 50;
      chipProp.submat_Y_size  = 256;
      chipProp.NX_submat      = 4;
      chipProp.NY_submat      = 1;
      good = true;
  }else if (type == FE4DtestPISA ){
      chipProp.chipName       = "SUPERPIX0";
      chipProp.chipID         = 0;
      chipProp.submat_X_size  = 64;
      chipProp.submat_Y_size  = 32;
      chipProp.NX_submat      = 2;
      chipProp.NY_submat      = 1;
      good = true;
  }else if (type == APSEL4DmicroBOLOGNA){
      chipProp.chipName       = "APSEL4D";
      chipProp.chipID         = 4;
      chipProp.submat_X_size  = 128;
      chipProp.submat_Y_size  = 32;
      chipProp.NX_submat      = 1;
      chipProp.NY_submat      = 1;
      good = true;

  }else if (type == simuSQARE2x16x32){
      chipProp.chipName       = "INMAPS";
      chipProp.chipID         = 5;
      chipProp.submat_X_size  = 16;
      chipProp.submat_Y_size  = 32;
      chipProp.NX_submat      = 2;
      chipProp.NY_submat      = 1;
      good = true;

  }else if (type == simuGenList ){
    cout<<"WARNING(parser): Parser could not retrieve chip properties automatically from type simuGenList. Please remember to specify them manually."<<endl;
  }else if (type == DAQLayerDump){
    cout<<"WARNING(parser): Parser could not retrieve chip properties automatically from type DAQLayerDump. Please remember to specify them manually."<<endl;
  }else if (type == StripFEBuffer){
      chipProp.chipName       = "StripChip";
      chipProp.chipID         = 10;
      chipProp.submat_X_size  = 1;
      chipProp.submat_Y_size  = 64;
      chipProp.NX_submat      = 1;
      chipProp.NY_submat      = 4;
      good = true;

  }else if (type ==simuSQARE2x48x128 || type ==simuSQARE2x48x128raw){
      chipProp.chipName       = "APSELVI";
      chipProp.chipID         = 6;
      chipProp.submat_X_size  = 48;
      chipProp.submat_Y_size  = 128;
      chipProp.NX_submat      = 2;
      chipProp.NY_submat      = 1;
      good = true;

  }else if (type ==simuSQARE2x16x128 || type == simuSQARE2x16x128raw ){
      chipProp.chipName       = "SUPERPIX1";
      chipProp.chipID         = 7;
      chipProp.submat_X_size  = 16;
      chipProp.submat_Y_size  = 128;
      chipProp.NX_submat      = 2;
      chipProp.NY_submat      = 1;
      good = true;

  }

  if (good){
    chipProp.mat_X_size     = chipProp.NX_submat*chipProp.submat_X_size;
    chipProp.mat_Y_size     = chipProp.NY_submat*chipProp.submat_Y_size;
    return true;
  }else return false;

}


bool parser::setChipProperties(string chipName, uint32_t chipID, uint32_t submat_X_size, uint32_t submat_Y_size, uint32_t NX_submat, uint32_t NY_submat){
  chipProperties.chipName       = chipName;
  chipProperties.chipID         = chipID;
  chipProperties.submat_X_size  = submat_X_size;
  chipProperties.submat_Y_size  = submat_Y_size;
  chipProperties.NX_submat      = NX_submat;
  chipProperties.NY_submat      = NY_submat;

  chipProperties.mat_X_size     = chipProperties.NX_submat*chipProperties.submat_X_size;
  chipProperties.mat_Y_size     = chipProperties.NY_submat*chipProperties.submat_Y_size;

  return true;
}

int parser::GetIdx(){
  return idx;
}

int parser::GetTime(){
  return Time;
}

int parser::GetHitsFound(){
  return hits_found;
}

int parser::GetLastTS(){
  return lastTS;
}

_DATATYPE parser::getDataTypeName(int n, char * text)
{
    switch(n)
    {
        case (APSEL4DmicroBOLOGNA):
        if (text!=0) sprintf(text,"APSEL4DmicroBOLOGNA");
        return APSEL4DmicroBOLOGNA;

        case (DAQLayerDump):
        if (text!=0) sprintf(text,"DAQLayerDump");
        return DAQLayerDump;

        case (FE4DtestPISA):
        if (text!=0) sprintf(text,"FE4DtestPISA");
        return FE4DtestPISA;

        case (simuGenList):
        if (text!=0) sprintf(text,"simuGenList");
        return simuGenList;

        case (simuSORTEX200x256):
        if (text!=0) sprintf(text,"simuSORTEX200x256");
        return simuSORTEX200x256;

        case (simuSQARE200x256):
        if (text!=0) sprintf(text, "simuSQUARE200x256");
        return simuSQARE200x256;

        case (simuSQARE2x16x32):
        if (text!=0) sprintf(text, "simuSQUARE2x16x32");
        return simuSQARE2x16x32;

        case (StripFEBuffer):
        if (text!=0) sprintf(text, "StripFEBuffer");
        return StripFEBuffer;

        case (simuSQARE2x48x128):
        if (text!=0) sprintf(text, "simuSQARE2x48x128");
        return simuSQARE2x48x128;

        case (simuSQARE2x48x128raw):
        if (text!=0) sprintf(text, "simuSQARE2x48x128raw");
        return simuSQARE2x48x128raw;

        case (simuSQARE2x16x128):
        if (text!=0) sprintf(text, "simuSQARE2x16x128");
        return simuSQARE2x16x128;

        case (simuSQARE2x16x128raw):
        if (text!=0) sprintf(text, "simuSQARE2x16x128raw");
        return simuSQARE2x16x128raw;

        default:
        if (text!=0) sprintf(text,"INVALID_DataType");
        return INVALID_DataType;

    }
}

_VERBOSITY parser::getVerbosityLevelName(int n, char * text)
{
    switch(n)
    {
        case (kSILENT):
        if (text!=0) sprintf(text,"kSILENT");
        return kSILENT;
        case (kMEDIUM):
        if (text!=0) sprintf(text,"kMEDIUM");
        return kMEDIUM;
        case (kDEBUG):
        if (text!=0) sprintf(text,"kDEBUG");
        return kDEBUG;
        default:
        if (text!=0) sprintf(text,"INVALID_VerbosityLevel");
        return INVALID_VerbosityLevel;

    }
}

void parser::fillInMatrixSize(matrixData *mdata)
{
    mdata->SetMatrixSize(chipProperties.mat_X_size,chipProperties.mat_Y_size);
}

uint32_t  parser::getMatrixWidth(_DATATYPE type){
    _CHIP_PROPERTIES cp;
    getChipProperties(type,cp);
    return cp.mat_X_size;
}

uint32_t  parser::getMatrixHeight(_DATATYPE type){
    _CHIP_PROPERTIES cp;
    getChipProperties(type,cp);
    return cp.mat_Y_size;
}

uint32_t  parser::getMatrixWidth(){
    return getMatrixWidth(dataType);
}

uint32_t  parser::getMatrixHeight(){
    return getMatrixHeight(dataType);
}



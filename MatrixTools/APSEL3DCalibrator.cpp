//INMAPS Calibrator.
// usage:
// ./INMAPSCalibrator datafileRNXXX.dat
// Retrieves RN from standard file name and gets calibration parameters from the SQL database tdaq on pcslim.bo.infn.it
// If more than one layers are active for calibration it takes the first from list:
//  EPMC1-FEM0
//  EPMC1-FEM1
//  EPMC2-FEM0
//  EPMC2-FEM1

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <deque>
#include <vector>
#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TKey.h>
#include "bit_operations.h"
#include "Slim5OutputParser.h"

#ifndef __CINT__
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TStopwatch.h>
#endif

//struct CalibParam {
//  int triggers;
//  int dacstart, dacstop, dacstep;
//  int delay;
//  int layers;
//  int ntype;
//};

typedef struct _FEMCalibParam{
  unsigned int dacstart;
  unsigned int dacstop;
  unsigned int dacstep;    
  unsigned int triggers;   //number of trigger sent (BC) for each cal step
  unsigned int time_window; //length of the acquisition window in Rdclk periods;
  unsigned int BC_wait;  //length of data awaiting after the acquisition window in BC clocks
  void Reset();
} FEMCalibParam;

void FEMCalibParam::Reset(){
  dacstart = 0;
  dacstop = 0;
  dacstep = 0;
  triggers = 0;
  time_window = 0;
  BC_wait = 0;
}

typedef struct _EPMCCalibParam{
  unsigned int layers;    //8 bit active layers;
  unsigned char type0;    //0 = SPX0, 3 = APSEL3D, 4=APSEL4D, 5= INMAPS32x32
  unsigned char type1;    //0 = SPX0, 3 = APSEL3D, 4=APSEL4D, 5= INMAPS32x32
  FEMCalibParam fem0_cp;
  FEMCalibParam fem1_cp;
  void Reset();
} EPMCCalibParam;

void EPMCCalibParam::Reset(){
  layers = 0;
  type0  = 255;
  type1  = 255;
  fem0_cp.Reset();
  fem1_cp.Reset();
}

struct CalibIndex {
  int trigidx;
  int colidx;
  int dacidx;
};

struct EOSdata{
  unsigned int TS;
  unsigned int parity_bit;
  unsigned int chip_n;
  unsigned int DACvalue;
  int parity;
};

struct CalibTrigData{
  bool MissCalTrigFlag;
  bool MissCalSoSFlag;
  unsigned int TrgCnt;
};

enum WarningFlags{
  IdenticalHitWarning = 1,
  MissedSoSWarning = 2,
  MissedCalibTrigWarning = 3
};


vector<TH1I*> calibpxl;
vector<TH1F*> occuppxl,occupmp;

TH2I *hevts,*hmatrix,*hmatrixcurr;
TH1I *hbco1,*hbco2, *DACstep_BCO;
TH2I *doubleHits;
TH1I *hitBit, *chipHitBit;
TH1I *hevtMult, *hcolMult;
TH1I *hitunsanitizable; 
TH2I *hcolsOut, *hbitswapsinCol;
TH1I *hbitWarning;

int BookHist(std::string base, const FEMCalibParam & calpar, unsigned  int layers);
int FillHist(_EVENT & evt, const FEMCalibParam & calpar, unsigned int layers, _VERBOSITY  verbosity);
int new_hMatrix(int DACidx);
//CalibIndex FindSteps(int exactbco,const CalibParam & calpar);
void NormalizeTo(const FEMCalibParam & calpar);    
bool sqlselect(int nrun,EPMCCalibParam & epmc1,EPMCCalibParam & epmc2 );
bool validCol(int col, int colstep);
bool SQLmode = true;
bool purify = false;
bool sanitize = false;
int run_number;
int dummy;
// Storage of Calib parameters
EPMCCalibParam EPMCcalpar1,EPMCcalpar2;
FEMCalibParam FEMcalpar;


unsigned long triggers=0;
unsigned long DACstart=0;
unsigned long DACstop=0;
unsigned long DACstep=0;
unsigned long Layers=0;
unsigned long nType=0;
unsigned int ROB = 0;
//unsigned int Nevents = 0;

const unsigned int col_steps = 16;

  
unsigned long evt_n       = 0; //number of events found
unsigned long corrupt_evt = 0; // number of corrupted event found  


//const float dac_to_mV = 1250./4096;
	
int main(int argc, char* argv[]){
  if (argc <= 1 ){
    cout << "Usage: A3DCalibrator [-v0|1|2] [-evtmax|-e n_of_events] [-rob 1|0] [-purify | -sanitize] [-local triggers DACstart DACstop DACstep Layers nType=3)] <eventfile>" << endl;     
    return -1;
  }
  cout<<"+++++++++++++++A3D_TC Calibrator 0.1 by F. Giorgi+++++++++++++++++++++++++"<<endl;
  cout << "Usage: A3DCalibrator [-v0|1|2] [-evtmax|-e n_of_events] [-rob 1|0] [-purify | -sanitize] [-local triggers DACstart DACstop DACstep Layers nType=3] <eventfile>" << endl<<"INFO:: -purify skip hits that belongs to wrong columns w.r.t. current col-step"
       <<endl<<"INFO: sanitize try to fix wrong column addresses if a single-bit error is found w.r.t current col-step"<<endl;
  
  ///////////// Command line decoding //////////////
  unsigned long   maxevt                 = 0;  
  _VERBOSITY      verbosity		 = kSILENT;
 
  int index = 1;
  while((index < argc) && (argv[index][0]=='-')) {
    string arg = argv[index]; 
    if(argv[index][1] == 'v'){ //e.g. -v2 == debug
      // Set verbosity flag
      switch(argv[index][2]){
      case '0': verbosity = kSILENT;
	break;
      case '1': verbosity = kMEDIUM;
	break;
      case '2': verbosity = kDEBUG;
	break;
      default: verbosity = kDEBUG;
	break;
      } //end switch   
      if(verbosity != 0)  cout << "Verbosity level: " << verbosity << endl;     
    } else if( (arg == "-evtmax") || (arg == "-e") ){
      index++;
      maxevt = strtoul(argv[index], NULL, 0);
      cout << "Event max set to " << maxevt << endl;
    } else if( (arg == "-rob") || (arg == "-e") ){
      index++;
      ROB = strtoul(argv[index], NULL, 0);
      cout << "ROB set to " << ROB << endl;
    } else if( arg == "-purify" ){
      if (sanitize) {
	cout<<"ERROR: Cannot activate both purify and sanitize options together"<<endl;
	return -1;
      }
      purify = true;
      cout << "Purify option active " << endl;
    } else if( arg == "-sanitize" ){
      if (purify) {
	cout<<"ERROR: Cannot activate both purify and sanitize options together"<<endl;
	return -1;
      }
      sanitize = true;
      cout << "Sanitize option active " << endl;
    } else if( arg == "-local" ){    
      SQLmode = false; 
      cout << "SQL disabled " << maxevt << endl;
      index++;
      triggers = strtoul(argv[index], NULL, 0);
      if (triggers == 0){cout<<"Bad -local option: triggers"<<endl; return -1;}
      index++;
      DACstart = strtoul(argv[index], NULL, 0);
      if (DACstart == 0){cout<<"Bad -local option: DACstart"<<endl; return -1;}
      index++;
      DACstop = strtoul(argv[index], NULL, 0);
      if (DACstop == 0) {cout<<"Bad -local option: DACstop"<<endl;return -1;}
      index++;
      DACstep = strtoul(argv[index], NULL, 0);
      if (DACstep == 0){cout<<"Bad -local option: DACstep"<<endl; return -1;}
      index++;
      Layers = strtoul(argv[index], NULL, 0);
      if (Layers == 0) {cout<<"Bad -local option: Layers (1 active layer at least: 0x1 layer zero active, 0x3 layer 0 and 1 active... )"<<endl;return -1;}
      index++;
      nType = strtoul(argv[index], NULL, 0);
      if (nType != 3 ){cout<<"Bad -local option: nType must be 3 (EPMC_PIXEL)"<<endl; return -1;}

    }else cout << "Unknown switch:" << arg << endl; 
    index++;
  }//end while
  
  // if index = argc it means we didn't supply only the flags but not the
  // filename
  if (argc == index ){
    cout << "Error: You didn't supply a filename" << endl;
    cout << "Usage: A3DCalibrator [-v0|1|2] [-evtmax|-e n_of_events] [-rob 1|0] [-purify | -sanitize] [-local triggers DACstart DACstop DACstep Layers nType=3] <eventfile>" << endl;
    return -1;
  }
  
  
  
  {  // ! note the local block 
    // Open data file to get the run number
    ifstream filetmp;
    filetmp.exceptions( ios::badbit | ios::failbit );
    try{ 
      filetmp.open(argv[index], ios::in | ios::binary);
      cout << "File " << argv[index] << " opened" << endl;
    } catch (std::ios_base::failure& e) {
      cout << "Can't Open File " << argv[index]
	   <<" index="<<index<<" argc="<<argc<<endl;
      return -1; 
    } catch(std::exception& e) {
      cout << "Exception: " << e.what();
      cout << "Can\'t open file due to stdlib exception." << endl;       
      return -1; 
    }
    
    Slim5OutputParser parsertmp(filetmp);
    parsertmp.setVerbosity(verbosity);
    bool another1 = false;
    cout<<"now I try to parse"<<endl;

    // discard all the initial corrupted events and move to the first good event
    while (!another1){
      try{
	another1 = parsertmp.FindEvent(); 
      } catch(std::exception& e){
	cout << "\t" <<  e.what() << endl << endl;
	//another1 = parsertmp.IsEndOfFile();
      }
    }
    run_number = parsertmp.GetEvent().run_number;
  } // end of local block
  
  cout<< "run number= "<< run_number<<endl;
  
  ///////////////////////////
  //return 0;
  //////////////////////////


  if (SQLmode){
    EPMCcalpar1.Reset();
    EPMCcalpar2.Reset();
    sqlselect(run_number, EPMCcalpar1, EPMCcalpar2);
    if(      EPMCcalpar1.type0==0x3 && EPMCcalpar1.layers==0x1){
      FEMcalpar = EPMCcalpar1.fem0_cp;
      Layers = EPMCcalpar1.layers;
    } else if (EPMCcalpar1.type1==0x3 && EPMCcalpar1.layers==0x2){
      FEMcalpar = EPMCcalpar1.fem1_cp;
      Layers = EPMCcalpar2.layers;
    } else if (EPMCcalpar2.type0==0x3 && EPMCcalpar2.layers==0x10){
      FEMcalpar = EPMCcalpar2.fem0_cp;
      Layers = EPMCcalpar2.layers;
    } else if (EPMCcalpar2.type1==0x3 && EPMCcalpar2.layers==0x20){
      FEMcalpar = EPMCcalpar2.fem1_cp;
      Layers = EPMCcalpar2.layers;
    } else {
      cout<<"ERROR! SQL returned no active A3D pixel layer "<<endl;
      return -1;
    }
    cout<<"Check the parameters found and type any character key and press return"<<endl;
    cin>>dummy;
  }else{
    FEMcalpar.triggers = triggers;
    FEMcalpar.dacstart = DACstart; 
    FEMcalpar.dacstop  = DACstop;
    FEMcalpar.dacstep  = DACstep;
  }
  
  // 
  // Root file opening
  //
  char rootfile[200];
  if(purify)
    snprintf(rootfile,199,"HistosINMAPSCalib_%07d_purified.root",run_number);
  else if (sanitize)
    snprintf(rootfile,199,"HistosINMAPSCalib_%07d_sanitized.root",run_number);
  else
    snprintf(rootfile,199,"HistosINMAPSCalib_%07d.root",run_number);
  TFile *f=TFile::Open(rootfile,"recreate","File di platano");  
  // Open the output root file
  if( f==0 ){
    cout << "Can\'t open output file " << rootfile << endl;
    exit(1);
  }

  //////////////////////////BOOKING HISTOGRAMS
  BookHist("Master",FEMcalpar,Layers);
  ////////////////////////.
  if(verbosity==kDEBUG||verbosity==kMEDIUM) cout<<"Histrograms are booked"<<endl;
  
  
  // data file opening	
  while( index<argc){
    //last arguments = files to open
    ifstream file;
    file.exceptions( ios::badbit | ios::failbit );
    try{
      file.open(argv[index], ios::in | ios::binary);
      cout << "File " << argv[index] << " opened" << endl;
    }
    catch (std::ios_base::failure& e) {
      cout << "Can't Open File" << endl;
    }
    catch(std::exception& e) {
      cout << "Exception: " << e.what();
      cout << "Can\'t open file due to stdlib exception." << endl;       
      return -1; 
    }
    //   catch(Slim5OutputParser::errImpossibleNewEvent e){
    //  cout << "Exception: " << e.what();
    //  cout << "Corrupted dataFile." << endl;       
    //  return -1; 
    // }    


if(verbosity==kDEBUG||verbosity==kMEDIUM) cout<<"Root data file opened"<<endl;
    


    /////////////// Search variables /////////////////
    
    Slim5OutputParser parser(file);
    parser.setVerbosity(verbosity);

    bool another1 = true;
    while( another1 ){      
      try{
	another1 = parser.FindEvent();
	evt_n++;
      }
      catch(std::exception& e){
	corrupt_evt++;
	cout << "\t" <<  e.what() << endl << endl;
	another1 = !parser.IsEndOfFile();
	continue;
      }
      _EVENT evt = parser.GetEvent();
      evt.ordinal = evt_n;
      if(verbosity==kDEBUG||verbosity==kMEDIUM) cout<<"---------new event "<<endl;
      if((FillHist(evt, FEMcalpar, Layers, verbosity))!=0)return -1; //FILLING THE HISTOGRAMS
      
	
      //if( evt_n<10 || 
      //  (evt_n<100 && (evt_n%10)==0) || 
      //  (evt_n<1000 && (evt_n%100)==0) || 
      //  (evt_n%1000)==0)
      //cout << "Event " << evt.ordinal << " length = " << evt.length 
      //     << endl;
	
      if(evt_n == maxevt) another1 = false;
    }
    index++;  // take next file
  }//end cycle on declared data files
  
  NormalizeTo(FEMcalpar);
  
  // Final statistics
  unsigned int expected_evt = FEMcalpar.triggers*col_steps*(FEMcalpar.dacstop - FEMcalpar.dacstart)/FEMcalpar.dacstep ;


  cout << evt_n             <<"\t\t GOOD events"<<endl;
  cout << corrupt_evt       <<"\t\t BAD  events"<<endl;
  cout <<"==========="<<endl;
  cout << evt_n+corrupt_evt <<"\t\t TOT  event fragments"<<endl<<endl;

  cout << expected_evt      <<"\t\t Expected  events"<<endl;
  
//
  f->Write();
  f->Close();
  
  //
  cout << "Output written to file "<<rootfile << endl;
  return 0;
}//END MAIN	





inline unsigned int regainChipHit(unsigned int raw){
  unsigned char px_row              = PIXEL::GetPxRow(raw);
  unsigned char px_col              = PIXEL::GetPxCol(raw);
  unsigned char pattern_bit_address = px_row%4;
  unsigned char zone_pattern        = 0x1<<pattern_bit_address;
  unsigned char zone_address        = px_row%2;
  unsigned char spars_address       = px_row/8;
  unsigned int ret = 0x0 | ((spars_address&0x3)<<9) | ((zone_address&0x1)<<8)  |  ((px_col&0xF)<<4)  | (zone_pattern&0xF);
  return ret;
}




/////////////////////FUNCTIONS////////////////////////////////////
int new_hMatrix(int DACidx){
  char name[40],title[80];
  snprintf(name,40,"hmatrix_%03d",DACidx);
  snprintf(title,80,"hmatrix_DACstep_%d",DACidx);
  hmatrixcurr = new TH2I(name,title,32,-0.5,31.5, 32,-0.5,31.5);
  return 0;
};


int FillHist(_EVENT & evt, const FEMCalibParam & calpar,unsigned int layers, _VERBOSITY verbosity){
  //STATIC VARIABLES
  static int prev_Nhit=-1;
  //indexes
  static struct CalibIndex CalIdx = {-1, -1, -1};
  // CalIdx.dacidx  = -1;
  // CalIdx.colidx  = -1;
  // CalIdx.trigidx = -1;

 
  static int prev_DACvalue=-1;
    //static int prev_higher_col=-1;
  static int prev_exactbco = 0;
  static unsigned  int trigger_counter=0;
 

  //LOCAL VARIABLES
  int hitMult = 0;
  int exactbco=0, dx=0;	
  int nhit=0;
  int raw=0;
  //hit data
  vector <int> pxl_ids;
  vector <int> col_ids;
  vector <int> row_ids;
  vector <int> debug_cols;

  EOSdata EOS_data;
  EOS_data.TS             =0;
  EOS_data.parity_bit     =0;
  EOS_data.chip_n         =0;
  EOS_data.DACvalue       =0;
  EOS_data.parity         =0;

  CalibTrigData trig_data; 
  trig_data.MissCalTrigFlag = false;
  trig_data.MissCalSoSFlag  = false;
  trig_data.TrgCnt          = 0;
  
  int column_ut=0; //column under test
  int edited_column_ut; //column under test with swapped bit
  int edited_pxl_id;
 

  unsigned int chipHit=0;
  unsigned int inspBit=0;
  int higher_col=0;
  int active_cols=0;

  if( evt.number_of_robs()>0 && evt.ROBs[ROB].rod.layers.size()>0 ){
    dx = (evt.ROBs[ROB].rod.BCO&31)-evt.ROBs[ROB].rod.timestamp;
    if(dx<0) dx +=32;
    exactbco= evt.ROBs[ROB].rod.BCO-dx;
    
    //Analyzing data
    // loop on all hits
    for(int rob=0; rob<evt.number_of_robs() ; ++rob){
      _ROB* myrob = &evt.ROBs[rob];
      if(evt.status==complete && exactbco>0 ){
	_ROD* myrod = &myrob->rod;
	for(unsigned int lay=0; lay<myrod->layers.size(); ++lay){
	  int mylay=lay+rob*8;
	  if( ((layers>>mylay)&1)==1 ){
	    nhit = myrod->layers[lay].number_of_hits;
	    if (verbosity==kDEBUG||verbosity==kMEDIUM) cout<<" nhits= "<<nhit<<endl;
	    
	    ///////////////////////HITS LOOP
	    for(int nh=1; nh<=nhit; nh++){
	      hitMult++;
	      //looking for identical hits in same event
	      for (int i=1; i<=nhit; i++){
		if (myrod->layers[lay].hits[nh].raw == myrod->layers[lay].hits[i].raw && i!=nh && !PIXEL::isDummyWord(myrod->layers[lay].hits[nh].raw)){
		  raw = myrod->layers[lay].hits[nh].raw;
		  doubleHits->Fill(PIXEL::GetPxCol(raw),PIXEL::GetPxRow(raw));
		  hbitWarning->Fill(IdenticalHitWarning);
		  if (verbosity == kDEBUG)cout<<"ERROR: Identical hits in event N: "<<evt.ordinal<<"  idx1 = "<<nh<<"   idx2 = "<<i<<endl;
		}
	      }
	      if(verbosity==kDEBUG||verbosity==kMEDIUM) cout<<"New hit found "<< nh <<endl;
	      raw=myrod->layers[lay].hits[nh].raw;

	      //Hit Bit histo filled here
	      chipHit = regainChipHit(raw);
	      for(int bit =0; bit<24; bit++){
		inspBit = (0x1<<bit);
		if(raw &inspBit) hitBit->Fill(bit);
		if(chipHit &inspBit) chipHitBit->Fill(bit); 
	      }

	      // FIRST HIT
	      if( nh == 1) {
		if (PIXEL::isEoE(raw) ){     //it is a trigger word    
		  if (verbosity==kDEBUG||verbosity==kMEDIUM) cout<<"        trigger found!"<<endl;
		  trigger_counter++;
		  prev_Nhit               = nhit;
		  EOS_data.TS             =PIXEL::GetHitTS(raw);
		  EOS_data.parity_bit     =PIXEL::GetParity(raw);
		  EOS_data.chip_n         =PIXEL::GetChipN(raw);
		  EOS_data.DACvalue       =PIXEL::GetDacStep(raw);
		  EOS_data.parity         =PIXEL::EvalParity(raw);
		} else {                     //se e' una hit
		  cout<<"ERROR:  Leading trigger word NOT found in event"<<endl; 
		  return -1;
		}
	      // SECOND HIT 
	      } else if (nh == 2){           // First dummy hit
		if (PIXEL::isDummyWord(raw)){
		  trig_data.MissCalTrigFlag = PIXEL::GetMissCalTrg(raw);
		  trig_data.TrgCnt          = PIXEL::GetTrgCnt(raw);		  
		}else{
		  cout<<"ERROR:  First hit is not a dummy word in event "<<endl; 
		  return -1;		
		}	
	      // THIRD HIT
	      } else if (nh == 3){            // Second dummy hit
		if (PIXEL::isDummyWord(raw)){
		  trig_data.MissCalSoSFlag  = PIXEL::GetMissCalSoS(raw);
		  trig_data.TrgCnt = trig_data.TrgCnt | (PIXEL::GetTrgCnt(raw)<<12);
		}else{
		  cout<<"ERROR:  Second hit is not a dummy word in event "<<endl; 
		  return -1;				
		}	
	      // ALL OTHER HITS
	      } else {
		if (PIXEL::isEoE(raw) ){
		  trigger_counter++;
		  cout<<"ERROR: hit "<< nh-1 <<" of Event "<<evt.ordinal<<" is a Trigger "<<endl;
		  return -1;
		}
		if (PIXEL::isDummyWord(raw)){
		   cout<<"ERROR: hit "<< nh-1 <<" of Event "<<evt.ordinal<<" is a Dummy Word "<<endl;
		  return -1;
		}
		pxl_ids.push_back(PIXEL::GetPxIdx(raw));
		col_ids.push_back(PIXEL::GetPxCol(raw));
		row_ids.push_back(PIXEL::GetPxRow(raw));
		if (col_ids.back() > higher_col){
		  higher_col = col_ids.back();
		  debug_cols.push_back(PIXEL::GetPxCol(raw));
		  active_cols++;
		}
	      }/// HIT NUMBER
	    }/////HITS LOOP

	    // set trigger idx  with counter value found in data
	    CalIdx.trigidx = trig_data.TrgCnt;

	    //set col idx
	    CalIdx.colidx = ((CalIdx.trigidx-1)/calpar.triggers)%col_steps; //triggers in data counts from 1 to N
	
	    // init checks
	    for (unsigned int i =0; i<debug_cols.size(); i++) {
	      column_ut = debug_cols.at(i);
	      //if (column_ut!= CalIdx.colidx*2 && column_ut!= CalIdx.colidx*2+1){
	      if (!validCol(column_ut,CalIdx.colidx)){
		cout<<"ERROR: col found out of current col step. Col is "<<column_ut<<" Col_step is "<<CalIdx.colidx<<" Trigger id is "<<(CalIdx.trigidx-1)%calpar.triggers;
		hcolsOut->Fill(CalIdx.colidx,column_ut); 
		//search for possible bit swap
		for(unsigned int bitn=0; bitn<32; bitn++){
		  edited_column_ut = column_ut ^ (0x1<<bitn); //swapping ONE  bit at a time
		  if (edited_column_ut == CalIdx.colidx*2 || edited_column_ut == CalIdx.colidx*2+1){ //found an error with Hamming distance ONE w.r.t a correct column.
		    hbitswapsinCol->Fill(bitn,CalIdx.colidx);
		    cout<<" SINGLE BIT ERROR ";
		    break;
		  }
		}
		cout<<endl;
	      }
	    }


	    // if (active_cols>2){
	    //  cout<<"ERROR: >2 active column found in event "<<evt.ordinal<<" higher col is "<<higher_col
	    //	    <<" when col_step id is: "<<CalIdx.colidx
	    //                     <<" active cols are: ";
	    // for (unsigned int i =0; i<debug_cols.size(); i++) cout<<debug_cols.at(i)<<" ";
	    // cout<<endl;
	    //}
	     




	    

	    if (trig_data.MissCalSoSFlag)  {
	      hbitWarning->Fill(MissedSoSWarning);
	      if (verbosity == kDEBUG) cout<<"WARNING: Missed Start of Scan Flag is Active "<<endl;
	    }
	    if (trig_data.MissCalTrigFlag)  {
	      hbitWarning->Fill(MissedCalibTrigWarning);
	      if (verbosity == kDEBUG) cout<<"WARNING: Missed Calib Trigger Flag is Active "<<endl;
	    }

	    if(verbosity==kDEBUG||verbosity==kMEDIUM) cout<<"Filling Histograms for event n  "<< evt.ordinal <<endl;   


	    //set DAC idx as a ordinal number every new DAC value
	    if (int(EOS_data.DACvalue) > prev_DACvalue) {
	      CalIdx.dacidx++;
	      cout<<"new DAC value "<<EOS_data.DACvalue<<" reconstructed DAC idx"<<CalIdx.dacidx<<endl;
	      prev_DACvalue = EOS_data.DACvalue;
	      //create new hMatrix histogram
	      new_hMatrix(CalIdx.dacidx);
	      //fill in hbco1
	      hbco1->SetBinContent(CalIdx.dacidx, exactbco-prev_exactbco);
	      prev_exactbco = exactbco;
	    }
	    
	    //Fill Step-Check histogram hevts
	    hevts->Fill(CalIdx.colidx, CalIdx.dacidx); 	

	    //Fill DACstep_BCO
	    DACstep_BCO->SetBinContent(CalIdx.dacidx, exactbco);
	  
	    //Fill the pixel histos
	    while (!pxl_ids.empty()) {
	      if (purify){               // Fills with PURIFY option selected
		if (validCol(col_ids.back(),CalIdx.colidx)){
		  calibpxl[pxl_ids.back()]->Fill(CalIdx.dacidx);      //pixel plot
		  hmatrix->Fill(col_ids.back(),row_ids.back());	      //cumulative matrix plot
		  hmatrixcurr->Fill(col_ids.back(),row_ids.back());   //current DAC step matrix plot
		}
		pxl_ids.pop_back();
		col_ids.pop_back();
		row_ids.pop_back();
	      }else if(sanitize){       // Fills with PURIFY option selected
		if (validCol(col_ids.back(),CalIdx.colidx)){   //VALID COL
		  calibpxl[pxl_ids.back()]->Fill(CalIdx.dacidx);      //pixel plot
		  hmatrix->Fill(col_ids.back(),row_ids.back());	      //cumulative matrix plot
		  hmatrixcurr->Fill(col_ids.back(),row_ids.back());   //current DAC step matrix plot
		}else{                                        //INVALID COL
		  column_ut = col_ids.back();
		  for(unsigned int bitn=0; bitn<32; bitn++){
		    edited_column_ut = column_ut ^ (0x1<<bitn); //swapping ONE  bit at a time
		    if (edited_column_ut == CalIdx.colidx*2 || edited_column_ut == CalIdx.colidx*2+1){
		      edited_pxl_id = edited_column_ut*32+row_ids.back();
		      calibpxl[edited_pxl_id]->Fill(CalIdx.dacidx);
		      //pixel plot
		      hmatrix->Fill(edited_column_ut,row_ids.back());	      //cumulative matrix plot
		      hmatrixcurr->Fill(edited_column_ut,row_ids.back());   //current DAC step matrix plot
		      break;
		    }else if (bitn==31) hitunsanitizable->Fill(CalIdx.colidx);
		  }
		}
		pxl_ids.pop_back();
		col_ids.pop_back();
		row_ids.pop_back();		
	      }else{                    // Fills all
		calibpxl[pxl_ids.back()]->Fill(CalIdx.dacidx);     //pixel plot
		hmatrix->Fill(col_ids.back(),row_ids.back());	   //cumulative matrix plot
		hmatrixcurr->Fill(col_ids.back(),row_ids.back());  //current DAC step matrix plot
		pxl_ids.pop_back();
		col_ids.pop_back();
		row_ids.pop_back();
	      }
	    }
	    

	    //Fill the hevtMult histo
	    hevtMult->Fill(hitMult);

	    //Fill colMult histo
	    hcolMult->Fill(active_cols);

	  }//end if mylayer
	}//end layer loop
      }//end if status complete
    }//end rob loop	
  }
  return 0;
}


int BookHist(std::string base, const FEMCalibParam & calpar, unsigned int layers){
  char name[40],title[80];
  const unsigned int col_steps = 16;
  const unsigned int X_size = 32;
  const unsigned int Y_size = 32;
  float dacmin,dacmax; 
  int dacsteps = (calpar.dacstop-calpar.dacstart)/calpar.dacstep+1;
  
  dacmin = -0.5;
  dacmax = dacsteps-0.5;

  snprintf(name,40,"hitBit");
  snprintf(title,80,"Hit bit active counts");
  hitBit = new TH1I(name,title,24,-0.5,23.5);

  snprintf(name,40,"chipHitBit");
  snprintf(title,80,"Chip hit bit active counts");
  chipHitBit = new TH1I(name,title,24,-0.5,23.5);

  snprintf(name,40,"hevts");
  snprintf(title,80,"%s: N trigger in  DAC step vs Col step",base.c_str());
  hevts = new TH2I(name,title,col_steps,-0.5,float(col_steps)-0.5,dacsteps,dacmin,dacmax);

  snprintf(name,40,"hmatrix");
  snprintf(title,80,"%s: integral spread",base.c_str());
  hmatrix = new TH2I(name,title,X_size,-0.5,float(X_size)-0.5, Y_size,-0.5,float(Y_size)-0.5);
  
  snprintf(name,40,"hbco1");
  snprintf(title,80,"%s: BCO counts for DAC steps ",base.c_str());
  hbco1 = new TH1I(name,title,dacsteps,dacmin,dacmax);

  snprintf(name,40,"hevtMult");
  snprintf(title,80,"Event Hit Multiplicity ");
  hevtMult = new TH1I(name,title,128,-0.5,127.5);

  snprintf(name,40,"hcolMult");
  snprintf(title,80,"Column Multiplicity in Event");
  hcolMult = new TH1I(name,title,10,-0.5,9.5);

  snprintf(name,40,"hbitWarning");
  snprintf(title,80,"Warning flags");
  hbitWarning = new TH1I(name,title,10,-0.5,9.5);

  snprintf(name,40,"hitunsanitizable");
  snprintf(title,80,"Unsanitizables hits per col step");
  hitunsanitizable = new TH1I(name,title,col_steps,-0.5, float(col_steps)-0.5);

  snprintf(name,40,"hcolsOut");
  snprintf(title,80,"Columns found (y) out of enabled matrix portion in a given col step (x)");
  hcolsOut = new TH2I(name,title,col_steps,-0.5, float(col_steps)-0.5, X_size,-0.5, float(X_size)-0.5);
 
  snprintf(name,40,"doubleHits");
  snprintf(title,80,"Double hits found ");
  doubleHits =  new TH2I(name,title,X_size,-0.5,float(X_size)-0.5, Y_size,-0.5,float(Y_size)-0.5);
 
  snprintf(name,40,"hbitswapsinCol");
  snprintf(title,80,"Probable columnn address bit (x) swapped counts w.r.t. correct col step(y) ");
  hbitswapsinCol=  new TH2I(name,title, 32,-0.5, 31.5, col_steps,-0.5, float(col_steps)-0.5);

	//BCO at new DACstep
  snprintf(name,40,"DACstep_BCO");
  snprintf(title,80,"%s: BCO seen at DACstep",base.c_str());
  DACstep_BCO = new TH1I(name,title,dacsteps,dacmin,dacmax);

	// global parameters
  snprintf(name,40,"activelayers");
  snprintf(title,80,"Active layers during calibration");
  TH1I* hlay = new TH1I(name,title,16,-0.5,15.5); // 
  for(int lay=0; lay<16; lay++){
    if( ((layers>>lay)&1)==1){
      hlay->Fill(lay);
    }
  }
  snprintf(name,40,"calibparam");
  snprintf(title,80,"Parameters for calibration trig,dac,delay");
  TH1I* hpar = new TH1I(name,title,10,0.5,10.5);
  hpar->SetBinContent(1,calpar.triggers);    // bin 1: triggers
  hpar->SetBinContent(2,calpar.dacstart);    // bin 2: dac start
  hpar->SetBinContent(3,calpar.dacstop);     // bin 3: dac stop
  hpar->SetBinContent(4,calpar.dacstep);     // bin 4: dac step
  hpar->SetBinContent(5,calpar.time_window); // bin 5: acquisition window length
  hpar->SetBinContent(6,calpar.BC_wait);     // bin6 : BC wait after trigger
  for(int i=0; i<1024; i++){
    snprintf(name,40,"hpixel_%04d",i);
    snprintf(title,80,"%s: pixel %04d  ",base.c_str(), i );
    calibpxl.push_back(new TH1I(name,title,dacsteps,dacmin-0.5,dacmax-0.5));
  }
  return 0;
}


//////////NORMALIZE_TO
void NormalizeTo(const FEMCalibParam & calpar){

  float dacmin,dacmax;
  int dacsteps;
  //////devono essere uguali a quelli di BookHist:
  dacsteps = (calpar.dacstop-calpar.dacstart)/calpar.dacstep+1;
  dacmin = -0.5;  
  dacmax = dacsteps-0.5;
  
  char name[40],title[80];
  for(int i=0; i<1024; i++){
    snprintf(name,40,"hoccupancy_%04d",i);
    snprintf(title,80,"hoccupancy: pixel %04d  ", i );
    occuppxl.push_back(new TH1F(name,title,dacsteps,dacmin,dacmax));
  }
  int trigs = calpar.triggers;
  double  values[dacsteps];
  double myvalues[dacsteps],errors[dacsteps];
  for(int i=0; i<1024; i++){
      for(int j=0; j<dacsteps; j++){
	  values[j] = calibpxl[i]->GetBinContent(j);
	  errors[j] = sqrt((values[j]+1)*(trigs-values[j])/
			   ((double) (trigs)))/((double) (trigs+1));
	  myvalues[j] = ((double)values[j])/(trigs+1);
	  if (myvalues[j]>1) cout<<"ERROR: Occupancy > 1"<<endl;
          occuppxl[i]->SetBinContent(j,myvalues[j]);
          occuppxl[i]->SetBinError(j,errors[j]);
      }
  }
}




///////SQL_SELECT 
//WARNING ::: remember to init the input structures before processing them with this function
bool sqlselect(int nrun, EPMCCalibParam & epmc1, EPMCCalibParam & epmc2 ){
  //
  // connect to mysql database
  //
  TSQLServer *db = TSQLServer::Connect("mysql://pcslim.bo.infn.it/tdaq","tdaq", "tdaq"); // da modificare
  if (db==0)     return false;
  else    {
      printf("Successfully connected to database\n");
      printf("Server info: %s\n", db->ServerInfo());
  }
   
  TSQLRow *row;
  TSQLResult *res;

  
  //
  // start timer
  //
  TStopwatch timer;
  timer.Start();
  char sql[1000];   // the sql query to be executed
  int nrows;        // number of rows returned by sql query
  int nfields;      // number of fields in each row
  //parameters in db
  int IDMaster;
  char runtype[200];
  int epmctype[2], epmcID[2];
  //epmcpixel fields
  int epmcNum[2];
  unsigned int Chip1RWEnable[2],     Chip2RWEnable[2];
  unsigned int Chip1ROIdentifier[2], Chip2ROIdentifier[2];
  unsigned int Chip1DACStart[2],     Chip2DACStart[2];
  unsigned int Chip1DACStop[2],     Chip2DACStop[2];
  unsigned int Chip1DACStep[2],      Chip2DACStep[2];
  unsigned int Chip1TrigValue[2],    Chip2TrigValue[2];
  unsigned int Chip1TimeWindow[2],   Chip2TimeWindow[2];
  unsigned int Chip1BCwait[2],       Chip2BCwait[2];
  unsigned char enaINMAPS_FEM = 0;
  unsigned int activeLayerBit = 0;
  int nLayers = 0;

  //CalibParam *ep = 0;  
  EPMCCalibParam *ep_cal = 0;
  


  ////////////////////////////////////////////////////////////////////////////////////
  // Get run type and IDMaster
  sprintf(sql,"select IDMaster,runtype from tdaq.runinfo WHERE run=%d", nrun);
  std::cout<<sql<<std::endl;

  res = db->Query(sql);

  nrows = res->GetRowCount();
  nfields = res->GetFieldCount();
  //  assert(nrows==1);
  //assert(nfields==2);
  


  row = res->Next();
  sscanf(row->GetField(0),"%d",&IDMaster);
  sscanf(row->GetField(1),"%s",runtype);
  delete row;
  delete res;
  std::cout << "IDMaster = " << IDMaster << std::endl;
  if (!strcmp(runtype, "MAPSCalibration") &&  !strcmp(runtype, "Calibration")){
    cout<<"ERROR: this is not a calibration data file"<<endl;
    return false;
  }

  
  /////////////////////////////////////////////////////////////////////////////////////
  //Get EPMC type 
  //  sprintf(sql, "select boardid,epmc1type,IDepmc1,epmc2type,IDepmc2 from tdaq.edroboard WHERE (ID=%d)",IDMaster);
  sprintf(sql, "select epmc1type,IDepmc1,epmc2type,IDepmc2 from tdaq.edroboard WHERE (run=%d)",nrun);
  std::cout<<sql<<std::endl;

       
  res = db->Query(sql);
  
  nrows   = res->GetRowCount();
  nfields = res->GetFieldCount();
  printf("\nGot %d rows with %d fields in result\n", nrows, nfields);
  if (nrows>1) {
    cout<<"ERROR: more than one active EDRO detected for run "<<nrun<<". INMAPS calibrations require 1EDRO partition"<<endl;
    return false;
  }

  row = res->Next();
  sscanf(row->GetField(0),"%d",&(epmctype[0]));
  sscanf(row->GetField(1),"%d",&(epmcID[0]));
  sscanf(row->GetField(2),"%d",&(epmctype[1]));
  sscanf(row->GetField(3),"%d",&(epmcID[1]));
  delete row;
  delete res;  

  if(epmctype[0]!=3 && epmctype[1]!=3){
    cout<<"ERROR: There's no EPMC with EPMC_PIXEL firmware loaded in run "<<nrun<<endl;
    return false;
  }

 
  sprintf(sql, "select EPMCnum,Chip1RWEnable, Chip1ROIdentifier, Chip1DACStart, Chip1DACStop, Chip1DACStep, Chip1TrigValue, Chip1TimeWindow, Chip1BCwait, "
                             " Chip2RWEnable, Chip2ROIdentifier, Chip2DACStart, Chip2DACStop, Chip2DACStep, Chip2TrigValue, Chip2TimeWindow, Chip2BCwait  "
               " from tdaq.epmcpixel WHERE (run=%d)",nrun);
  std::cout<<sql<<std::endl;
 
  res = db->Query(sql);

  nrows   = (res->GetRowCount()>1) ? 2 : 1 ; //prevent eventual errors in database (more than two epmc for the same calibration run are not allowed)
  nfields = res->GetFieldCount();
  printf("\nGot %d rows with %d fields in result\n", nrows, nfields);

  for (int epmc=0; epmc<nrows; epmc++){//EPMC loop
 
    enaINMAPS_FEM = 0;
    //Get calib param for current EPMC
    cout<<"Found EPMC in db: "<<endl<<endl;

    row = res->Next();
    sscanf(row->GetField(0),"%d", &epmcNum[epmc]);                cout<<hex<<" epmcNum            : "<<epmcNum[epmc]<<endl;
    sscanf(row->GetField(1),"%u", &Chip1RWEnable[epmc]);          cout<<hex<<" Chip1RWEnable      : "<<Chip1RWEnable[epmc]<<endl;
    sscanf(row->GetField(2),"%u", &Chip1ROIdentifier[epmc]);      cout<<hex<<" Chip1ROIdentifier  : "<<Chip1ROIdentifier[epmc]<<endl;
    sscanf(row->GetField(3),"%u", &Chip1DACStart[epmc]);          cout<<hex<<" Chip1DACStart      : "<<Chip1DACStart[epmc]<<endl;
    sscanf(row->GetField(4),"%u", &Chip1DACStop[epmc] );          cout<<hex<<" Chip1DACStop       : "<<Chip1DACStop[epmc]<<endl;
    sscanf(row->GetField(5),"%u", &Chip1DACStep[epmc]);           cout<<hex<<" Chip1DACStep       : "<<Chip1DACStep[epmc]<<endl;
    sscanf(row->GetField(6),"%u", &Chip1TrigValue[epmc]);         cout<<hex<<" Chip1TrigValue     : "<<Chip1TrigValue[epmc]<<endl;
    sscanf(row->GetField(7),"%u", &Chip1TimeWindow[epmc]);        cout<<hex<<" Chip1TimeWindow    : "<<Chip1TimeWindow[epmc]<<endl;
    sscanf(row->GetField(8),"%u", &Chip1BCwait[epmc]);            cout<<hex<<" Chip1BCwait        : "<<Chip1BCwait[epmc]<<endl;
    cout<<endl;
    sscanf(row->GetField(9),"%u",  &(Chip2RWEnable[epmc]));       cout<<hex<<" Chip2RWEnable      : "<<Chip2RWEnable[epmc]<<endl;
    sscanf(row->GetField(10),"%u", &(Chip2ROIdentifier[epmc]));   cout<<hex<<" Chip2ROIdentifier  : "<<Chip2ROIdentifier[epmc]<<endl;
    sscanf(row->GetField(11),"%u", &(Chip2DACStart[epmc]));       cout<<hex<<" Chip2DACStart      : "<<Chip2DACStart[epmc]<<endl;
    sscanf(row->GetField(12),"%u", &(Chip2DACStop[epmc] ));       cout<<hex<<" Chip2DACStop       : "<<Chip2DACStop[epmc]<<endl;
    sscanf(row->GetField(13),"%u", &(Chip2DACStep[epmc]));        cout<<hex<<" Chip2DACStep       : "<<Chip2DACStep[epmc]<<endl;
    sscanf(row->GetField(14),"%u", &(Chip2TrigValue[epmc]));      cout<<hex<<" Chip2TrigValue     : "<<Chip2TrigValue[epmc]<<endl;
    sscanf(row->GetField(15),"%u", &(Chip2TimeWindow[epmc]));     cout<<hex<<" Chip2TimeWindow    : "<<Chip2TimeWindow[epmc]<<endl;
    sscanf(row->GetField(16),"%u", &(Chip2BCwait[epmc]));         cout<<hex<<" Chip2BCwait        : "<<Chip2BCwait[epmc]<<endl;

    cout<<dec;

    printf("\nSearching EPMC %d \n", epmcNum[epmc]);
    //select the epmc calib param struct to be filled
    ep_cal = (epmcNum[epmc]==1) ? &epmc1 : &epmc2;
  
    if ((Chip1ROIdentifier[epmc]&0xFF000)==0x03000 && (Chip1RWEnable[epmc]&0x80000000) == 0x80000000 ){ //FEM0 is A3D and active
      nLayers++;
      //set the layers bit
      activeLayerBit = 0x1 <<((epmcNum[epmc]==1)? 0:4 );
      ep_cal->layers = ep_cal->layers | activeLayerBit; 
      printf("\nFound new active layer 0x%x \n", ep_cal->layers);
      //set the layer type: A3D
      ep_cal->type0 = 0x3;
      //set the FEM0 calib parameters
      ep_cal->fem0_cp.dacstart    = Chip1DACStart[epmc];
      ep_cal->fem0_cp.dacstop     = Chip1DACStop[epmc];
      ep_cal->fem0_cp.dacstep     = Chip1DACStep[epmc];
      ep_cal->fem0_cp.triggers    = Chip1TrigValue[epmc];
      ep_cal->fem0_cp.time_window = Chip1TimeWindow[epmc];
      ep_cal->fem0_cp.BC_wait     = Chip1BCwait[epmc];
    }
    
    if ((Chip2ROIdentifier[epmc]&0xFF000)==0x03000 && (Chip2RWEnable[epmc]&0x80000000) == 0x80000000 ) {//FEM1 is A3D and active
      nLayers++;
      //set the layers bit
      activeLayerBit = 0x2 <<((epmcNum[epmc]==1)? 0:4 );
      ep_cal->layers = ep_cal->layers | activeLayerBit; 
      //set the layer type: A3D
      ep_cal->type1 = 0x3;
      //set the FEM1 calib parameters
      ep_cal->fem1_cp.dacstart    = Chip2DACStart[epmc];
      ep_cal->fem1_cp.dacstop     = Chip2DACStop[epmc];
      ep_cal->fem1_cp.dacstep     = Chip2DACStep[epmc];
      ep_cal->fem1_cp.triggers    = Chip2TrigValue[epmc];
      ep_cal->fem1_cp.time_window = Chip2TimeWindow[epmc];
      ep_cal->fem1_cp.BC_wait     = Chip2BCwait[epmc];
    }
    delete row;
  }
  delete res;  
  delete db;

   
 
  // stop timer and print results
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();

  printf("\nRealTime=%f seconds, CpuTime=%f seconds\n", rtime, ctime);

  if(nLayers==0){
    cout<<"ERROR: No active layers found in run "<<nrun<<endl;
    return false;
  }


  return true; 
}


bool validCol(int col, int colstep){
  if  (col != colstep*2 && col != colstep*2+1)  
    return false;
  else
    return true;
}

























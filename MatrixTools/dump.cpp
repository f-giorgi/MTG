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



int main(int argc, char* argv[]){
  ofstream hit_out("hit_out.txt");
  cout << "Layer DUMP program.  " <<EVT_HEADER_SIZE << endl;
  cout << "extract ROB's ROD's layer's hit into hit_out.txt file "<<endl;
 

  if (argc <= 1 ){
    cout << "Usage: dump [-v0|1|2] [-evtmax|-e n_of_events] [-ch channel(0->7)] [-ed <eventN>][ -rob 0|1] <eventfile>" << endl;     
    return -1;
  }
  
  
  ///////////// Command line decoding //////////////
  unsigned long   maxevt                 = 0; 
  unsigned long   ROB                    = 0;
  long long int   ED                     = -1;
  _VERBOSITY      verbosity		 = kSILENT;
  int index = 1;           
  int channel = 1;
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
    }else if( (arg == "-evtmax") || (arg == "-e") ){
      index++;
      maxevt = strtoul(argv[index], NULL, 0);
      cout << "Event max set to " << maxevt << endl;
    }else if( (arg == "-ch") ){
      index++;
      channel = strtoul(argv[index], NULL, 0);
      cout << "Data Channel set to " << channel << endl;
    }else if( (arg == "-ed") ){
      index++;
      ED = strtoul(argv[index], NULL, 0);
      cout << "You want to see hits of event N  " << ED << endl;
    }else if( (arg == "-rob") ){
      index++;
      ROB = strtoul(argv[index], NULL, 0);
      cout << "ROB set to " << ROB << endl;
    } else 
      cout << "Unknown switch:" << arg << endl; 
    index++;
  }//end while
  
  // if index = argc it means we didn't supply only the flags but not the
  // filename
  if (argc == index ){
    cout << "Error: You didn't supply a filename" << endl;
    cout << "Usage: dump [-v0|1|2] [-evtmax|-e n_of_events] <eventfile>" << endl;
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
      cout << "Can't Open File" << argv[index]
	   <<" index="<<index<<" argc="<<argc<<endl;
    }	catch(std::exception& e) {
      cout << "Exception: " << e.what();
      cout << "Can\'t open file due to stdlib exception." << endl;       
      return -1; 
    }
    



    
    Slim5OutputParser parsertmp(filetmp);
    parsertmp.setVerbosity(verbosity);
    //bool another1;
    cout<<"now I try to parse"<<endl;
    
    /* try{
      another1 = parsertmp.FindEvent(); 
    } catch(std::exception& e){
      cout << "\t" <<  e.what() << endl << endl;
      another1 = parsertmp.IsEndOfFile();
    }*/
    unsigned int *raw;
    _CHUNK * raw_c;
    int N_trigger=0;
    int prev_TS=0;
    int TS;
    int Delta_TS;
    int N_hits=0;
    int trigger_x_DAC = 0;
    int DAC = 0;
    int prevDAC = 0;
    unsigned long evtN= 0;

    bool another1 = false;
    while (!another1){
      try{
	another1 = parsertmp.FindEvent(); 
      } catch(std::exception& e){
	cout << "\t" <<  e.what() << endl << endl;
      }
    }
    //first good event

    while (another1){
      try{
	another1 = parsertmp.FindEvent();
	//evt_n++;
      }
      catch(std::exception& e){
	cout << "\t" <<  e.what() << endl << endl;
	another1 = parsertmp.IsEndOfFile();
      }

      // hit_out<<hex<<0xee1234ee<<endl;
      evtN++;
      for (unsigned int i =0; i <parsertmp.GetEvent().ROBs[ROB].rod.layers[channel].hits.size() ; i++){
	raw_c =  &parsertmp.GetEvent().ROBs[ROB].rod.layers[channel].hits[i].raw;
	raw = reinterpret_cast<unsigned int*>(raw_c);
	hit_out<<hex<< *raw <<endl;
	if (evtN == ED-1 )  parsertmp.setVerbosity(kDEBUG);
	else parsertmp.setVerbosity(verbosity);
	if (evtN == ED ) {
	  cout<<"EVT_"<<ED<<" hit "<<i<<": "<<hex<< *raw <<dec<<endl;
	}

	if ((*raw &0x1) == 0x1){
	  N_trigger ++;
	  
	  TS = ((*raw & 0xff0000)>>16);
	  DAC = ((*raw & 0x1ffe)>>1);
	  Delta_TS = ((TS - prev_TS>0) ?  TS - prev_TS : TS - prev_TS+255 );
	  
	  // if (TS - prev_TS>0) Delta_TS =TS - prev_TS;
	  //else  Delta_TS = TS - prev_TS+255;
	  if (DAC>prevDAC){ 
	    if (prevDAC!=0) cout<<"TOT trigger for DAC step "<<hex<<prevDAC<<dec<<": "<<trigger_x_DAC <<endl;
	    trigger_x_DAC = 0;
	    prevDAC = DAC;
	  }
	  trigger_x_DAC++;
	  
	  if (verbosity== kMEDIUM || verbosity == kDEBUG) cout<<hex<< *raw<<" Cal. Trigger TS "<<TS <<" DAC " <<DAC<<" prev Nhits "<<dec<<N_hits<<" Delta TS "<<dec<<Delta_TS <<" Trigger N " <<dec<<N_trigger<<endl;
	  prev_TS =TS;
	  N_hits=0;
	  
	}else
	  N_hits++;
      }
    }
    
    
    
  } // end of local block
  
  
  
  
}

// FM Giorgi
// microBo analyzer -- designed for data analysis for the single electron interference experiment.
// Version 0.1 -- added file exist control
// Version 0.2 -- added capability to remove NSE events
//						 -- reduced verbosity while events are analyzed
// Version 0.3 -- added analyzer and clusters classes
//						 -- rate evaluation
//						 -- MatrixData and parser changed
#include <fstream>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <vector>
#include <TH2I.h>
#include <TH1I.h>
#include <TTree.h>
#include <TFile.h>
#include <TKey.h>
#include <TCanvas.h>
//exploit external file from the EventViewer GUI program: parser class also include matrixData class
#include "parser.h"
#include "matrixData.h"
#include "clusters.h"
#include "analyzer.h"

#define HIT_TS_MASK   0x000FF //TIME STAMP OF THE HIT
#define ROW_MASK      0xF8000 //row of the hit
#define R_COL_MASK    0x06000 //Pixel Column within MP
#define A_COL_MASK    0x01F00 //MP column


										
#define GET_PX_ACOL(x)  ( (x & A_COL_MASK)  >> 8 )
#define GET_PX_RCOL(x)  ( (x & R_COL_MASK)  >> 13 )
#define GET_PX_ROW(x)   ( (x & ROW_MASK)    >> 15 )

#define APSEL4DgetX(x) 	(GET_PX_ACOL(x)*4 + GET_PX_RCOL(x))
#define APSEL4DgetY(x)  (GET_PX_ROW(x))
#define APSEL4DgetTS(x) (x & HIT_TS_MASK) 



using namespace std;

bool and_array(vector<bool> x){
  unsigned int count = 0;
  for (unsigned int i=0; i<x.size(); i++){
    if (x[i]==true){
      count++;
    }
  }
  if (count==x.size()) return true; else return false;
}



int main(int argc, char* argv[]) {
  
	
  if (argc==2) {
    //Print Welcome
    cout<<endl<<endl;
    cout<<"===================================================================================="<<endl;
    cout<<" HIT ANALYZER v0.3"<<endl; 
    cout<<" Single Electron Interference Experiment @ microBo "<<endl;
    cout<<" program by F.M. Giorgi"<<endl;
    cout<<"===================================================================================="<<endl;
    cout<<endl;
		
  }else if (argc<2){
    cout<<" Need to specify the file name to analyze. Program quitting"<<endl;
    return 0;
  } else if (argc>2){
    cout<<" Too many arguments. Program quitting"<<endl;
    return 0;
  }



  //parsing file
  TH1I *rate;
  TH1I *mult;
  TH2I *spread;
  TH1I *Xspread, *Yspread, *time, *Xspread_lower_fourth, *Xspread_skew;
  parser *myparser;
  matrixData *mymatrixdata;
  const double kSkew = 0.45;
  int TS=0;
  int multiplicity;
  string answer;
  bool answerOK=false;
  bool removeNSE=false;
  int event_counter=0;
  int prevTS=0;
  int deltaTS=0;
  _EVENT myevent;
  myevent.Reset();
  ofstream decFileOut;
  decFileOut.open("decoded_output.txt");
	

  mymatrixdata = new matrixData();
  myparser		 = new parser();

	
	
  myparser->setDataType(APSEL4DmicroBOLOGNA);	 	
  myparser->setVerbosity(kSILENT);
  ifstream datafile;
  datafile.open(argv[1]);
  if (!datafile.is_open()){
    cout<<"File "<< argv[1]<<" not found"<<endl;
    return -1;
  }
  if (myparser->attachOutputFile(datafile)) cout<<"file attached"<<endl;
  //parse whole file
  myparser->fillMatrixData(mymatrixdata);	
  cout<<"Parsing File"<<endl;
  //print on screen all the events parsed.
  mymatrixdata->Print();
  cout<<"Final parser absolute time "<<myparser->GetTime()<<endl;
  cout<<"FInal parser event idx "<<myparser->GetIdx()<<endl;	
  cout<<"MD read pointer: "<<mymatrixdata->GetReadPointer()<<endl;	
	
  while(!answerOK){
    cout<<"remove Non Single Electron (NSE) events? [y/n]"<<endl;
    cin>>answer;
    if(answer.size()>1 || answer.size()<1 || (answer!="y" && answer!="n")) {
      cout<<"Bad answer, answer only 'y' for yes or 'n' for no."<<endl;
    }else{
      answerOK=true;
      if (answer=="y") removeNSE=true; else removeNSE=false;
    }
  }
	
  char filter[10];
  //(removeNSE==true) ? filter="NSEfilter" : filter="";
  (removeNSE==true) ?  	snprintf(filter,10,"%s","NSEfilter") :  snprintf(filter,1,"%s","");

	
	
	
  char title[100];
  snprintf(title,99,"Hit multiplicity%s_%s",filter,argv[1]);
  mult = new TH1I("multip",title, 41, -0.5, 40.5);
  snprintf(title,99,"Hit dispersion%s_%s",filter,argv[1]);
  spread = new TH2I("spread",title, 128, -0.5, 127.5, 16, -0.5, 31.5);
 	
  snprintf(title,99,"Hit Xdispersion%s_%s",filter,argv[1]);
  Xspread = new TH1I("Xspread",title, 128, -0.5, 127.5);

  snprintf(title,99,"Hit Xdisp_lowerfourth%s_%s",filter,argv[1]);
  Xspread_lower_fourth = new TH1I("Xspread_LF",title, 128, -0.5, 127.5);

 
  snprintf(title,99,"Hit Xdisp_manual_skew%s_s=%f_%s",filter,kSkew,argv[1]);
  Xspread_skew = new TH1I("Xspread_skew",title, 128, -0.5, 127.5);

  snprintf(title,99,"Hit Ydispersion%s_%s",filter,argv[1]);
  Yspread = new TH1I("Yspread",title, 128, -0.5, 127.5);

  snprintf(title,99,"Time dispersion%s_%s",filter,argv[1]);
  time = new TH1I("DeltaTime",title, 256, -0.5, 255.5);

  snprintf(title,99,"Rate%s_%s",filter,argv[1]);
  rate = new TH1I("Rate",title, 1, -0.5, 0.5);
	

  char rootfile[200];
  //	snprintf(rootfile,199,"histo%07d.root",run_n);
  snprintf(rootfile,199,"histo%s_%s.root",filter,argv[1]);
  TFile *f = TFile::Open(rootfile,"recreate");  	
	
	
	
  while (mymatrixdata->GetNextEvent(myevent)) {
	  
    TS = myevent.TimeStamp;
    event_counter++;
    if(event_counter%20000 == 0)cout<<event_counter<<" events analyzed :"<<endl;
		

    multiplicity=0;
    //spread histogram filling
    if (removeNSE==false){		
      for (unsigned int i=0; i<myevent.hits.size(); i++){
	//!!!!occhio che sto eliminando il pixel noioso APPARENTE al momento. quello realmente noioso e' il 127,31
	if (myevent.hits[i].X_address!=127 || myevent.hits[i].Y_address!=30){
	  //report on decoded data file
	  decFileOut<<myevent.TimeStamp<<"\t"<<myevent.hits[i].X_address<<"\t"<<myevent.hits[i].Y_address<<endl;
		      
		      
		      
	  multiplicity++;
	  spread->Fill(myevent.hits[i].X_address,myevent.hits[i].Y_address);
	  Xspread->Fill(myevent.hits[i].X_address);
	  Xspread_skew->Fill((myevent.hits[i].X_address)+myevent.hits[i].Y_address*kSkew);
	  if (myevent.hits[i].Y_address<8)Xspread_lower_fourth->Fill(myevent.hits[i].X_address);
	  Yspread->Fill(myevent.hits[i].Y_address);
	}
      }
    }else{		//removing NSE events
      if (myevent.hits.size()==1){
	if (myevent.hits[0].X_address!=127 || myevent.hits[0].Y_address!=30){
	  multiplicity++;
	  spread->Fill(myevent.hits[0].X_address,myevent.hits[0].Y_address);
	  Xspread->Fill(myevent.hits[0].X_address);
	  Xspread_skew->Fill((myevent.hits[0].X_address)+myevent.hits[0].Y_address*kSkew);
	  if (myevent.hits[0].Y_address<8)Xspread_lower_fourth->Fill(myevent.hits[0].X_address);
	  Yspread->Fill(myevent.hits[0].Y_address);
						
	}			
      }
    }
		
    //		deltaTS= ((myevent.TimeStamp-prevTS < 0) ? myevent.TimeStamp-prevTS+256 : myevent.TimeStamp-prevTS);
    //		prevTS=myevent.TimeStamp;		
		
    deltaTS=myevent.AbsTime-prevTS;
    if (deltaTS<0)cout<<"WARNING: deltaTS<0 in main program"<<endl;
    prevTS=myevent.AbsTime;
    time->Fill(deltaTS);

		


    //multiplicity histogram filling
    //mult->Fill(myevent.hits.size());
    if(multiplicity>0){	
      mult->Fill(multiplicity); //cosi' rimnuovo il pixel noioso
    }
    //fai che se gli eventi non sono consecutuvi incrementi il bin 0, con gli eventi vuoti
    //fai la molteplicita' dei cluster
  } 
  cout<<"myparser Time   "<<myparser->GetTime()<<endl;
  cout<<"myevent Abstime "<<myevent.AbsTime<<endl;
  cout<<"myparser hits_found         "<<myparser->GetHitsFound()<<endl;
  cout<<"mymatrixdata counts of hits "<<mymatrixdata->CountHits()<<endl;
  if (mymatrixdata->GetLastEvent(myevent)) cout<<"mymatrixData last event idx    "<<myevent.idx<<endl;		
  cout<<"mymatrixdata event_buffer size "<<mymatrixdata->GetSize()<<endl;
  analyzer *myanalyzer= new analyzer(300, 165., *mymatrixdata);
  clusters *myclusterdata= new clusters();
  //	if(myanalyzer->AttachMData(*mymatrixdata))cout<<"mymatrixdata attached to myanalyzer"<<endl;
  if(myanalyzer->AttachCData(*myclusterdata))cout<<"myclusterdata attached to myanalyzer"<<endl;
  //	myanalyzer->SetTimeRes(165.);
  myanalyzer->SetVerbosity(kSILENT);
  float Rate= 1.e+3*myparser->GetHitsFound()/(myparser->GetTime()* myanalyzer->GetTimeRes());
  cout<<"rate (kHz)"<< Rate<<endl;
  //cout<<"rate from myanalyzer (kHz)		"<< myanalyzer->EvalRate()<<endl;
	
  rate->SetBinContent(1, Rate);

  //clusters stuff
  myanalyzer->FindAllClusters();
	
	


  //////
  int lowM=0;
  int highM=0;
  const int MULT_LIMIT=3; //included in low sum
	
  for (int i=0; i<MULT_LIMIT+2; i++){
    lowM += int(mult->GetBinContent(i));
  }
	
  for (int i=MULT_LIMIT+2; i<42; i++){
    highM += int(mult->GetBinContent(i));
  }
	
  double ratio= float(highM)/float(lowM);
  cout<<"higher multiplicity / lower multiplicity "<<ratio<<"  ("<<MULT_LIMIT+1<<"-41 /"<<" 0-"<<MULT_LIMIT<<")"<<endl;
  int overflows = int(mult->GetBinContent(42));
  cout<<"Oveflows "<<overflows<<" (mult>41)"<<endl;
  ///////


	
  mult->Write("multiplicity");
  spread->Write("dispersion");
  Xspread->Write("Xdispersion");
  Xspread_lower_fourth->Write("Xdisp_lower_fourth");
  Xspread_skew->Write("Xdisp_manual_skew");
  Yspread->Write("Ydispersion");
  time->Write("DeltaTime");
  rate->Write("rate");
  f->Write();	
  f->Close();
  cout<<"root file written "<<rootfile<<endl;	
	

}



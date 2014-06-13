#include <fstream>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <vector>

#ifdef ROOT_SUPPORT
 #include <TFile.h>
 #include <TQObject.h>
#endif

#include <string>
//anal. tools
#include "matrixData.h"
#include "parser.h"
#include "clusters.h"
#include "analyzer.h"


using namespace std;


int main(int argc, char* argv[]) {

  matrixData *mymatrixData = new matrixData();
  clusters   *myclusters	 = new clusters();
	
	

  char error_message[450] = "This program requires the following files: \n run_number.txt \n gen_dataXXXXXXX.txt \n read_data_outXXXXXXX.txt \n read_raw_data_outXXXXXXX.txt \n trigger_reportXXXXXXX.txt \n where XXXXXXX is the run number identifier. \n It retrieves the run number ID by reading into the run_number.txt file that must be kept updated in the data  directory. It can also retrieve parameters from run_parametersXXXXXXX.txt file";

  //determine the run number
  //if no argument, default run number is defined in last row of run_number.txt file 
  //else one can pass as an argument the run_number to be analyzed
  int n = -1;
  int run_n = -1;
  bool useRawData = false;
  char  *optname, *optval;
  string optname_s, optval_s;
  string chipName;
  int index = 1;
  _VERBOSITY verbosity = kSILENT;
  int runIsTriggered = 0;
  double tres =0.0;
  int triglat = 0;
  uint32_t BCphase             = 1;
  uint32_t fullNoise           = 0;
  //calibration parameters
  uint32_t calibrate           = 0;
  uint32_t calib_DAC_MIN       = 0; 
  uint32_t calib_DAC_MAX       = 100;
  uint32_t calib_DAC_STEP      = 1;
  uint32_t calib_N_TRIGGER     = 1;
  uint32_t calib_N_ACTIVE_COLS = 1;
  uint32_t calib_TRIG_MODE     = 0;
  // uint32_t calib_TIME_WINDOW = 0;
  // uint32_t calib_TRIG_WAIT   = 0;

  _DATATYPE dataType = (_DATATYPE)((int)_DATATYPElast +10); //default type
  char dataFilesPath[50];

  //parsing the option list
  while ((index<argc) && (argv[index][0]=='-')){
    //set the option name string and the option value that follows. 
    //NB optval makes sense only if current option has an argument
    optname = argv[index];
    optname_s = optname;
    if (index+1 < argc){
      optval  = argv[index+1];

      optval_s = optval;
    }else {
      optval = NULL;
      optval_s = "";
    }

    if (optname_s == "-v"){
      index++; //increment index if this switch has an argument;
      if (optval != NULL){
        //verbosity = strtol(optval, NULL, 0);
        //cout<<atol(optval)<<endl;
        switch(atol(optval)){
        case 0: verbosity = kSILENT;
          break;
        case 1: verbosity = kMEDIUM;
          break;
        case 2: verbosity = kDEBUG;
          break;
        default: cout<<"Invalid Verbosity"<<endl;
          return -1;
          break;
        } //end switch   
      }	else {
        cout<<"Invalid number of argument"<<endl;
        return -1;
      }
    } else if(optname_s == "-rn"){
      index++;//increment index if this switch has an argument;
      if (optval != NULL){
        run_n = strtol(optval, NULL, 0); 
        if (run_n <0) {
          cout<<"ERROR: run number must be positive"<<endl;
          return -1;
        }
      }	else {
        cout<<"Invalid number of argument"<<endl;
        return -1;	
      }
    } else if(optname_s == "-trig"){
      index++;//increment index if this switch has an argument;
      if (optval != NULL){
        runIsTriggered = strtol(optval, NULL, 0); 
        if (runIsTriggered != 0 && runIsTriggered!= 1) {
          cout<<"ERROR: put 0 for DATAPUSH and 1 for TRIGGERED runs"<<endl;
          return -1;
        }
      }	else {
        cout<<"Invalid number of argument"<<endl;
        return -1;	
      }
    } else if(optname_s == "-type"){
      index++;//increment index if this switch has an argument;
      if (optval != NULL){
        dataType = (_DATATYPE)strtol(optval, NULL, 0); 
        if (dataType < _DATATYPEfirst || dataType > _DATATYPElast   ) {
          cout<<"ERROR: invalid data types. List of valid types follows:"<<endl;
          parser::listDataTypes();
          return -1;
        }
      }	else {
        cout<<"Invalid number of argument"<<endl;
        return -1;	
      }
    } else if(optname_s == "-tres"){
      index++;//increment index if this switch has an argument;
      if (optval != NULL){
        tres = strtod(optval, NULL); 
        if (tres <= 0.0  ) {
          cout<<"ERROR: invalid time resolution. time resolution must be positive and expressed in us (double)"<<endl;
          return -1;
        }
      }	else {
        cout<<"Invalid number of argument"<<endl;
        return -1;	
      }
    } else if(optname_s == "-triglat"){
      index++;//increment index if this switch has an argument;
      if (optval != NULL){
        triglat = strtol(optval, NULL, 0); 
        if (triglat <= 0  ) {
          cout<<"ERROR: invalid trig latency. latency must be positive and expressed in ps (integer)"<<endl;
          return -1;
        }
      }	else {
        cout<<"Invalid number of argument"<<endl;
        return -1;	
      }
    } else if(optname_s == "-BCphase"){
      index++;//increment index if this switch has an argument;
      if (optval != NULL){
        BCphase = strtol(optval, NULL, 0); 
        if (BCphase <= 0  ) {
          cout<<"ERROR: invalid BCphase. Phase must be positive and expressed in ps (integer)"<<endl;
          return -1;
        }
      }	else {
        cout<<"Invalid number of argument"<<endl;
        return -1;	
      }
      
    } else {
      cout << "BAD switch: " << optname_s << endl;
      return -1;
    }
    index++; // global index increment for every switch;
  }
  
  // if no chip type has been assigned exit with error
  if (dataType < _DATATYPEfirst || dataType > _DATATYPElast   ) {
    cout<<"Error: no chip type has been set. Use the -type switch with the following options."<<endl;
    parser::listDataTypes();
    return -1;
  }

  //data files path switch
  switch(dataType){
  case simuSORTEX200x256: 
    snprintf(dataFilesPath, 49, "SORTEX");
    break;
  case simuSQARE200x256: 
    snprintf(dataFilesPath, 49,  "SQUARE");
    break;
  case simuSQARE2x16x32: 
    snprintf(dataFilesPath, 49,  "INMAPS32x32");
    break;
  case StripFEBuffer: 
    snprintf(dataFilesPath, 49,  "Strip");    
    break;
  case simuSQARE2x48x128raw: 
    useRawData = true;
  case simuSQARE2x48x128: 
    snprintf(dataFilesPath, 49,  "APSELVI");    
    break;
  case simuSQARE2x16x128raw:
    useRawData = true;
  case simuSQARE2x16x128: 
    snprintf(dataFilesPath, 49,  "SUPERPIX1");    
    break;
  default: cout<<"Invalid DataType. You have selected a data type which is not supported for chip simulation analysis or data files switch in main() function may be not up-to-date"<<endl;
    return -1;
  } //end switch   
  

  // if no run number has been assigned  -> search in the run_number.txt file
  if (run_n == -1){
    cout<<"Automatic detection of run number:"<<endl;
    char rn_file[200];
    snprintf(rn_file,199,"%s/run_number.txt",dataFilesPath);
    ifstream run_number(rn_file);
    
    // input files check
    if (!run_number.is_open()) {
      cout<<"unable to find file "<< rn_file<<endl;
      cout<<error_message<<endl;
      return -1; 
    }
    //determining the Run number
    while(!run_number.eof()){
      run_number>>n;
    }
    run_n = n-1;
    cout<<"   ---> Retrieved run number: "<< run_n <<endl;
  }

  //get parameters from file
  
  cout<<"Retrieving parameters from file: ";
  char par_file[200];
  snprintf(par_file,199,"%s/run_parameters%07d.txt",dataFilesPath, run_n);
  ifstream param_file(par_file);
  cout<<par_file<<endl;
  if (!param_file.is_open()) {
    cout<<"unable to find file "<< par_file <<endl;
    cout<<error_message<<endl;
    return -1; 
  }

  string par;
  uint64_t BCperiod_ps;
  uint32_t trigBClat=0;

  while (!param_file.eof()){
    param_file>>par;
    if (!param_file.good()) break;
    //cout<<"analyzing parameter "<<par<<endl;
    if(par == "sim_triggered_mode"){
      param_file>>runIsTriggered;
      cout<<"run is triggered: "<< runIsTriggered<<endl;
      if (runIsTriggered != 0 && runIsTriggered!= 1) {
        cout<<"ERROR: put 0 for DATAPUSH and 1 for TRIGGERED runs"<<endl;
        return -1;
      }
     
      //trigger latency
    } else if(par == "sim_trigger_latency"){
      param_file>>trigBClat;
      cout<<"trig lat in BC counts: "<<trigBClat<<endl;
      if (trigBClat <= 0  ) {
        cout<<"ERROR: invalid trig BC latency. latency must be positive and expressed in BC counts"<<endl;
        return -1;
      }
    } else if(par == "chipName"){
      param_file>>chipName;
      cout<<"chip Name: "<<chipName<<endl;
      if (chipName != "APSELVI" && chipName!="SUPERPIX1"  ) {
        cout<<"ERROR: invalid chipName."<<endl;
        return -1;
      }
    } else if(par== "BCCLK_period"){
      param_file>>BCperiod_ps;
      cout<<"BC period (ps) : "<< BCperiod_ps <<endl;
      if (BCperiod_ps <= 0 ) {
        cout<<"ERROR: invalid time resolution. time resolution must be positive and expressed in ps (integer)"<<endl;
        return -1;
      }
      tres=BCperiod_ps*1.0e-6;
      cout<<"tres: "<<tres<<endl;
    } else if(par == "BCphase"){
      param_file>>BCphase;
      cout<<"BCphase "<< BCphase<<endl;
      if (BCphase <= 0  ) {
        cout<<"ERROR: invalid BCphase. latency must be positive and expressed in ps (integer)"<<endl;
        return -1;
      }
    } else if(par == "calibrate"){
      param_file>>calibrate;
      cout<<"Calibrate "<<calibrate<<endl;
      if (calibrate <0 || calibrate > 1 ) {
        cout<<"ERROR: invalid calibrate parameter.Valid values are 0 and 1."<<endl;
        return -1;
      }
    } else if(par == "calib_DAC_MIN"){
      param_file>>calib_DAC_MIN;
      cout<<"calib_DAC_MIN "<<calib_DAC_MIN<<endl;
      if (calib_DAC_MIN < 0 ) {
        cout<<"ERROR: invalid calib_DAC_MIN parameter.Value must be positive."<<endl;
        return -1;
      }
    } else if(par == "fullNoise"){
      param_file>>fullNoise;
      cout<<"fullNoise "<<fullNoise<<endl;
      if (fullNoise < 0 || fullNoise >1 ) {
        cout<<"ERROR: invalid fullNoise parameter.Value must be 0 or 1."<<endl;
        return -1;
      }
    } else if(par == "calib_DAC_MAX"){
      param_file>>calib_DAC_MAX;
      cout<<"calib_DAC_MAX "<<calib_DAC_MAX<<endl;
      if (calib_DAC_MAX < 0 ) {
        cout<<"ERROR: invalid calib_DAC_MAX parameter.Value must be natural."<<endl;
        return -1;
      }
    } else if(par == "calib_DAC_STEP"){
      param_file>>calib_DAC_STEP;
      cout<<"calib_DAC_STEP "<<calib_DAC_STEP<<endl;
      if (calib_DAC_STEP <= 0 ) {
        cout<<"ERROR: invalid calib_DAC_STEP parameter.Value must be positive."<<endl;
        return -1;
      }
    } else if(par == "calib_N_TRIGGER"){
      param_file>>calib_N_TRIGGER;
      cout<<"calib_N_TRIGGER "<<calib_N_TRIGGER<<endl;
      if (calib_N_TRIGGER <= 0 ) {
        cout<<"ERROR: invalid calib_N_TRIGGER parameter.Value must be positive."<<endl;
        return -1;
      }
    } else if(par == "calib_N_ACTIVE_COLS"){
      param_file>>calib_N_ACTIVE_COLS;
      cout<<"calib_N_ACTIVE_COLS "<<calib_N_ACTIVE_COLS<<endl;
      if (calib_N_ACTIVE_COLS <= 0 ) {
        cout<<"ERROR: invalid calib_N_ACTIVE_COLS parameter.Value must be positive."<<endl;
        return -1;
      }
    } else if(par == "calib_TRIG_MODE"){
      param_file>>calib_TRIG_MODE;
      cout<<"calib_TRIG_MODE "<<calib_TRIG_MODE<<endl;
      if (calib_TRIG_MODE < 0 ) {
        cout<<"ERROR: invalid calib_TRIG_MODE parameter.Value must be natural."<<endl;
        return -1;
      }
    } else
      //if gen_th,SRB_gen_th
      // if (par == "gen_th" || par == SRB_gen_th)
      //  param_file>>
      param_file>>par;   
  }
  //now you must have trigBClat and BCperiod_ps, calculate the triglat in ps.
  triglat = trigBClat*BCperiod_ps;
  cout<<"trig lat in ps: "<<triglat<<endl;   

  _CHIP_PROPERTIES chip_p;
  parser::getChipProperties(dataType, chip_p);
  if (chip_p.chipName != chipName) {
    cout<<"ERROR(MatSimAnalysis): Type of data is different from the requested."<<endl;
    return -1;
  }
  cout<<"INFO(MatSimAnalysis): Retrieved chip properties for chip "<<chip_p.chipName<<endl;




  //files to handle
  //	generated data
  char gen_file[200];
  snprintf(gen_file,199,"%s/gen_data%07d.txt",dataFilesPath,run_n);
  ifstream gen_data(gen_file);

  // read out data
  char read_out_file[200];
  snprintf(read_out_file,199,"%s/read_data_out%07d.txt",dataFilesPath,run_n);
  ifstream read_data_out(read_out_file);

  // read out RAW data
  char read_raw_file[200];
  snprintf(read_raw_file,199,"%s/read_raw_data_out%07d.txt",dataFilesPath,run_n);
  ifstream read_raw_out(read_raw_file);

  // trig data
  char trig_file[200];
  snprintf(trig_file,199,"%s/trigger_report%07d.txt",dataFilesPath,run_n);
  ifstream trig_data(trig_file);

  // input files check
  if (!gen_data.is_open() || !read_data_out.is_open() || !read_raw_out.is_open()|| !trig_data.is_open()) {
    cout<<"unable to find one or more input data files"<<endl;
    cout<<error_message<<endl;
    //return 0; 
  }


  // report files
  char DataOutReportfile[200];
  snprintf(DataOutReportfile,199,"%s/DataOutReport%07d.txt",dataFilesPath, run_n);
  ofstream DataOutReport(DataOutReportfile);
  
  char DataGenReportfile[200];
  snprintf(DataGenReportfile,199,"%s/DataGenReport%07d.txt",dataFilesPath, run_n);
  ofstream DataGenReport(DataGenReportfile);

  // report files
  char ExtraHitReportfile[200];
  snprintf(ExtraHitReportfile,199,"%s/ExtraHitReport%07d.txt",dataFilesPath, run_n);
  ofstream ExtraHitReport(ExtraHitReportfile);
  
  char LostHitReportfile[200];
  snprintf(LostHitReportfile,199,"%s/LostHitReport%07d.txt",dataFilesPath, run_n);
  ofstream LostHitReport(LostHitReportfile);


  //// root file	
  //	char rootfile[200];
  //	snprintf(rootfile,199,"SQUARE/histo%07d.root",run_n);
  //	TFile *f=TFile::Open(rootfile,"recreate");  	


  cout<<"analyzing RUN NUMBER "<< run_n <<endl;
  DataOutReport<<"RUN NUMBER "<< run_n << endl;
  DataGenReport<<"RUN NUMBER "<< run_n << endl;
	
  //cout<<"outFile "<<read_out_file<<endl;
	
  //////////////////////////////////////////////////////////////////////////////////////////
  ////ANALISI 
  //////////////////////////////////////////////////////////////////////////////////////////

  
  analyzer 	*myanalyzer = new analyzer(run_n, tres);
  if(!myanalyzer->AttachMData(*mymatrixData))	return -1;
  if(!myanalyzer->AttachCData(*myclusters))		return -1;	
  myanalyzer->SetVerbosity(verbosity);
  myanalyzer->SetTriggeredFlag(runIsTriggered);
  myanalyzer->SetTriggerLatency(triglat);  

  //	//parsing read_data_out and performing cluster analysis
  if (useRawData){
    if(!myanalyzer->ParseOutputData(read_raw_out, dataType)) 
      return -1; }
  else 
    if(!myanalyzer->ParseOutputData(read_data_out, dataType)) 
      return -1;	

  mymatrixData->SortXY();
  mymatrixData->PrintAll(DataOutReport);
  // uint32_t SM_X_size = chip_p.submat_X_size;
  // uint32_t SM_Y_size = chip_p.submat_Y_size;
  // uint32_t NX_SM     = chip_p.NX_submat;

	
  //find clusters and fill cluster buffer
  	if(!myanalyzer->FindAllClusters())	//return  -1;	//questo non va alla fine
  		cout<<"FindAllClusters went bad"<<endl;
	
	
    //cout<<"You have worked on Run "<<myanalyzer->GetRunNumber()<<endl;
	
  //analyze and produce root file
  #ifdef ROOT_SUPPORT
  	myanalyzer->AnalyzeCData(simuSQARE2x16x32);		
	#endif
	
  //mymatrixData->Print(2, 10);
  //myclusters->Print(2, 5);

  _EVENT myLostEvent;		
  _EVENT myExtraEvent;
  _EVENT myOutEvent;
  // _EVENT myOutEventCopy;
  _EVENT myReferenceEvent;
  _EVENT myReferenceEventCopy;
  uint32_t delta;
  
  if (calibrate){
    cout<<"INFO(MatSimAnalysis): Analyzing a Calibration Run "<<endl;
    myReferenceEvent.Reset();
    myReferenceEventCopy.Reset();
    myOutEvent.Reset();
    //myOutEventCopy.Reset();
    mymatrixData->ResetReadPointer();

    for(uint32_t DAC = calib_DAC_MIN; DAC < calib_DAC_MAX; DAC+=  calib_DAC_STEP){
      cout<<"INFO(MatSimAnalysis): Analyzing DAC step = "<<DAC <<endl;
      for(uint32_t colStep = 0; colStep <chip_p.submat_X_size ; colStep += calib_N_ACTIVE_COLS){
        //cout<<"INFO(MatSimAnalysis): Analyzing column step = "<<colStep <<endl;
        myReferenceEvent.Reset();
        myReferenceEventCopy.Reset();
        //fill reference event with expected hits
        for(uint32_t sm=0; sm < chip_p.NX_submat; sm++){
          for(uint32_t i =0; i< calib_N_ACTIVE_COLS; i++){
            for(uint32_t j =0 ; j<chip_p.submat_Y_size; j++){
              myReferenceEvent.AddHit(colStep+i+(sm*chip_p.submat_X_size),j);
            }
          }
        }
        //myReferenceEvent.Print();
        //myOutEvent.Print();

        for(uint32_t trigN = 0; trigN < calib_N_TRIGGER; trigN++){
          myReferenceEventCopy = myReferenceEvent;
          //retrieve output event
          if(!mymatrixData->GetNextEvent(myOutEvent)){
            cout<<"ERROR(MatSimAnalysis): missing events in calibration output"<<endl;
            return -1;
          }
          //myOutEvent.Print();
          //myReferenceEventCopy.Print();
          if(!myanalyzer->XCheck_Events(myOutEvent,myReferenceEventCopy, delta)){
            cout<<"ERROR(MatSimAnalysis): hits are missing in the calibration stream. Lost: "<<(chip_p.NX_submat*chip_p.submat_X_size)-delta<<endl;
          } //else cout<<"Trig OK"<<endl;         
        }
      }
    }
    cout<<"INFO(MatSimAnalysis): Calibration OK, all expected data was found on output"<<endl;
    return 0;
    
  //fullNoise run check
  }else if (fullNoise == 1){
    cout<<"WARNING(MatSimAnalysis): Full Noise simulation check is not supported yet."<<endl;
    return 0; 

  //standard Montecarlo simulations  
  }else{
     cout<<"INFO(MatSimAnalysis): Analyzing a Montecarlo Run "<<endl;

    //parse the generated hit file
    matrixData *simGenData = new matrixData();
    matrixData *lostHit = new matrixData();
    matrixData *extraHit = new matrixData();

  
    if (!myanalyzer->ParseSimGenData(gen_data, trig_data, simGenData)) {
      cout<<"check_sim: could not parse generated hit file"<<endl;
      return -1;
    }
	
    simGenData->SortXY();
    simGenData->PrintAll(DataGenReport);

    if(!myanalyzer->XCheck(simGenData, lostHit, extraHit)){
      //print only those which are not empty, otherwise use PrintAll();
      lostHit->Print(LostHitReport);
      extraHit->Print(ExtraHitReport);

      //cout<<"============Hits lost:========================="<<endl;
      //lostHit->Print();
      //cout<<"============Extra hits:========================"<<endl;
      //extraHit->Print();
		
  
   


      //////////ROOT based operations
#ifdef ROOT_SUPPORT   
      myLostEvent.Reset();	
      myExtraEvent.Reset();	
      myOutEvent.Reset();
      //myReferenceEvent.Reset();
      TFile hfile("htree.root","RECREATE","ROOT file with trees");    
      TTree *lostTree = new TTree("lostTree", "A tree that contains 1 branch for the lost hits");
      TTree *extraTree = new TTree("extraTree", "A tree that contains 1 branch for the extra hits");
      TH2I * hmatrix = new TH2I("outHits","Output hits" , chip_p.mat_X_size,-0.5,float(chip_p.mat_X_size)-0.5, chip_p.mat_Y_size,-0.5,float(chip_p.mat_Y_size)-0.5  );
      
      lostTree->Branch("lostHit", &myLostEvent, "idx/i:TimeStamp:hits");
      extraTree->Branch("extraHit", &myExtraEvent, "idx/i:TimeStamp:hits");

      lostHit->ResetReadPointer();
      extraHit->ResetReadPointer();
      mymatrixData->ResetReadPointer();
      //simGenData->ResetReadPointer();

      while(lostHit->GetNextEvent(myLostEvent)){
        lostTree->Fill();
      }
      while(extraHit->GetNextEvent(myExtraEvent)){
        extraTree->Fill();
      }
      while(mymatrixData->GetNextEvent(myOutEvent)){
        for (int i =0; i< myOutEvent.GetSize(); i++){
          hmatrix->Fill(myOutEvent.hits[i].X_address,myOutEvent.hits[i].Y_address );
        }
      }
 //  while(simGenData->GetNextEvent(myReferenceEvent){
              
//       }

      
#endif




     
      //Check if lost hits are correctly flagged in data stream.
      //   This works only with raw data analysis.
      uint32_t NflaggedLostHits = 0;

      //    const uint32_t HRB1_MSK_1 = 0x400;
      //     const uint32_t HRB1_MSK_0 = 0x4;
      //     const uint32_t SJ_MSK_1   = 0x200;
      //     const uint32_t SJ_MSK_0   = 0x2;
      //     const uint32_t HRB2_MSK_1 = 0x100;
      //     const uint32_t HRB2_MSK_0 = 0x1;
      bool SM_lost_data[chip_p.NX_submat];
      for (uint32_t i=0; i<chip_p.NX_submat; i++){ SM_lost_data[i]=false; }

      bool SM0_lost_data = false;
      bool SM1_lost_data = false;
      myLostEvent.Reset();
      myOutEvent.Reset();
      if (useRawData) {
    
        lostHit->ResetReadPointer();
        while(lostHit->GetNextEvent(myLostEvent)){
          SM0_lost_data = false;
          SM1_lost_data = false;
          if (myLostEvent.GetSize()>0){                          // if the event has LOST HITS
            // retrieve warnign information from flags in TS words
            mymatrixData->GetEvent(myLostEvent.idx, myOutEvent); // retrieve the event with same ID

            for (uint32_t sm=0; sm<chip_p.NX_submat; sm++){
              if (myOutEvent.flags & (EVTflags_SJ_MASK<<(sm*8))) SM_lost_data[sm]=true;  //check the sweepJump flag
            }

//             if (myOutEvent.flags & EVTflags_SQARE2x48x128_SJ_MASK0)                             // controllo se sweepJump flag SM0
//               SM0_lost_data = true;
//             if (myOutEvent.flags & EVTflags_SQARE2x48x128_SJ_MASK1)                             // controllo se sweepJump flag SM1
//               SM1_lost_data = true;


            mymatrixData->GetEvent(myLostEvent.idx+1, myOutEvent); // retrieve event with  ID+1 

            for (uint32_t sm=0; sm<chip_p.NX_submat; sm++){
              if ((myOutEvent.flags & (EVTflags_HRB1_MASK<<(sm*8))) | ( myOutEvent.flags & (EVTflags_HRB2_MASK<<(sm*8)))) SM_lost_data[sm]=true;  //check the hit reject flags on B1 and B2
            }



//             if ((myOutEvent.flags & EVTflags_SQARE2x48x128_HRB1_MASK0) | (myOutEvent.flags & EVTflags_SQARE2x48x128_HRB2_MASK0))
//               SM0_lost_data = true;
//             if ((myOutEvent.flags & EVTflags_SQARE2x48x128_HRB1_MASK1) | (myOutEvent.flags & EVTflags_SQARE2x48x128_HRB2_MASK1))
//               SM1_lost_data = true; 

            //increment the number of flagged lost hits

            for (int i=0; i< myLostEvent.GetSize(); i++){
              for (uint32_t sm=0; sm<chip_p.NX_submat; sm++){
                if (myLostEvent.hits[i].X_address >= sm*chip_p.submat_X_size && myLostEvent.hits[i].X_address < (sm+1)*chip_p.submat_X_size ) {
                  if (SM_lost_data[sm]) NflaggedLostHits++;
                  else cout<< "WARNING(MatSimAnalysis): Event ID " <<myLostEvent.idx<<" has lost hits in SM "<<sm<<" which are not correctly flagged"<<endl; 
                }
              }

//               if (myLostEvent.hits[i].X_address <  48 ){
//                 if (SM0_lost_data) NflaggedLostHits++;   //if it belongs to the sm 0
//                 else cout<< "WARNING(MatSimAnalysis): Event ID " <<myLostEvent.idx<<" has lost hits in SM0 which are not correctly flagged"<<endl; 
//               }else{
//                 if(SM1_lost_data) NflaggedLostHits++; 
//                 else cout<< "WARNING(MatSimAnalysis): Event ID " <<myLostEvent.idx<<" has lost hits in SM1 which are not correctly flagged"<<endl; 
//               }



            }
          }
        }
      }
      else cout<<"INFO(MatSimAnalysis): You are not analyzing raw data. The event flag analysis will no be performed on lost hits"<<endl;

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







      cout<<endl<<endl<<"############################  REPORT    ##############################"<<endl;

      cout<<"Hit lost          : "<<lostHit->CountHits()<<endl;
      cout<<"Correctly flagged : "<<NflaggedLostHits<<endl;
      cout<<"In-Out Diff.      : "<<simGenData->CountHits()-mymatrixData->CountHits()<<endl<<endl;

      cout<<"Extra hits        : "<<extraHit->CountHits()<<endl;
      {//print the extra hits
        myExtraEvent.Reset();
        extraHit->ResetReadPointer();
        if (extraHit->CountHits()>10){
          cout<<"There are lot of extra hits (>50), please refer to the ExtraHitReportXXXXX.txt file for the full list."<<endl;
        }else{
          while(extraHit->GetNextEvent(myExtraEvent)){
            for (int i = 0; i< myExtraEvent.GetSize(); i++){
              cout<<"    EVT ID : "<<setw(8)<<myExtraEvent.idx
                  <<"            X: "<<setw(5)<<myExtraEvent.hits[i].X_address
                  <<"            Y: "<<setw(5)<<myExtraEvent.hits[i].Y_address
                  <<"            HIT ID: "<<myExtraEvent.hits[i].X_address*128+myExtraEvent.hits[i].Y_address <<endl;  ///QUESTO CODICE NON E' PARAMETRIZZATO
            }
          }
        }
      }
      
      cout<<"TS crossing hits  : "<<myanalyzer->GetTSXingHits()<<endl;
      cout<<"They are          : "<<(myanalyzer->GetTSXingHits()*100.0)/myanalyzer->GetOutputHits()<<"% of output hits"<<endl;
      cout<<"BC ext-int phase  : "<<fixed<<setprecision(2)<< (BCphase*100.0)/(tres*1.0e+6)<<"% "<<endl;
      cout<<endl;


#ifdef ROOT_SUPPORT
      lostTree->Print();
      extraTree->Print();
      hmatrix->Print();
      hfile.Write();
#endif
  

		
      return -1;	
    }
	
    cout<<"CROSS CHECK SUCCESFUL"<<endl;
    cout<<"TS crossing hits:  "<<myanalyzer->GetTSXingHits()<<endl;


    //	ConsoleOut *Cout 	 = new ConsoleOut("Main");
    //	//equivalent to ConsoleOut *Cout 				 = new ConsoleOut("Main", false, ios_base::out);
	
    //	Cout->SetConsoleVerbosity(kDEBUG);	
    //	Cout->Print(WARNING, kMEDIUM, " vediamo se funziona");
			 
	
    return 0;
  }
}


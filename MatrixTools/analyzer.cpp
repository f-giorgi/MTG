#include "analyzer.h"

//constructor
analyzer::analyzer(){
  Init();
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): New analyzer created"<<std::endl;
}

analyzer::analyzer(int RN){
  Init();
  SetRunNumber(RN);
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): New analyzer created for Run "<<GetRunNumber()<<std::endl;
}

analyzer::analyzer(int RN, double TRes){
  Init();
  SetRunNumber(RN);
  SetTimeRes(TRes);
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): New analyzer created for Run "<<GetRunNumber()<<" (Time Res = "<<GetTimeRes()<<" us)"<<std::endl;
}


analyzer::analyzer(int RN, double TRes, matrixData & m_data){
  if(!AttachMData(m_data))std::cout<<"WARNING(analyzer): could not attach matrixData while creating new analyzer"<<std::endl;
  Init();
  SetRunNumber(RN);
  SetTimeRes(TRes);
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): New analyzer created for Run "<<GetRunNumber()<<" (Time Res = "<<GetTimeRes()<<" us)"<<std::endl;
}

analyzer::analyzer(int RN, double TRes, matrixData & m_data, matrixData & buffer_data){
  if(!AttachMData(m_data))std::cout<<"WARNING(analyzer): could not attach matrixData while creating new analyzer"<<std::endl;
  if(!AttachBufData(buffer_data))std::cout<<"WARNING(analyzer): could not attach buffer Data while creating new analyzer"<<std::endl;
  Init();
  SetRunNumber(RN);
  SetTimeRes(TRes);
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): New analyzer created for Run "<<GetRunNumber()<<" (Time Res = "<<GetTimeRes()<<" us)"<<std::endl;
}


void analyzer::Init(){
  Version = VERSION;
  SetTimeRes(.5);//expressed in us
  SetRunNumber(-1);
  SetTriggeredFlag(1);
  parameters.Reset();
  SetVerbosity(kSILENT);
  TSXingHits = 0;
  m_data = 0;
  buffer_data = 0;
  c_data= 0;	
}

bool analyzer::AttachMData(matrixData  &data){
  m_data = &data;
  if(!matrixDataIsAttached()) std::cout<<"WARNING(analyzer): matrixData is not attached"<<std::endl;
  return matrixDataIsAttached();
}

bool analyzer::AttachCData(clusters  &data){
  c_data = &data;
  if(!clusterDataIsAttached()) std::cout<<"WARNING(analyzer): clusterData is not attached"<<std::endl;	
  return clusterDataIsAttached();
}

bool analyzer::AttachBufData(matrixData  &data){
  buffer_data = &data;
  if(!bufferDataIsAttached()) std::cout<<"WARNING(analyzer): buffer data is not attached"<<std::endl;
  return bufferDataIsAttached();
}



void analyzer::ResetParameters(){
  parameters.Reset();
}

void analyzer::SetMicroParameters(int C1, int C2, int HV, int magn, int cam_length, int current){	
  parameters.C1=C1;
  parameters.C2=C2;
  parameters.HV=HV;
  parameters.magn=magn;
  parameters.cam_length=cam_length;
  parameters.current=current;
}

void analyzer::SetVerbosity(_VERBOSITY level){
  verbosity=level;
  if (verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): verbosity level "<<verbosity<<std::endl;
}

int analyzer::GetTSXingHits(){

  return TSXingHits;
}



// return true only if the manhattan distance is 1
bool analyzer::HitProximity(_HIT & p1, _HIT & p2){
  if (Get2DManhattanDistance(p1, p2) == 0 ) {
    if (verbosity==kMEDIUM || verbosity==kDEBUG)std::cout<<"WARNING: distance evaluated is 0"<<std::endl;
    return false;
  } else if (Get2DManhattanDistance(p1, p2) == 1) return true;
  else return false;

}

bool analyzer::ClusterProximity(_CLUSTER & c1, _CLUSTER & c2){
  for(unsigned int i=0; i<c1.hits.size(); i++){
    for(unsigned int j=0; j<c2.hits.size(); j++){
      if (c1.hits[i].X_address == c2.hits[j].X_address && c1.hits[i].Y_address == c2.hits[j].Y_address){
	if (verbosity==kMEDIUM || verbosity==kDEBUG)std::cout<<"WARNING: same hit found in two cluster while evaluating proximity  "<<c1.hits[i].X_address<<" "<<c1.hits[i].Y_address<<std::endl;
      }
      if(HitProximity(c1.hits[i],c2.hits[j])) return true;
    }
  }
  return false;
}


//add the hits in c1 to the hit list of c2 
bool analyzer::ClusterCopyHits(_CLUSTER & source, _CLUSTER & dest){
  if (source.hits.size()==0){
    return false;
    if (verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"WARNING the cluster you want to copy is empty of hits"<<std::endl;
  }
  if (&source == 0){
    return false;
    if (verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"WARNING the source cluster does not exist"<<std::endl;
  }

  if (&dest == 0){ 
    return false;
    if (verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"WARNING the destination cluster does not exist"<<std::endl;
  }

  for (unsigned int i=0; i<source.hits.size(); i++){
    dest.hits.push_back(source.hits[i]);
    dest.mult++;
  }
  return true;
}

bool analyzer::ClusterMerge(_CLUSTER & source, _CLUSTER & dest){
  if (source.hits.size()==0){
    return false;
    if (verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"WARNING the cluster you want to copy is empty of hits"<<std::endl;
  }
  if (&source == 0){
    return false;
    if (verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"WARNING the source cluster does not exist"<<std::endl;
  }

  if (&dest == 0){ 
    return false;
    if (verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"WARNING the destination cluster does not exist"<<std::endl;
  }

  ClusterCopyHits(source, dest);
  EvalClusterXcm(dest);
  EvalClusterYcm(dest);
  EvalClusterXspread(dest);
  EvalClusterYspread(dest);
  return true;
}


double analyzer::EvalClusterXcm(_CLUSTER & c1){
  int sum = 0;
  int N = 0;
  double ret;
  if (c1.hits.size()==0) return -1.;
  for(unsigned int i=0; i<c1.hits.size(); i++){
    sum += c1.hits[i].X_address;
    N++;
  }
  ret = double(sum)/double(N);
  c1.Xcm = ret;
  return ret;
}

double analyzer::EvalClusterYcm(_CLUSTER & c1){
  int sum = 0;
  int N = 0;
  double ret;
  if (c1.hits.size()==0) return -1.;
  for(unsigned int i=0; i<c1.hits.size(); i++){
    sum += c1.hits[i].Y_address;
    N++;
  }
  ret = double(sum)/double(N);
  c1.Ycm = ret;
  return ret;
}

int analyzer::EvalClusterXspread(_CLUSTER & c1){
  uint32_t min = 1000000;
  uint32_t max	= 0;
  int ret =0;
  if (c1.hits.size()==0) return -1;
  for(unsigned int i=0; i<c1.hits.size(); i++){
    min = c1.hits[i].X_address < min ? c1.hits[i].X_address : min;		
    max = c1.hits[i].X_address > max ? c1.hits[i].X_address : max;		
  }
  ret = (max-min+1);
  c1.Xspread = ret;
  return ret;
}

int analyzer::EvalClusterYspread(_CLUSTER & c1){
  uint32_t min = 1000000;
  uint32_t max	= 0;
  int ret =0;
  if (c1.hits.size()==0) return -1;
  for(unsigned int i=0; i<c1.hits.size(); i++){
    min = c1.hits[i].Y_address < min ? c1.hits[i].Y_address : min;		
    max = c1.hits[i].Y_address > max ? c1.hits[i].Y_address : max;		
  }
  ret = (max-min+1);
  c1.Yspread = ret;
  return ret;
}



int analyzer::FindClusters(_EVENT & event, _CLUSTER_EVENT & cluster_event){
  //WARNING FindClusters reset the content of cluster_event structure
  int ret=0;
  //this function returns: 
  // -1 = no hits found (empty events allowed in SQUARE architectures)
  // 0 = no cluster found (only separate hits)
  // n = nunmber of cluster with mult>1
		
  _CLUSTER cluster;
  cluster.Reset();
  // the event hits buffer is empty
  if (event.hits.size()==0) return -1;		
	
  //reset the cluster event
  cluster_event.Reset();
	
  //fill the cluster event with event tags
  cluster_event.TimeStamp = event.TimeStamp;
  cluster_event.idx = event.idx;
  cluster_event.AbsTime = event.AbsTime;
  cluster_event.RealTime = float(event.AbsTime)*TimeRes;
	
  //aggiungi la funzione che la calcola:
  cluster_event.av_dist = 0;
	
	
  //first step, each hit represents its own cluster, the distances between those objects are defined by the Manhattan distance:
  //create a cluster for each hit. 
  for (unsigned int i=0; i<event.hits.size(); i++){
    //fill in cluster data
    cluster.hits.push_back(event.hits[i]);
    cluster.Xcm = event.hits[i].X_address;
    cluster.Ycm = event.hits[i].Y_address;
    cluster.mult = 1;
    cluster.Xspread = 1;
    cluster.Yspread = 1;
    //push back this cluster in the cluster_event.cluster_buffer
    cluster_event.cluster_buffer.push_back(cluster);
    cluster_event.nclust++;
    cluster.Reset();
  }
  bool near_found=true;
  while (near_found){
    //if (verbosity==kDEBUG) std::cout<<"new while"<<std::endl;
    near_found=false;
    for (unsigned int i=0; i<cluster_event.cluster_buffer.size(); i++){
      //if (verbosity==kDEBUG) std::cout<<"  new i loop "<<i<<std::endl;
      for (unsigned int j=0; j<cluster_event.cluster_buffer.size(); j++){
	if (j!=i){
	  //if (verbosity==kDEBUG) std::cout<<"    new j loop "<<j<<std::endl;
	  // if cluster i is near cluster j
	  if(ClusterProximity(cluster_event.cluster_buffer[i], cluster_event.cluster_buffer[j])){
	    //if (verbosity==kDEBUG) std::cout<<"    found cluster proximity "<<std::endl;
	    ClusterMerge(cluster_event.cluster_buffer[i],cluster_event.cluster_buffer[j]);
	    cluster_event.nclust--;
	    //if (verbosity==kDEBUG) std::cout<<"    cluster merge executed"<<std::endl;
	    cluster_event.cluster_buffer.erase(cluster_event.cluster_buffer.begin()+i);				
	    near_found=true;	
	    break;
	  }
	}
      }
    }
  }
	
  //evaluate the return value
  for(unsigned int i =0;  i<cluster_event.cluster_buffer.size(); i++){
    if(cluster_event.cluster_buffer[i].mult>1) ret++;
  }
	
  return ret;
}



bool analyzer::FindAllClusters(){	
  _EVENT event;
  event.Reset();
  _CLUSTER_EVENT cluster_event;
  cluster_event.Reset();
  bool ret=false;
  //start from beginning of matrixData
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Clustering algorithm started " <<std::endl;

  if(!matrixDataIsAttached() || !clusterDataIsAttached()){ 
    if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"ERROR(analyzer) Data structures not attached to analyzer"<<std::endl;
    return ret;
  }	
	
  m_data->ResetReadPointer();		

  while (m_data->GetNextEvent(event)){
    if(FindClusters(event, cluster_event)>=0){
      c_data->AddClusterEvent(cluster_event);
      ret=true;
    }
  }
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Cluster search finished"<<std::endl;	
  return ret;
}

int analyzer::Get2DManhattanDistance(_HIT p1, _HIT p2){
  int X_dist=0;
  int Y_dist=0;
  (int(p1.X_address) - int(p2.X_address)) > 0 ? X_dist = (int(p1.X_address) - int(p2.X_address)) : X_dist = (int(p2.X_address) - int(p1.X_address));
  (int(p1.Y_address) - int(p2.Y_address)) > 0 ? Y_dist = (int(p1.Y_address) - int(p2.Y_address)) : Y_dist = (int(p2.Y_address) - int(p1.Y_address)); 
  return X_dist+Y_dist;
}

void analyzer::SetTimeRes(double x){
  TimeRes=x;
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Time Res = "<<GetTimeRes()<<" us"<<std::endl;
}

void analyzer::SetRunNumber(int RN){
  Run_number=RN;	
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Run Number = "<<GetRunNumber()<<std::endl;
}

void analyzer::SetTriggeredFlag(int x){
  if (x == 0) {
    triggered = false;
  }else{
    triggered = true;
  }
  if (verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): triggered flag set to "<<triggered<<std::endl;
}

double analyzer::GetTimeRes(){
  return TimeRes;
}

int analyzer::GetRunNumber(){
  return Run_number;
}

double analyzer::EvalRate(){
  double rate;
  _EVENT event;
  if (m_data->GetLastEvent(event)){
    rate=10.e+3*m_data->CountHits()/( event.AbsTime*GetTimeRes());////CONTROLLALO CHE NON SO SE VA BENE
    return rate;
  }else return -1.;
}

bool analyzer::ParseOutputData(ifstream & file, _DATATYPE DataType){


  std::cout<<"INFO(analyzer): Parsing  data file " <<std::endl;
	
	
  if(!matrixDataIsAttached()){ 
    if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"ERROR(analyzer) matrixData structure not attached to analyzer"<<std::endl;
    return false;
  }
	
  parser *myparser = new parser();
  myparser->setDataType(DataType);	 	
  myparser->setVerbosity(verbosity);
  myparser->setTriggeredFlag(triggered);
  myparser->setTimeRes(TimeRes);

  if (!(myparser->attachOutputFile(file)) || !(file.is_open())) {
    if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"File not valid"<<std::endl;		
    return  false;
  }
	
  if(!(myparser->CheckOutputFile(DataType))){
    if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"File Check gone BAD"<<std::endl;	
    return false;	
  }
	
	
  //parse whole file
  if(!myparser->fillMatrixData(m_data))return false;
	
  outputHits = myparser->GetHitsFound();
  Time = myparser->GetTime();
  lastTS = myparser->GetLastTS();

  
  std::cout<<"INFO(analyzer): Found "<<m_data->GetSize()<<" events and "<<outputHits<<" hits in output data file"<<std::endl;
	
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Output Data File checked and  parsed"<<std::endl;	
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Checking Fileout for double hits in same event"<<std::endl;		
  if(!CheckMatrixData(m_data)){
    if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"WARNING(analyzer): found double hit within same event while checking Fileout data"<<std::endl;	
  }	else if (verbosity==kMEDIUM || verbosity==kDEBUG)  std::cout<<"INFO(analyzer): No double hits found, checkMatrixData OK"<<std::endl;
	
  delete myparser;
  return true;
}

#ifdef ROOT_SUPPORT

bool analyzer::RootPlots(){
	

  return true;
}


bool analyzer::AnalyzeCData(_DATATYPE DataType){
  //devi avere parsato e riempito i cluster

  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Analyzing Cluster Data " <<std::endl;
	
  if(!clusterDataIsAttached()){ 
    if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"ERROR(analyzer) cluster data structure not attached to analyzer"<<std::endl;
    return false;
  }

  if(c_data->GetSize()==0){ 
    if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"WARNING(analyzer) found empty cluster data structure while analyzing"<<std::endl;
    return true;
  }
	
  int prevTS=0;
  int deltaTS=0;
  int multiplicity=0;
  char rootfile[200];
  int MatXSize;
  int MatYSize;
  double Rate= 0.0; 
  double kSkew= 0.45;
	
  //select the correct matrix dimension
  switch (DataType) {
  case simuSORTEX200x256:
    MatXSize=SRTX_MAT_X_SIZE;
    MatYSize=SRTX_MAT_Y_SIZE;			
    break;
  case simuSQARE2x16x32:
    MatXSize=32;
    MatYSize=32;			
    break;
  case simuSQARE200x256:
    MatXSize=200;
    MatYSize=256;
    break;
  case APSEL4DmicroBOLOGNA:
    MatXSize=A4D_MAT_X_SIZE;
    MatYSize=A4D_MAT_Y_SIZE;
    break;
  case FE4DtestPISA:
    MatXSize=SPX0_MAT_X_SIZE;
    MatYSize=SPX0_MAT_Y_SIZE;		
    break;
  default:
    MatXSize=200;
    MatYSize=256;
  }
	
  snprintf(rootfile,199,"histo_%i.root", GetRunNumber());
  TFile *f = TFile::Open(rootfile,"recreate");  	

  //book histograms
  TH1I *mult;
  TH2I *dispersion2D; //was *spread;
  TH1I *Xdispersion; //was *Xspread; 
  TH1I *Ydispersion; //was *Yspread; 
  TH1I *DiagonalDispersion; //was *Xspread_skew;
  TH1I *time; 
  TH1I *rate;
  TH2I *ClusterSpread;
  TH1I *ClusterMult;
  TH1I *NClustEvent;
  TH2I *ClusterCMDispersion2D;
  TH1I *ClusterDistance;

  char title[100];
	
  snprintf(title,99,"Hit multiplicity_%i",GetRunNumber());
  mult = new TH1I("mult",title, 41, -0.5, 40.5);
 	
  snprintf(title,99,"Hit dispersion_%i",GetRunNumber());
  dispersion2D = new TH2I("dispersion2D",title, MatXSize, -0.5, MatXSize-0.5, MatYSize, -0.5, MatYSize-0.5);
 	
  snprintf(title,99,"Hit Xdispersion_%i",GetRunNumber());
  Xdispersion = new TH1I("Xdispersion",title, MatXSize, -0.5, MatXSize-0.5);

  snprintf(title,99,"Hit Ydispersion_%i",GetRunNumber());
  Ydispersion = new TH1I("Ydispersion",title, MatYSize, -0.5, MatYSize-0.5);
 
  snprintf(title,99,"Hit Xdisp_manual_skew%f_%i", kSkew, GetRunNumber());
  DiagonalDispersion = new TH1I("DiagonalDispersion",title,  MatXSize, -0.5, MatXSize-0.5);

  snprintf(title,99,"Time dispersion_%i",GetRunNumber());
  time = new TH1I("DeltaTime",title, 256, -0.5, 255.5);

  snprintf(title,99,"Rate_%i",GetRunNumber());
  rate = new TH1I("Rate",title, 1, -0.5, 0.5);
	
  snprintf(title,99,"Cluster Spread_%i",GetRunNumber());
  ClusterSpread = new TH2I("ClusterSpread",title, 20, 0.5, 20.5, 20, 0.5, 20.5);

  snprintf(title,99,"Cluster mult_%i",GetRunNumber());
  ClusterMult = new TH1I("ClusterMult",title, 40, -0.5, 39.5);

  snprintf(title,99,"N Clusters per event_%i",GetRunNumber());
  NClustEvent = new TH1I("NClustEvent",title, 25, -0.5, 24.5);
	
  snprintf(title,99,"Cluster CM dispersion_%i",GetRunNumber());
  ClusterCMDispersion2D = new TH2I("ClusterCMDispersion2D",title, MatXSize, -0.5, MatXSize-0.5, MatYSize, -0.5, MatYSize-0.5);
	
  snprintf(title,99,"Cluster distance_%i",GetRunNumber());
  ClusterDistance = new TH1I("ClusterDistance",title, MatXSize+MatYSize, -0.5, MatXSize+MatYSize-0.5);
			
  _CLUSTER_EVENT cluster_event;
  cluster_event.Reset();
	
  c_data->ResetReadPointer();
  while(c_data->GetNextEvent(cluster_event)){
    multiplicity=0;
    if(verbosity==kDEBUG) std::cout<<"INFO(analyzer): Analyzing Cluster Event ID  "<< cluster_event.idx<<std::endl;
    //eventualmente fai un tree di root qui per ogni cluster ....
			
    NClustEvent->Fill(cluster_event.GetSize());
    ClusterDistance->Fill(cluster_event.av_dist);
			
    for(uint32_t j=0; j< cluster_event.cluster_buffer.size(); j++){	
      if(verbosity==kDEBUG) std::cout<<"    "<<std::endl;					
      if(verbosity==kDEBUG) std::cout<<"INFO(analyzer): Analyzing Cluster  "<<j<<std::endl;			

      //				std::cout<<"Cluster Multiplicity =  "<<mult<<std::endl;
      ClusterMult->Fill(cluster_event.cluster_buffer[j].GetSize()); 
				
      //				std::cout<<"Xcm = "<<Xcm<<std::endl;
      //				std::cout<<"Ycm = "<<Ycm<<std::endl;
      ClusterCMDispersion2D->Fill(cluster_event.cluster_buffer[j].Xcm, cluster_event.cluster_buffer[j].Ycm); 
	
				
      //				std::cout<<"Xspread = "<<Xspread<<std::endl;	
      //				std::cout<<"Yspread = "<<Yspread<<std::endl;	
      ClusterSpread->Fill(cluster_event.cluster_buffer[j].Xspread, cluster_event.cluster_buffer[j].Yspread);


      //				std::cout<<"Cluster hit list "<<std::endl;				
      for (uint32_t i=0; i < cluster_event.cluster_buffer[j].hits.size(); i++){			
	//					hits[i].Print();
	dispersion2D->Fill(cluster_event.cluster_buffer[j].hits[i].X_address, cluster_event.cluster_buffer[j].hits[i].Y_address);
	Xdispersion->Fill(cluster_event.cluster_buffer[j].hits[i].X_address);
	Ydispersion->Fill(cluster_event.cluster_buffer[j].hits[i].Y_address);
	DiagonalDispersion->Fill((cluster_event.cluster_buffer[j].hits[i].X_address)+cluster_event.cluster_buffer[j].hits[i].Y_address*kSkew);

	multiplicity++;
      }
    }
    //hits multiplicity
    mult->Fill(multiplicity);
    //time arrival distribution
    deltaTS = cluster_event.AbsTime-prevTS;
    if (deltaTS<0) if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"WARNING(analyzer): deltaTS<0 while analyzing cluster data"<<std::endl;
    prevTS = cluster_event.AbsTime;
    time->Fill(deltaTS);
					
  }
		
  //Rate = 10.e+3 * c_data->CountHits() / ( Time * GetTimeRes());
  //cout<<"rate (kHz)"<< Rate<<endl;
  //cout<<"rate from myanalyzer (kHz)		"<< myanalyzer->EvalRate()<<endl;
  rate->SetBinContent(1, Rate);
	
  mult->Write();
  dispersion2D->Write();
  Xdispersion->Write();  
  Ydispersion->Write(); 
  DiagonalDispersion->Write(); 
  time->Write(); 
  rate->Write();
  ClusterSpread->Write();
  ClusterMult->Write();
  NClustEvent->Write();
  ClusterCMDispersion2D->Write();
  ClusterDistance->Write();

	
  f->Write();	
  f->Close();
	


  return true;
}
//end if root support
#endif

bool analyzer::ParseBufferData(ifstream & file, ifstream & trig_file){
  //int BClat = trigger_latency/int(TimeRes*1.0e+6);
  std::cout<<"INFO(analyzer): Parsing Strip Buffered hits data file " <<std::endl;
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): trig_latency "<<trigger_latency<<" TimeRes "<<TimeRes
    //<<" BClat "<<BClat
						       <<std::endl; 
  parser *myparser = new parser();
  myparser->setDataType(StripFEBuffer);	 	
  myparser->setVerbosity(verbosity);
  myparser->setTriggeredFlag(triggered);
  myparser->setTrigLatency(trigger_latency);
  myparser->setTimeRes(TimeRes);
  //myparser->setBClat(BClat);
  
  if (!(myparser->attachOutputFile(file)) || !(file.is_open())) {
    if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"ERROR(analyzer): File not valid while parsing buffer data file"<<std::endl;		
    return  false;
  }
  if (!(myparser->attachTrigFile(trig_file)) || !(trig_file.is_open())) {
    if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"ERROR(analyzer): Trig File not valid while parsing buffer data file"<<std::endl;		
    return  false;
  }
  
  
  //parse whole file
  if(!myparser->fillMatrixData(buffer_data)) return false;
  outputHits= myparser->GetHitsFound();
  Time = myparser->GetTime();
  lastTS = myparser->GetLastTS();

  std::cout<<"INFO(analyzer): Found "<<buffer_data->GetSize()<<" events and "<<outputHits<<" hits in strip buffer data file"<<std::endl;
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Generated Hits file parsed"<<std::endl;	
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Checking Generated Hits file"<<std::endl;		
  
  if(!CheckMatrixData(buffer_data)) {
    if(verbosity==kMEDIUM || verbosity==kDEBUG)	std::cout<<"WARNING(analyzer): found double hit within same event while checking generated hit data"<<std::endl;
  } else if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Check OK"<<std::endl;
  delete myparser;
  return true; 
}

bool analyzer::ParseSimGenData(ifstream & file, ifstream & trig_file, matrixData *simGenData){
  std::cout<<"INFO(analyzer): Parsing Generated Hits data file " <<std::endl;
  //int BClat = trigger_latency/int(TimeRes*1.0e+6);	
		
  parser *myparser = new parser();
  myparser->setDataType(simuGenList);	 	
  myparser->setVerbosity(verbosity);
  myparser->setTriggeredFlag(triggered);
  if (triggered) myparser->setTrigLatency(trigger_latency);
  //myparser->setBClat(BClat);
  myparser->setTimeRes(TimeRes);

  if (!(myparser->attachOutputFile(file)) || !(file.is_open())) {
    if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"ERROR(analyzer): File not valid while parsing generated hit data file"<<std::endl;		
    return  false;
  }

  if (!(myparser->attachTrigFile(trig_file)) || !(trig_file.is_open())) {
    if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"ERROR(analyzer): Trig File not valid while parsing generated hit data file"<<std::endl;		
    return  false;
  }
	
  if(!(myparser->CheckOutputFile(simuGenList))){
    if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"File Check gone BAD"<<std::endl;	
    return false;	
  }
	
  //parse whole file
  if(!myparser->fillMatrixData(simGenData))return false;
	
  genHits = myparser->GetHitsFound();
  Time = myparser->GetTime();
  lastTS = myparser->GetLastTS();
	
  std::cout<<"INFO(analyzer): Found "<<simGenData->GetSize()<<" events and "<<genHits<<" hits in generated data file"<<std::endl;
	
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Generated Hits file checked and parsed"<<std::endl;	
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Checking generated data structure"<<std::endl;		
  if(!CheckMatrixData(simGenData)) {
    if(verbosity==kMEDIUM || verbosity==kDEBUG)	std::cout<<"WARNING(analyzer): found double hit within same event while checking generated hit data"<<std::endl;	
  } else if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Check OK"<<std::endl;
	
  delete myparser;
  return true;

}


bool analyzer::CheckEvent(_EVENT & evt){
  //check for double hits in event;
  for (int j=0; j<evt.GetSize(); j++){
    for (int k=0; k<evt.GetSize(); k++){
      if ( evt.hits[j].X_address == evt.hits[k].X_address && evt.hits[j].Y_address == evt.hits[k].Y_address && j!=k){
	return false;
      }
    }			 
  }
  return true;
}


bool analyzer::CheckMatrixData(matrixData *matrix_data){
  _EVENT evt;
  bool ret = true;
  //check for events with double hits;
  matrix_data->ResetReadPointer();
  while (matrix_data->GetNextEvent(evt)){
    if (!CheckEvent(evt)){
      ret = false;
      if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"WARNING(analyzer): event "<<evt.idx<<" has double hit."<<std::endl;
    }
  }
  return ret;
}


/*X check of a couple of events. This function actaully reduces the events of the common hits
It returns true if the Xcheck is OK, and set in the external variable delta the number of reduced hits*/
bool analyzer::XCheck_Events(_EVENT & evt1, _EVENT & evt2, uint32_t & delta){
  bool ret = true;
  delta = 0;
  if (evt1.GetSize() != evt2.GetSize()){
    if(verbosity==kDEBUG) std::cout<<"WARNING(analyzer): different size of events idx "<<evt1.idx<<". "
							 <<" Evt1 size "<<evt1.GetSize()<<" "
							 <<" Evt2 size "<<evt2.GetSize()<<" "
							 <<std::endl;	
  }	
  for (int i = evt1.GetSize()-1; i>=0 ; i--) {
    for (int j = evt2.GetSize()-1; j>=0; j--)	{
      if(evt1.hits[i].X_address == evt2.hits[j].X_address && evt1.hits[i].Y_address == evt2.hits[j].Y_address ){
        evt1.hits.erase(evt1.hits.begin()+i);
        evt2.hits.erase(evt2.hits.begin()+j);
        delta++;
        break;
      }
    }
  }	
  if (evt1.GetSize()>0) ret = false;
  if (evt2.GetSize()>0) ret = false;
  return ret;	
}




/*Xchek 2 events, generating a residual matrixData for each one, lostHit for genEvent, extraHit for outEvent
It also check the TS Xing hits in datapush mode, Xchecking 1 event backward in the extraHit matrixData.
It returns true if the Xcheck is OK. Same Event TS for triggered mode, no extra hit after TS Xing search for datapush*/
bool analyzer::XCheck_Events(_EVENT & GenEvt, _EVENT & OutEvt, matrixData *lostHit, matrixData  *extraHit){
  bool ret = true;
  uint32_t delta = 0;
  //int prev_extra_size=0;
  _EVENT GenEvt_int;
  _EVENT OutEvt_int;
  _EVENT prev_OutEvt;
	
  //copy events to local
  GenEvt_int = GenEvt;
  OutEvt_int = OutEvt;
  //////////////////////////////////
  if (triggered){
    ret = XCheck_Events(GenEvt_int, OutEvt_int, delta);
    if (GenEvt_int.GetSize()>0){
      if(verbosity==kDEBUG) std::cout<<"WARNING(analyzer): hits are lost at event "<<GenEvt_int.idx<<" evt size"<<GenEvt_int.GetSize()<<std::endl;
      lostHit->AddEvent(GenEvt_int);
      if(verbosity==kDEBUG) std::cout<<"WARNING(analyzer): Printing the hits lost in the event "<<GenEvt_int.idx<<std::endl;
      if(verbosity==kDEBUG) GenEvt_int.Print();
    }
    if(OutEvt_int.GetSize()>0){
      if(verbosity==kDEBUG) std::cout<<"WARNING(analyzer): extra hits found at event "<<OutEvt_int.idx<<" evt size "<<OutEvt_int.GetSize()<<std::endl;
      extraHit->AddEvent(OutEvt_int);
      if(verbosity==kDEBUG) std::cout<<"WARNING(analyzer): Printing the extra hits left in the event "<<OutEvt_int.idx<<std::endl;
      if(verbosity==kDEBUG) OutEvt_int.Print();
    }
  }else{ // data-push mode
    if (extraHit->GetSize() == 0){ // no extra hit at all by now.
        ret = XCheck_Events(GenEvt_int, OutEvt_int, delta); // take out common hits
        lostHit->AddEvent(GenEvt_int);               // add the gen event with remaining hits to lostHit
        extraHit->AddEvent(OutEvt_int);              // add the out event with remaining hits to extraHit
    } else {                      // extra hit matrixData is not empty
        extraHit->GetLastEvent(prev_OutEvt);         //take previous reduced out event
        if (prev_OutEvt.GetSize()>0){                // if it is not empty:
        
          XCheck_Events(GenEvt_int, prev_OutEvt, delta); // Xcheck previous reduced out event with current gen Event
          TSXingHits+= delta;
          XCheck_Events(GenEvt_int, OutEvt_int, delta);  //Xcheck current out event with current gen Event
          extraHit->Pop_BackEvent();              //delete the last extraHit event, to replace with the new one reduced of the TS X-ing hits.
          extraHit->AddEvent(prev_OutEvt);
          extraHit->AddEvent(OutEvt_int);
          lostHit->AddEvent(GenEvt_int);
          if (GenEvt_int.GetSize()>0)ret = false;
        } else {
            ret = XCheck_Events(GenEvt_int, OutEvt_int, delta);            // take out common hits
            lostHit->AddEvent(GenEvt_int);     // add the gen event with remaining hits to lostHit
            extraHit->AddEvent(OutEvt_int);    // add the out event with remaining hits to extraHit
        }
    }
  }
  //////////////////////////////////
/*


  ret = XCheck_Events(GenEvt_int, OutEvt_int);

  //if something is wrong, try to see if it is TS crossing, only in data push case
  if (!ret  && !triggered){
    if(GenEvt_int.GetSize()>0) {
      //quando non trovo delle hit in uscita in un evento, controllo se sono state trovate in piu' nell'evento precedente. 
      //(TS X-ing a causa del ritardo di propagazione del segnale TSCNT. Questo ritardo varia di riga in riga).
      //altrimenti e' proprio un errore.
      if (extraHit->GetSize()== 0) return false;// matrixData extraHit e' vuota quindi non sono TS X-ing hits
      extraHit->GetLastEvent(prev_OutEvt); //prendo l'ultimo evento di extraHit
      prev_extra_size=prev_OutEvt.GetSize();
      if(XCheck_Events(GenEvt_int, prev_OutEvt)){
	ret = true;
	TSXingHits+=prev_extra_size;
	extraHit->Pop_BackEvent();
	if(verbosity== kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): TS X-ing hits found between event id "<<prev_OutEvt.idx<<" and event id "<<GenEvt_int.idx<<std::endl;
      } else{//tutte o alcune non sono TS X-ing Hits
	if (GenEvt_int.GetSize()>0){
	  ret = false;
	  TSXingHits += prev_extra_size-GenEvt_int.GetSize();
	  if(verbosity==kDEBUG) std::cout<<"ERROR(analyzer): hits are lost even after TS X-ing check in event id "<<GenEvt_int.idx<<std::endl;
	  lostHit->AddEvent(GenEvt_int);
	  if(verbosity==kDEBUG) std::cout<<"ERROR(analyzer): Printing the hits lost in the event "<<GenEvt_int.idx<<std::endl;
	  if(verbosity==kDEBUG) GenEvt_int.Print();
	} 
	if(prev_OutEvt.GetSize()>0){
	  ret = false;
	  TSXingHits += prev_extra_size-prev_OutEvt.GetSize();
	  extraHit->Pop_BackEvent(); //elimino l'evento precedente cosi' lo scrivo dopo averlo ridotto delle TS X-ing hits.
	  if(verbosity==kDEBUG) std::cout<<"ERROR(analyzer): extra hits found even after TS X-ing check in the event "<<OutEvt_int.idx<<std::endl;
	  extraHit->AddEvent(prev_OutEvt);
	  if(verbosity==kDEBUG) std::cout<<"ERROR(analyzer): Printing the extra hits left in the event "<<OutEvt_int.idx<<std::endl;
	  if(verbosity==kDEBUG) OutEvt_int.Print();	
	}
      }
    }

    if(OutEvt_int.GetSize()>0){
      //quando trovo delle extra hit in uscita, potrebbero risultate generate nel TS successivo. 
      //Escono con il TS precedente a causa del ritardo di propagazione del segnale TSCNT sulla matrice.
      //NON asserisco ret=false, al prossimo giro controllo se trovo delle TS X-ing hits.
      //(avro' delle hit lost e quindi GenEvt_int.GetSize()>0)
      if(verbosity==kDEBUG) std::cout<<"WARNING(analyzer): extra hits in event id "<<OutEvt_int.idx<<std::endl;
      extraHit->AddEvent(OutEvt_int);
      if(verbosity==kDEBUG) std::cout<<"WARNING(analyzer): Printing the extra hits in the event "<<OutEvt_int.idx<<std::endl;
      if(verbosity==kDEBUG) OutEvt_int.Print();	
    }
  } else if (!ret){
    if (GenEvt_int.GetSize()>0){
      if(verbosity==kDEBUG) std::cout<<"WARNING(analyzer): hits are lost at event "<<GenEvt_int.idx<<" evt size"<<GenEvt_int.GetSize()<<std::endl;
      lostHit->AddEvent(GenEvt_int);
      if(verbosity==kDEBUG) std::cout<<"WARNING(analyzer): Printing the hits lost in the event "<<GenEvt_int.idx<<std::endl;
      if(verbosity==kDEBUG) GenEvt_int.Print();
    } 
    if(OutEvt_int.GetSize()>0){
      if(verbosity==kDEBUG) std::cout<<"WARNING(analyzer): extra hits found at event "<<OutEvt_int.idx<<" evt size "<<OutEvt_int.GetSize()<<std::endl;
      extraHit->AddEvent(OutEvt_int);
      if(verbosity==kDEBUG) std::cout<<"WARNING(analyzer): Printing the extra hits left in the event "<<OutEvt_int.idx<<std::endl;
      if(verbosity==kDEBUG) OutEvt_int.Print();	
    } 
  }
	
*/
  return ret;
}
////////////////////////////////////////////////////////////////////////////////////////
//		XCheck
//Check the matrixData m_data (member of analyzer) with an external matrixData object. 
bool analyzer::XCheck(matrixData *simGenData, matrixData *lostHit, matrixData  *extraHit){
  if(verbosity==kMEDIUM || verbosity==kDEBUG) std::cout<<"INFO(analyzer): Xcheck started"<<std::endl;	
	
  _EVENT GenEvt, OutEvt;
  GenEvt.Reset();
  OutEvt.Reset();
  bool good = true;
	
  //Check SAME number of events
  if (m_data->GetSize() != simGenData->GetSize()) {
    std::cout<<"WARNING(analyzer): Xcheck -> different matrix data size "<<m_data->GetSize()
							 <<", gen data size "<<simGenData->GetSize()<<". Analysis will  be performed anyway, keep sharp eyes."<<std::endl;	
  }		
  simGenData->ResetReadPointer();
  m_data->ResetReadPointer();
  

  /* resume table */
  int Ncol = 3;
  int Wcol = 20;
  const char mark = '*';
  /****************************************************************/
  std::cout.fill(mark);
  std::cout<< std::setw(Ncol*Wcol+4+(Ncol-1)*3) << "" <<std::endl;
  std::cout.fill(' ');

  std::cout<< mark << " " << std::setw(Wcol*2+3) <<"Xcheck: Gen Vs Out data" << std::setw(Wcol+4)  <<" "<< mark<< std::endl;
 
  /****************************************************************/
  std::cout.fill(mark);
  std::cout<< std::setw(Ncol*Wcol+4+(Ncol-1)*3) << "" <<std::endl;
  std::cout.fill(' ');

  std::cout<< mark << " " << std::setw(Wcol) << " "            <<" "<<mark<<" "<< std::setw(Wcol) << "Output Data"       <<" "<<mark<<" "<< std::setw(Wcol)  <<"Generated Data"        <<" "<< mark<< std::endl;
 /****************************************************************/
  std::cout.fill(mark);
  std::cout<< std::setw(Ncol*Wcol+4+(Ncol-1)*3) << "" <<std::endl;
  std::cout.fill(' ');
 
  std::cout<< mark << " " <<std::setw(Wcol) << "Tot Hits"      <<" "<<mark<<" "<< std::setw(Wcol) << m_data->CountHits() <<" "<<mark<<" "<< std::setw(Wcol)  << simGenData->CountHits() <<" "<<mark<< std::endl;
 /****************************************************************/
  std::cout.fill(mark);
  std::cout<< std::setw(Ncol*Wcol+4+(Ncol-1)*3) << "" <<std::endl;
  std::cout.fill(' ');

  std::cout<< mark << " " <<std::setw(Wcol) << "Tot Events"    <<" "<<mark<<" "<< std::setw(Wcol) << m_data->GetSize()   <<" "<<mark<<" "<< std::setw(Wcol)  << simGenData->GetSize()   <<" "<<mark<<" "<< std::endl;

  /****************************************************************/
  std::cout.fill(mark);
  std::cout<< std::setw(Ncol*Wcol+4+(Ncol-1)*3) << "" <<std::endl;
  std::cout.fill(' ');


  
	
  while(simGenData->GetNextEvent(GenEvt)){ 
    if(!m_data->GetNextEvent(OutEvt)){
      std::cout<<"ERROR(analyzer):Could not read output data"<<std::endl;
      return false;
    };
    //check the events
    if (!XCheck_Events(GenEvt, OutEvt, lostHit, extraHit)) good=false;
  }

  return good;
}

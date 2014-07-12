#include <vector>
#include <iostream> 
#include <iomanip>
#include <iomanip>

#ifdef ROOT_SUPPORT
 #include <TQObject.h>
 #include <TH2I.h>
 #include <TH1I.h>
 #include <TTree.h>
 #include <TFile.h>
 #include <TKey.h>
 #include <TCanvas.h>
#endif


//anal. tools
#include "matrixData.h"
#include "parser.h"
#include "clusters.h"

//da fare:
//1) il Xcheck delle hit (potenziare parser per altri tipi di file)
//2) il Xcheck delle hit con trigger
//4) l'interfaccia di standard output
//5) fai una ignore list per ignorare certi pixel

#ifndef __VERBOSITY__
#define __VERBOSITY__
enum _VERBOSITY {
  kSILENT = 0,
  kMEDIUM = 1,
  kDEBUG  = 2
};
#endif


#ifndef __ANALYZER__
#define __ANALYZER__


//////DEFINE ANALYZER VERSION HERE//////////
const double VERSION = 0.3;
static const unsigned int SRTX_MAT_X_SIZE=200;
static const unsigned int SRTX_MAT_Y_SIZE=256;
static const unsigned int A4D_MAT_X_SIZE=128;
static const unsigned int A4D_MAT_Y_SIZE=32;
static const unsigned int SPX0_MAT_X_SIZE=128;
static const unsigned int SPX0_MAT_Y_SIZE=32;


typedef struct{
  int C1;
  int C2;
  int HV;
  int magn;
  int cam_length;
  int current;
  void Reset();
}_MICRO_PARAMETERS;

void _MICRO_PARAMETERS::Reset(){
  C1=-1;
  C2=-1;
  HV=-1;
  magn=-1;
  cam_length=-1;
  current=-1;
}

using namespace std;

class analyzer{

 private:
  double TimeRes; //BC period (time resolution) in us 
  int Run_number;
  double Version;
  uint32_t Time;
  uint32_t lastTS;
  bool triggered;
  unsigned int trigger_latency; //trigger latency in ps
  _MICRO_PARAMETERS parameters;
  _VERBOSITY verbosity; 	
  matrixData * m_data;
  matrixData * buffer_data;
  // to be added as class memeber
  //	matrixData * gen_data;
  //	matrixData * lostHit;
  //	matrixData * extraHit;
  int TSXingHits;	
  clusters * c_data;	
  int outputHits;
  int genHits;

 public:
  analyzer();
  analyzer(int RN);
  analyzer(int RN, double TRes);
  analyzer(int RN, double TRes, matrixData & m_data);
  analyzer(int RN, double TRes, matrixData & m_data, matrixData & buffer_data);
  void Init();
  void SetVerbosity(_VERBOSITY level);
  bool AttachMData(matrixData  &data);
  bool matrixDataIsAttached()  { return m_data != 0 ? true : false;  }
  bool bufferDataIsAttached()  { return buffer_data != 0 ? true : false;  }
  bool AttachCData(clusters &data);
  bool AttachBufData(matrixData  &data);
  bool clusterDataIsAttached()  { return c_data != 0 ? true : false;  }
  void ResetParameters();
  void SetMicroParameters(int C1, int C2, int HV, int magn, int cam_length, int current);
  void SetTimeRes(double x);
  void SetTriggerLatency(unsigned int x){trigger_latency = x;}
  void SetRunNumber(int RN);
  void SetTriggeredFlag(int x);
  double GetTimeRes();
  int GetRunNumber();
  double EvalRate();

  int GetOutputHits(){return outputHits;};
  int GetGenHits(){return genHits;};
  int GetTSXingHits();
  bool HitProximity(_HIT & p1, _HIT & p2);
  bool ClusterProximity(_CLUSTER & c1, _CLUSTER & c2);
  bool ClusterCopyHits(_CLUSTER & c1, _CLUSTER & c2);
  bool ClusterMerge(_CLUSTER & source, _CLUSTER & dest);
  double EvalClusterXcm(_CLUSTER & c1);
  double EvalClusterYcm(_CLUSTER & c1);
  int EvalClusterXspread(_CLUSTER & c1);
  int EvalClusterYspread(_CLUSTER & c1);
  int Get2DManhattanDistance(_HIT p1, _HIT p2);
  int FindClusters(_EVENT & event, _CLUSTER_EVENT & cluster_event);
  bool FindAllClusters();
  bool ParseOutputData(ifstream & file, _DATATYPE DataType);
  bool ParseSimGenData(ifstream & file, ifstream & trig_file, matrixData *simGenData);
  bool ParseBufferData(ifstream & file, ifstream & trig_file);
  
  bool CheckEvent(_EVENT & evt);
  bool CheckMatrixData(matrixData *matrix_data);
  bool XCheck_Events(_EVENT & evt1, _EVENT & evt2, uint32_t & delta);
  bool XCheck_Events(_EVENT & evt1, _EVENT & evt2, matrixData *lostHit, matrixData  *extraHit);
  bool XCheck(matrixData *simGenData, matrixData *lostHit, matrixData  *extraHit);
  #ifdef ROOT_SUPPORT
  bool AnalyzeCData(_DATATYPE DataType);
  bool RootPlots();
  #endif
};





#endif

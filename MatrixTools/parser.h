#include <vector>
#include <iostream>
#include <fstream>
#include <string>

//anal. tools
#include "matrixData.h"

#ifndef __VERBOSITY__
#define __VERBOSITY__
enum _VERBOSITY {
  kSILENT = 1,
  kMEDIUM = 2,
  kDEBUG  = 3,
  INVALID_VerbosityLevel = 4,
  _VERBOSITYfirst = kSILENT,
  _VERBOSITYlast = kDEBUG
};
#endif

#ifndef __CHIP_PROPERTIES__
#define __CHIP_PROPERTIES__
struct _CHIP_PROPERTIES{
  std::string   chipName;
  uint32_t chipID;
  uint32_t mat_X_size;
  uint32_t mat_Y_size;
  uint32_t submat_X_size;
  uint32_t submat_Y_size;
  uint32_t NX_submat;
  uint32_t NY_submat;
};
#endif

#ifndef __DATATYPE__
#define __DATATYPE__
enum _DATATYPE {
  simuSORTEX200x256     = 1, 
  simuSQARE200x256      = 2, 
  FE4DtestPISA          = 3, 
  APSEL4DmicroBOLOGNA   = 4,
  simuSQARE2x16x32      = 5,
  simuGenList           = 6,
  DAQLayerDump          = 7,
  StripFEBuffer         = 8,
  simuSQARE2x48x128     = 9,
  simuSQARE2x48x128raw  = 10,
  simuSQARE2x16x128     = 11,
  simuSQARE2x16x128raw  = 12,
  INVALID_DataType      = 13,
  _DATATYPEfirst        = simuSORTEX200x256,
  _DATATYPElast         = simuSQARE2x16x128raw
};//NB!!! remember to update the static listDataTypes, getDataTypeName and setProperties functions
#endif
 
#ifndef __SQUAREHEADER__
#define __SQUAREHEADER__
struct _SQUAREHEADER {
  int submatr_addr;
  int parity;
  int hitRejectB1;
  int sweepJump;
  int hitRejectB2;
  int dummy;
  int TS; 
};
#endif


#ifndef __SQUAREHIT__
#define __SQUAREHIT__
struct _SQUAREHIT {
  int submatr_addr;
  int B2_addr;
  int X_address;
  int Y_address;
};
#endif

#ifndef __TRIG_DATA_LINE__
#define __TRIG_DATA_LINE__
struct _TRIG_DATA_LINE {
  int trig;
  int TS;
  uint64_t  sim_time;
  std::string tres;
};
#endif

#ifndef __BUF_DATA_LINE__
#define __BUF_DATA_LINE__
struct _BUF_DATA_LINE {
  uint64_t idx;
  unsigned int strip;
  int TS;
  uint64_t  sim_time;
  std::string tres;
};
#endif

#ifndef __GEN_DATA_LINE__
#define __GEN_DATA_LINE__
struct _GEN_DATA_LINE {
  uint64_t idx;
  uint32_t x;
  uint32_t y; 
  uint64_t sim_time;
  std::string tres;
};
#endif


#ifndef __PARSER__
#define __PARSER__
////////////////////////////////////APSEL4D
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

/////////////////////////////////////DAQ DUMP
#define DAQLayerDumpTS_MASK  0x00FF0000
#define DAQLayerDumpX_MASK   0x000000FE
#define DAQLayerDumpY_MASK   0x00001F00
#define DAQLayerDumpEoE_MASK 0x00000001
#define DAQLayerDumpDAC_MASK 0x00001FFE


#define DAQLayerDumpGetTS(x)  ((x & DAQLayerDumpTS_MASK )>>16)
#define DAQLayerDumpGetX(x)   ((x & DAQLayerDumpX_MASK )>>1)
#define DAQLayerDumpGetY(x)   ((x & DAQLayerDumpY_MASK )>>8)
#define DAQLayerDumpIsEoE(x)  ((x & DAQLayerDumpEoE_MASK ))
#define DAQLayerDumpGetDAC(x) ((x & DAQLayerDumpDAC_MASK )>>1)

////////////////////////////////////SQARE2x48x128 
#define SQARE2x48x128_TSorHIT_MASK 0x8000

#define SQARE2x48x128_PARITY_MASK  0x4000
#define SQARE2x48x128_HRB1_MASK    0x2000

#define SQARE2x48x128_SJ_MASK      0x1000
#define SQARE2x48x128_HRB2_MASK    0x0800
#define SQARE2x48x128_SUBMAT_ID    0x0100
#define SQARE2x48x128_TS           0x00FF

#define SQARE2x48x128_SPARS_ADDR   0x6000
#define SQARE2x48x128_YZONE_ADDR   0x1C00
#define SQARE2x48x128_X_ADDR       0x03F0
#define SQARE2x48x128_Z_PATTERN    0x000F

///////////////////////////////////SQARE2x16x128
#define SQARE2x16x128_TSorHIT_MASK 0x2000

#define SQARE2x16x128_PARITY_MASK  0x1000
#define SQARE2x16x128_HRB1_MASK    0x0800

#define SQARE2x16x128_SJ_MASK      0x0400
#define SQARE2x16x128_HRB2_MASK    0x0200
#define SQARE2x16x128_SUBMAT_ID    0x0100
#define SQARE2x16x128_TS           0x00FF

#define SQARE2x16x128_SPARS_ADDR   0x1800
#define SQARE2x16x128_YZONE_ADDR   0x0700
#define SQARE2x16x128_X_ADDR       0x00F0
#define SQARE2x16x128_Z_PATTERN    0x000F

///////////////////////////////////EVENT FLAGS 
#define EVTflags_HRB2_MASK         0x1
#define EVTflags_SJ_MASK           0x2
#define EVTflags_HRB1_MASK         0x4


//#define EVTflags_SQARE2x48x128_HRB1_MASK0 0x4
//#define EVTflags_SQARE2x48x128_HRB1_MASK1 0x400
//#define EVTflags_SQARE2x48x128_SJ_MASK0   0x2
//#define EVTflags_SQARE2x48x128_SJ_MASK1   0x200
//#define EVTflags_SQARE2x48x128_HRB2_MASK0 0x1
//#define EVTflags_SQARE2x48x128_HRB2_MASK1 0x100


/* #define SQARE2x48x128_IS_TS(x)       ((x & SQARE2x48x128_TSorHIT_MASK)>>15) */
/* #define SQARE2x48x128getSUBMAT_ID(x) ((x & SQARE2x48x128_SUBMAT_ID )>>14) */
/* #define SQARE2x48x128getPARITY(x)    ((x & SQARE2x48x128_PARITY_MASK)>>13) */
/* #define SQARE2x48x128getHRB1(x)      ((x & SQARE2x48x128_HRB1_MASK)>>12) */
/* #define SQARE2x48x128getSJ(x)        ((x & SQARE2x48x128_SJ_MASK)>>11) */
/* #define SQARE2x48x128getHRB2(x)      ((x & SQARE2x48x128_HRB2_MASK)>>10) */
/* #define SQARE2x48x128getTS(x)        ( x & SQARE2x48x128_TS) */

using namespace std;

class parser{
  
private:
  uint32_t idx; //index of events
  uint32_t  hits_found;
  uint32_t Time;
  uint32_t lastTS;
  uint32_t lastSM_ID;
  bool triggered;
  uint32_t trig_latency; //trigger latency in ps
  double TimeRes;      //BC period (time resolution) in us
  _DATATYPE dataType;  
  _CHIP_PROPERTIES chipProperties;
  _VERBOSITY verbosity;
  uint32_t getBClat();
  uint32_t getMaskedValue(uint32_t mask, uint32_t val);

public:
  
  ifstream*  outFile;
  ifstream* trigFile;
  parser(ifstream & file);
  parser();
  //	virtual ~parser(){};
  void Init();
  static void listDataTypes();
  unsigned int from_gray_conv(unsigned int TS);
  bool and_array(vector<bool> & x);
  bool OutputfileIsAttached()  { return outFile != 0 ? true : false;  }
  bool TrigfileIsAttached()  { return trigFile != 0 ? true : false;  }	
  bool attachOutputFile(ifstream &file);
  bool attachTrigFile(ifstream &file);
  bool findEvent(_EVENT & evt);
  void setDataType(_DATATYPE DataType);
  void setVerbosity(_VERBOSITY level);
  void setTrigLatency(unsigned int lat);
  //void setBClat(int x);
  void setTimeRes(double x);
  void setTriggeredFlag(int x);
  bool fillMatrixData(matrixData * m_data);
  void fileRewind();
  bool CheckOutputFile(_DATATYPE DataType);
  bool Read_SQ_Header(ifstream & file,  _SQUAREHEADER & SQ_header);
  bool Read_RAW_SQ_TS(uint32_t data, _DATATYPE type,  _SQUAREHEADER & SQ_header);
  bool Read_RAW_SQ_HITS(uint32_t data,_DATATYPE type,  int submatID, vector<_HIT> &  hits );
  bool Read_SQ_Hit(ifstream & file, _SQUAREHIT & SQ_hit);
  bool Check_SQ_TS(unsigned int N_submat, int TS, vector<int> & TS_buff);
  bool CheckGenFile();
  bool ReadTDline(_TRIG_DATA_LINE &TDline);
  bool ReadBDline(_BUF_DATA_LINE &BDline);
  bool ReadGDline(_GEN_DATA_LINE &GDline);
  static bool getChipProperties(_DATATYPE type, _CHIP_PROPERTIES & ChipProp);
  bool setChipProperties(_DATATYPE type); 
  bool setChipProperties(string typeName, uint32_t typeID, uint32_t submat_X_size, uint32_t submat_Y_size, uint32_t NX_submat, uint32_t NY_submat);
  int GetIdx();
  int GetTime();
  int GetHitsFound();
  int GetLastTS();
  static _DATATYPE getDataTypeName(int n, char * text = 0);
  static _VERBOSITY getVerbosityLevelName(int n, char * text = 0);
  void fillInMatrixSize(matrixData *mdata);
  static uint32_t getMatrixWidth(_DATATYPE type);
  static uint32_t getMatrixHeight(_DATATYPE type);
  uint32_t getMatrixWidth();
  uint32_t getMatrixHeight();

};


#endif






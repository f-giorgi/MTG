#include <stdint.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

 
#ifndef __MATRIXDATA__
	#define __MATRIXDATA__

typedef struct{
		uint32_t X_address;
		uint32_t Y_address;
		void Print();
		void Print(int indent);
}_HIT;


void _HIT::Print(){
	std::cout<<"X:"<<std::setw(4)<<X_address<<"     Y:"<<std::setw(4)<<Y_address<<std::endl;
}


void _HIT::Print(int indent){
	//TAB(indent);
	std::cout<<"X:"<<std::setw(4)<<X_address<<"     Y:"<<std::setw(4)<<Y_address<<std::endl;
}


typedef struct{
  uint64_t idx;
  uint32_t TimeStamp;
  uint64_t AbsTime;
  uint32_t flags;
  std::vector<_HIT> hits;
  void Reset();
  void Print();
  void Print(std::ostream & outstream);
  //void Print(int indent);
  void AddHit(int X, int Y);
  void sortXY();
  uint32_t to_gray_conv(uint32_t TS);
  int GetSize(){return hits.size();}
}_EVENT;

void _EVENT::Reset(){
	TimeStamp=-1;
	hits.clear();
	idx=-1;
	AbsTime=-1;
  flags = 0;
}

void _EVENT::Print(std::ostream & outstream){
  	outstream<<"Event ID   "<<idx<<std::endl;
    outstream<<"   Time Stamp "<<TimeStamp<<"   0x"<<std::hex<<TimeStamp<<"  Gray code 0x"<<to_gray_conv(TimeStamp)<<std::dec<<std::endl;
	outstream<<"   Abs. Time  "<<AbsTime<<std::endl;	
  outstream<<"   Flags "<<std::hex<<flags<<std::dec<<std::endl;
	outstream<<"   Hit List:  "<<std::endl;		
	for (unsigned int j=0; j<hits.size(); j++){		
	  outstream<<"       X:"<<std::setw(4)<<hits[j].X_address<<"     Y:"<<std::setw(4)<<hits[j].Y_address<<std::endl;	
	}
	outstream<<"\n";
}

void _EVENT::Print(){
  //print on the strandard output
  std::ofstream mycout;
  mycout.copyfmt(std::cout);                                  //1
  mycout.clear(std::cout.rdstate());                          //2
  mycout.std::basic_ios<char>::rdbuf(std::cout.rdbuf());      //3
  Print(mycout);
}

/* void _EVENT::Print(int indent){ */
/* 	TAB(indent); */
/* 	std::cout<<"Event ID   "<<idx<<std::endl; */
/* 	TAB(indent); */
/* 	std::cout<<"Time Stamp "<<TimeStamp<<std::endl; */
/* 	TAB(indent); */
/* 	std::cout<<"Abs. Time  "<<AbsTime<<std::endl;	 */
/* 	TAB(indent); */
/* 	std::cout<<"Hit List:  "<<std::endl;		 */
/* 	for (unsigned int j=0; j<hits.size(); j++){ */
/* 		TAB(indent); */
/* 		TAB(indent); */
/* 		std::cout<<"X:"<<std::setw(4)<<hits[j].X_address<<"     Y:"<<std::setw(4)<<hits[j].Y_address<<std::endl;	 */
/* 	} */
/* } */

void _EVENT::AddHit(int X, int Y){
	_HIT hit;
	hit.X_address=X;
	hit.Y_address=Y;
	hits.push_back(hit);
}

void _EVENT::sortXY(){
  bool sorted = true;
  _HIT hitBuf;

  if (hits.size()>1){
    while(sorted){
      sorted = false;
      for (uint32_t i = 0; i<(hits.size()-1); i++){
	if (hits[i].X_address > hits[i+1].X_address){
	  hitBuf    = hits[i];
	  hits[i]   = hits[i+1];
	  hits[i+1] = hitBuf; 
	  sorted    = true;
	} else if (hits[i].X_address == hits[i+1].X_address){
	  if (hits[i].Y_address > hits[i+1].Y_address){
	    hitBuf    = hits[i];
	    hits[i]   = hits[i+1];
	    hits[i+1] = hitBuf; 
	    sorted    = true;
	  }
	}
      }
    }    
  }
}

uint32_t _EVENT::to_gray_conv(uint32_t TS){
  uint32_t mask		      = 0x80;    
  //uint32_t bit_shifter	= TS & mask;	
  uint32_t result	      = TS & mask;

  for (int i=6; i>=0; i--){
    result |= ((TS & mask) >> 1) ^ (TS & (mask>>1));
    mask = mask >> 1;
  }
  return result;
}



typedef std::vector<_EVENT> _EVENTBUFFER;



class matrixData{
	private:
	_EVENTBUFFER event_buffer;
	uint32_t read_pointer;
  uint32_t Xsize;
  uint32_t Ysize;
	
	public:
	//constructor
	matrixData(_EVENTBUFFER & data);
	matrixData();
	virtual ~matrixData(){delete &event_buffer;};
	void Init(); 
	void AddEvent(_EVENT & event);
	bool GetEvent(uint32_t index, _EVENT & event);
	bool GetLastEvent(_EVENT & event);
	bool GetNextEvent(_EVENT & event);
	bool GetPrevEvent(_EVENT & event);
	void Pop_BackEvent();
	void ResetReadPointer();
	void SetReadPointer(uint32_t idx);
	int GetReadPointer();
	int GetSize();
	int CountHits();
	void SortXY();
	void Print();
	void Print(uint32_t n);
	//void Print(uint32_t indent);
	//void Print(uint32_t indent , uint32_t n);
	void Print(std::ostream & outstream);
	void Print(std::ostream & outstream, uint32_t n);
  void PrintAll(std::ostream & outstream);
  void PrintAll(std::ostream & outstream, uint32_t n);
  void SetMatrixSize(uint32_t x, uint32_t y);
  uint32_t GetMatrixXSize();
  uint32_t GetMatrixYSize();

};


#endif


















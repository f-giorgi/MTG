#include <vector>
#include <iostream> 
#include <iomanip>
//#include <TQObject.h>
//anal. tools
#include "matrixData.h"


#ifndef __CLUSTERS__
	#define __CLUSTERS__


typedef struct{
	uint32_t mult;
	std::vector<_HIT> hits;
	double Xcm;
	double Ycm;
	uint32_t Xspread;
	uint32_t Yspread;
	void Reset();
	void AddHit(int X, int Y);
	void Print();
	void Print(int indent);
	int GetSize(){ return hits.size();}
	static void TAB(int indent){ for(int tab=0; tab<indent; tab++) std::cout<<" ";	}
}_CLUSTER;  

void _CLUSTER::Reset(){
	hits.clear();
	Xcm=-1.;
	Ycm=-1.;
	mult=0;
	Xspread=0;
	Yspread=0;
}
 
void _CLUSTER::AddHit(int X, int Y){
	_HIT hit;
	hit.X_address=X;
	hit.Y_address=Y;
	hits.push_back(hit);
}


void _CLUSTER::Print(){
	std::cout<<"Cluster Multiplicity =  "<<mult<<std::endl;
	std::cout<<"Cluster hit list "<<std::endl;
	for (uint32_t i=0; i < hits.size(); i++){
		TAB(2);
		hits[i].Print();
	}
	std::cout<<"Xcm = "<<Xcm<<std::endl;
	std::cout<<"Ycm = "<<Ycm<<std::endl;
	std::cout<<"Xspread = "<<Xspread<<std::endl;	
	std::cout<<"Yspread = "<<Yspread<<std::endl;	
}


void _CLUSTER::Print(int indent){
	TAB(indent);
	std::cout<<"Cluster Multiplicity =  "<<mult<<std::endl;
	
	TAB(indent);
	std::cout<<"Cluster hit list "<<std::endl;
	for (uint32_t i=0; i < hits.size(); i++){
		TAB(indent);
		TAB(2);
		hits[i].Print();
	}
	
	TAB(indent);
	std::cout<<"Xcm = "<<Xcm<<std::endl;
	
	TAB(indent);
	std::cout<<"Ycm = "<<Ycm<<std::endl;
	
	TAB(indent);
	std::cout<<"Xspread = "<<Xspread<<std::endl;
	
	TAB(indent);
	std::cout<<"Yspread = "<<Yspread<<std::endl;	
}


typedef struct {
	std::vector<_CLUSTER> cluster_buffer;
	uint32_t idx;
	uint32_t TimeStamp;
	uint32_t AbsTime;
	double RealTime;
	uint32_t nclust;
	uint32_t av_dist;
	void Reset();
	int GetSize(){return cluster_buffer.size();}
}_CLUSTER_EVENT;


void _CLUSTER_EVENT::Reset(){
	cluster_buffer.clear();
	idx=-1;
	TimeStamp=-1;
	AbsTime=-1;
	RealTime=-1.;
	nclust=0;
	av_dist=-1;
}

typedef std::vector<_CLUSTER_EVENT> _CLUSTER_EVENT_BUFFER;


class clusters{
	private:
	_CLUSTER_EVENT_BUFFER cluster_event_buffer;
	uint32_t read_pointer;
	
	public:
	clusters();
	clusters(_CLUSTER_EVENT_BUFFER & data);
	void Init();
	void AddClusterEvent(_CLUSTER_EVENT & cluster_event);
	bool GetClusterEvent(uint32_t index, _CLUSTER_EVENT & cluster_event);
	bool GetNextEvent(_CLUSTER_EVENT & cluster_event);
	void ResetReadPointer();
	int GetReadPointer();
	int GetSize();
	int CountHits();
	void Print();
	void Print(uint32_t indent);
	void Print(uint32_t indent, uint32_t n);
};




#endif

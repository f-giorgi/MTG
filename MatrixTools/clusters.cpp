#include "clusters.h"

clusters::clusters(){
	Init();
}

clusters::clusters(_CLUSTER_EVENT_BUFFER & data){
	Init();
	cluster_event_buffer = data;
}

void clusters::Init(){
	cluster_event_buffer.clear();
	read_pointer=0;
}

void clusters::AddClusterEvent(_CLUSTER_EVENT & cluster_event){
	cluster_event_buffer.push_back(cluster_event);
}

bool clusters::GetClusterEvent(uint32_t index, _CLUSTER_EVENT & cluster_event){
	if (index >= cluster_event_buffer.size())return false;
	else {
		cluster_event = cluster_event_buffer[index];
		return true;
	}
}

bool clusters::GetNextEvent(_CLUSTER_EVENT & cluster_event){
	if (read_pointer<cluster_event_buffer.size()) {
		cluster_event = cluster_event_buffer[read_pointer];
		read_pointer++;
		return true;
	}else{
		std::cout<<"no more events"<<std::endl;
		return false;
	}
}

void clusters::ResetReadPointer(){
	std::cout<<"clusters class read pointer reset"<<std::endl;
	read_pointer=0;
}

int clusters::GetReadPointer(){
	return read_pointer;
}

int clusters::GetSize(){
	return cluster_event_buffer.size();
}

int clusters::CountHits(){
	int count=0;
	for (int i=0; i<GetSize(); i++){
		for ( int j=0; i<cluster_event_buffer[i].GetSize(); j++){ 	
			count += cluster_event_buffer[i].cluster_buffer[j].GetSize();
		}
	}
	return count;
}



void clusters::Print(){
	_CLUSTER_EVENT cluster_event;
	cluster_event.Reset();

	std::cout<<"========PRINTING ALL CLUSTER EVENTS========"<<std::endl;
	for(uint32_t i=0; i<cluster_event_buffer.size(); i++){
		std::cout<<"Cluster Event ID  "<< cluster_event_buffer[i].idx<<std::endl;
		std::cout<<"Time Stamp       "<< cluster_event_buffer[i].TimeStamp<<std::endl;
		std::cout<<"Absolute Time    "<< cluster_event_buffer[i].AbsTime<<std::endl;
		std::cout<<"Real Time        "<< cluster_event_buffer[i].RealTime<<std::endl;
		std::cout<<"N clusters       "<< cluster_event_buffer[i].nclust<<std::endl;	
		std::cout<<"Average distance "<< cluster_event_buffer[i].av_dist<<std::endl;
		for(uint32_t j=0; j< cluster_event_buffer[i].cluster_buffer.size(); j++){
      _CLUSTER::TAB(2);
			std::cout<<"--------------------        "<<std::endl;
			_CLUSTER::TAB(2);
			std::cout<<"Cluster N        "<<j<<std::endl;
			cluster_event_buffer[i].cluster_buffer[j].Print(2);
		}
		std::cout<<"==================="<<std::endl;
	}
}
 
void clusters::Print(uint32_t indent){
	_CLUSTER_EVENT cluster_event;
	cluster_event.Reset();
	_CLUSTER::TAB(indent);
	std::cout<<"========PRINTING ALL CLUSTER EVENTS========"<<std::endl;
	for(uint32_t i=0; i<cluster_event_buffer.size(); i++){
		_CLUSTER::TAB(indent);
		std::cout<<"Cluster Event ID  "<< cluster_event_buffer[i].idx<<std::endl;
		_CLUSTER::TAB(indent);
		std::cout<<"Time Stamp       "<< cluster_event_buffer[i].TimeStamp<<std::endl;
		_CLUSTER::TAB(indent);
		std::cout<<"Absolute Time    "<< cluster_event_buffer[i].AbsTime<<std::endl;
		_CLUSTER::TAB(indent);
		std::cout<<"Real Time        "<< cluster_event_buffer[i].RealTime<<std::endl;
		_CLUSTER::TAB(indent);
		std::cout<<"N clusters       "<< cluster_event_buffer[i].nclust<<std::endl;	
		_CLUSTER::TAB(indent);
		std::cout<<"Average distance "<< cluster_event_buffer[i].av_dist<<std::endl;
		for(uint32_t j=0; j< cluster_event_buffer[i].cluster_buffer.size(); j++){
			_CLUSTER::TAB(2*indent);
			std::cout<<"--------------------        "<<std::endl;
			_CLUSTER::TAB(2*indent);
			std::cout<<"Cluster N        "<<j<<std::endl;
			cluster_event_buffer[i].cluster_buffer[j].Print(2*indent);
		}
		_CLUSTER::TAB(indent);
		std::cout<<"==================="<<std::endl;
	}
}
 
void clusters::Print(uint32_t indent, uint32_t n){
	_CLUSTER_EVENT cluster_event;
	cluster_event.Reset();
	_CLUSTER::TAB(indent);
	std::cout<<"========PRINTING the first 50 CLUSTER EVENTS========"<<std::endl;
	for(uint32_t i=0; i<n && cluster_event_buffer.size(); i++){
		_CLUSTER::TAB(indent);
		std::cout<<"Cluster Event ID  "<< cluster_event_buffer[i].idx<<std::endl;
		_CLUSTER::TAB(indent);
		std::cout<<"Time Stamp       "<< cluster_event_buffer[i].TimeStamp<<std::endl;
		_CLUSTER::TAB(indent);
		std::cout<<"Absolute Time    "<< cluster_event_buffer[i].AbsTime<<std::endl;
		_CLUSTER::TAB(indent);
		std::cout<<"Real Time        "<< cluster_event_buffer[i].RealTime<<std::endl;
		_CLUSTER::TAB(indent);
		std::cout<<"N clusters       "<< cluster_event_buffer[i].nclust<<std::endl;	
		_CLUSTER::TAB(indent);
		std::cout<<"Average distance "<< cluster_event_buffer[i].av_dist<<std::endl;
		for(uint32_t j=0; j< cluster_event_buffer[i].cluster_buffer.size(); j++){
			_CLUSTER::TAB(2*indent);
			std::cout<<"--------------------        "<<std::endl;
			_CLUSTER::TAB(2*indent);
			std::cout<<"Cluster N        "<<j<<std::endl;
			cluster_event_buffer[i].cluster_buffer[j].Print(2*indent);
		}
		_CLUSTER::TAB(indent);
		std::cout<<"==================="<<std::endl;
	}
}

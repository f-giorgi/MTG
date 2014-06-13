#include "matrixData.h"


matrixData::matrixData(_EVENTBUFFER & data){
	Init();
	event_buffer = data;
}

matrixData::matrixData(){
	Init();
}


void matrixData::Init(){
	event_buffer.clear();
	read_pointer = 0;
  Xsize = 1;
  Ysize = 1;
}
 

void matrixData::Pop_BackEvent(){
		event_buffer.pop_back();	
}

bool matrixData::GetNextEvent(_EVENT & event){
	if (read_pointer<event_buffer.size()) {
		event = event_buffer[read_pointer];
		read_pointer++;
		return true;
	}else{
		//std::cout<<"no more events"<<std::endl;
		return false;
	}
}

bool matrixData::GetPrevEvent(_EVENT & event){
	if (read_pointer>0) {
		event = event_buffer[read_pointer-2];
		read_pointer--;
		return true;
	}else{
		//std::cout<<"no more events"<<std::endl;
		return false;
	}
}

bool matrixData::GetEvent(uint32_t index, _EVENT & event){
	if (index >= event_buffer.size())return false;
	else {
		event = event_buffer[index];
		return true;
	}
}

void matrixData::ResetReadPointer(){
	//std::cout<<"matrixData read pointer reset"<<std::endl;
	read_pointer=0;
}

void matrixData::SetReadPointer(uint32_t idx){
	//std::cout<<"matrixData read pointer reset"<<std::endl;
	read_pointer=idx;
}

void matrixData::AddEvent(_EVENT & event){
	event_buffer.push_back(event);
}

int matrixData::GetReadPointer(){
	return read_pointer;
}

int matrixData::GetSize(){

return event_buffer.size();
}

bool matrixData::GetLastEvent(_EVENT & event){
	if (event_buffer.size()==0){
	  std::cout<<"WARNING(matrixData): requested last event of an empty matrixData object"<<std::endl;
		return false;
	}
	event=event_buffer[event_buffer.size()-1];//get the last element in array
	//if (&event != 0) return true; else return false;
	return true;
}

 
void matrixData::SortXY(){
  std::cout<<"INFO(matrixData): Sorting event buffer"<<std::endl;
  for (uint32_t i =0; i<event_buffer.size(); i++){
    event_buffer[i].sortXY();
  }
}

void matrixData::Print(uint32_t n){
  std::ofstream mycout;

  mycout.copyfmt(std::cout);                                  //1
  mycout.clear(std::cout.rdstate());                          //2
  mycout.std::basic_ios<char>::rdbuf(std::cout.rdbuf());      //3
  Print(mycout, n);
 
}

void matrixData::Print(){ 
  Print(0);
}

 
void matrixData::Print(std::ostream & outstream){
  Print(outstream, 0);
}

void matrixData::Print(std::ostream & outstream, uint32_t n ){
  if (n==0) n=event_buffer.size();
  outstream<<"========PRINTING MATRIX DATA========"<<std::endl;
	for (uint32_t i =0; i<n && i<event_buffer.size(); i++){
	  if(event_buffer[i].GetSize()!= 0) event_buffer[i].Print(outstream);
	  
		/* //print event tags */
/* 		outstream<<"New Event = = = = idx      :"<<event_buffer[i].idx<<std::endl; */
/* 		outstream<<"          = = = = TimeStamp: 0x"<<std::hex<<event_buffer[i].TimeStamp <<" (" <<std::dec<<event_buffer[i].TimeStamp <<")"<<std::endl; */
/* 		outstream<<"          = = = = AbsTime  :"<<event_buffer[i].AbsTime<<std::endl; */
/* 		//print event hits */
/* 		for (unsigned int j=0; j<event_buffer[i].hits.size(); j++){ */
/* 			outstream<<"X:"<<std::setw(4)<<event_buffer[i].hits[j].X_address<<" Y:"<<std::setw(4)<<event_buffer[i].hits[j].Y_address<<std::endl;	 */
/* 		} */
	}
}

void matrixData::PrintAll(std::ostream & outstream){
  PrintAll(outstream, 0);
}

void matrixData::PrintAll(std::ostream & outstream, uint32_t n ){
  if (n==0) n=event_buffer.size();
  outstream<<"========PRINTING MATRIX DATA========"<<std::endl;
	for (uint32_t i =0; i<n && i<event_buffer.size(); i++){
	  event_buffer[i].Print(outstream);
	}
}




int matrixData::CountHits(){
	int count=0;
	for (unsigned int i =0; i<event_buffer.size(); i++){
			count += event_buffer[i].hits.size();
	}
	return count;
}

void matrixData::SetMatrixSize(uint32_t x, uint32_t y)
{
    Xsize = x;
    Ysize = y;
}

uint32_t matrixData::GetMatrixXSize()
{
    return Xsize;
}
uint32_t matrixData::GetMatrixYSize()
{
    return Ysize;
}

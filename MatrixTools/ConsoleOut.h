#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
//#include <TQObject.h>


#ifndef __TAB__
	#define __TAB__
	void TAB(int indent){
		for(int tab=0; tab<indent; tab++) std::cout<<" ";
	}
	#define TAB_SIZE 2
#endif


#ifndef __VERBOSITY__
	#define __VERBOSITY__
enum _VERBOSITY {
  kSILENT = 0,
  kMEDIUM = 1,
  kDEBUG  = 2
};
#endif

#ifndef __MESSAGETYPE__
	#define __MESSAGETYPE____
enum _MESSAGETYPE {
  INFO 		= 0,
  WARNING = 1,
  ERROR  	= 2
};
#endif


#ifndef __CONSOLEOUT__
	#define __CONSOLEOUT__
typedef std::string Cstring;


class ConsoleOut{

	private:
	Cstring Owner;
	bool FileRedirect;
	std::ios_base::openmode FileOpenMode;
	_VERBOSITY ConsoleVerbosity;
	_VERBOSITY FileoutVerbosity;
	ofstream fileout;
	
	_CLUSTER_EVENT_BUFFER cluster_event_buffer;
	UInt_t read_pointer;
	
	public:
	ConsoleOut();
	ConsoleOut(std::string own);
	ConsoleOut(std::string own, bool FileRedirect);
	ConsoleOut(Cstring own, bool FileRedirect, std::ios_base::openmode OM);
	void Init();
	void SetOwner(Cstring own){Owner = own;}
	void SetFileRedirect();
	void SetFileRedirect(std::ios_base::openmode OM);
	bool GetFileRedirect(){return FileRedirect;}
	void SetFileOpenMode(std::ios_base::openmode OM){FileOpenMode = OM;	}
	void SetConsoleVerbosity(_VERBOSITY level)	{ConsoleVerbosity = level;			}
	void SetFileoutVerbosity(_VERBOSITY level)	{FileoutVerbosity = level;			}	
	void Print(_MESSAGETYPE MType, _VERBOSITY Clevel, Cstring message);
	void Print(_MESSAGETYPE MType, _VERBOSITY Clevel, _VERBOSITY Flevel, Cstring message);
	};
	
	
ConsoleOut::ConsoleOut(){
	Init();
}	

ConsoleOut::ConsoleOut(Cstring own){
	Init();
	SetOwner(own);

}

ConsoleOut::ConsoleOut(Cstring own, bool FileRedirect){
	Init();
	SetOwner(own);	
	if(FileRedirect) SetFileRedirect();
}	

ConsoleOut::ConsoleOut(Cstring own, bool FileRedirect, std::ios_base::openmode OM){
	Init();
	SetOwner(own);	
	SetFileOpenMode(OM);
	if(FileRedirect) SetFileRedirect();
}	


void ConsoleOut::Init(){
	SetOwner("");
	FileRedirect = false;
	SetFileOpenMode(std::ios_base::out);
	SetConsoleVerbosity(kMEDIUM);
	SetFileoutVerbosity(kDEBUG);
}

void ConsoleOut::SetFileRedirect(){
	FileRedirect = true;
	SetFileOpenMode(std::ios_base::out);
	if(!fileout.is_open()) fileout.open("ConsoleOut.txt", FileOpenMode);
}

void ConsoleOut::SetFileRedirect(std::ios_base::openmode OM){
	FileRedirect = true;
	SetFileOpenMode(OM);
	if(!fileout.is_open()) fileout.open("ConsoleOut.txt", FileOpenMode);
}


void ConsoleOut::Print(_MESSAGETYPE MType, _VERBOSITY Clevel, Cstring message){
	Cstring header;
	switch (MType){
		case INFO:
			header="INFO";
			break;
		case WARNING:
			header="WARNING";
			break;
		case ERROR:
			header="ERROR";	
	}

	if(Clevel<=ConsoleVerbosity){
		std::cout<<header<<"("<<Owner<<"): "<<message<<std::endl;	
	}

	if(FileRedirect){			
		if(Clevel<=FileoutVerbosity){
			fileout<<header<<"("<<Owner<<"): "<<message<<std::endl;	
		}							
	}
}
	

void ConsoleOut::Print(_MESSAGETYPE MType, _VERBOSITY Clevel, _VERBOSITY Flevel, Cstring message){
	Cstring header;
	switch (MType){
		case INFO:
			header="INFO";
			break;
		case WARNING:
			header="WARNING";
			break;
		case ERROR:
			header="ERROR";	
	}
	if(Clevel<=ConsoleVerbosity){
		std::cout<<header<<"("<<Owner<<"): "<<message<<std::endl;	
	}

	if(FileRedirect){			
		if(Flevel<=FileoutVerbosity){
			fileout<<header<<"("<<Owner<<"): "<<message<<std::endl;	
		}						
	}
}
	





#endif


#include <TQObject.h>
//#include <TGCanvas.h>
#include <RQ_OBJECT.h>  
//#include <TGComboBox.h>
//#include <TGNumberEntry.h>
//#include <TGFrame.h>
//#include "parser.h"
#include <TH2I.h>
#include <TCanvas.h> 

class TGNumberEntry;
class TGFrame;
class TGComboBox;
class TGCanvas;
class TGWindow; 
class TGMainFrame; 
class TRootEmbeddedCanvas;  
class parser;
class matrixData; 
class TGCheckButton;

class TestMainFrame {
	RQ_OBJECT("TestMainFrame")

private: 

	static const unsigned int kMATRIX_X_SIZE=128;
	static const unsigned int kMATRIX_Y_SIZE=32;

	TGMainFrame *fMain;
	//TGCanvas  	*myCanvas;
	TRootEmbeddedCanvas *fEcanvas, *myCanvas;
	TGNumberEntry *Nevent, *Delay;
	TGComboBox *backFill, *dataSelector; 
	TGCheckButton *integ ;
//	parser *myparser;
//	matrixData *mymatrixdata; 

public:
	matrixData *mymatrixdata;
	parser     *myparser;
	TH2I *spread;
	TCanvas *fCanvas; 
	TestMainFrame(const TGWindow *p,UInt_t w,UInt_t h);
	virtual ~TestMainFrame();
	
	
	void CloseWindow();
	void MDRewind();
	void PlotSpread();
//	void PlotSingle();
	void PlotOne();
	void RevOne();
//	void PlotOneInteg();
	void PlotReset();

	
	//void ParseFile();
  //void ParseFile(char * filename);
}; 

#include <vector>
#include <TApplication.h>
#include <TImage.h>
#include <TGClient.h>
#include <TF1.h> 
#include <TH1I.h>
#include <TStyle.h>
#include <TRandom.h>  
#include <TGButton.h>
#include <TGComboBox.h>
#include <TColor.h>
#include <TGLabel.h>
#include <TGImageMap.h>
#include <TGNumberEntry.h>
#include <TRootEmbeddedCanvas.h>
#include "EventViewer.h" 
#include "parser.h"
/*
void ParseFile(char * filename, parser * par, matrixData * mymdata){
  
	ifstream datafile;
	datafile.open(filename);
	if (par->attachFile(datafile))std::cout<<"file attached"<<std::endl;  
	par->fillMatrixData(mymdata);
 	datafile.close();
 	par->fileRewind();
}
*/


TestMainFrame::TestMainFrame(const TGWindow *p,UInt_t w,UInt_t h) {


	fMain = new TGMainFrame(p, w, h);
	//fCanvasWindow = new TGCanvas(fMain, 400, 240);
	//fMain->AddFrame(fCanvasWindow, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,0, 0, 2, 2));
	fMain->Connect("CloseWindow()", "TestMainFrame", fMain, "CloseWindow()");	

/*	fEcanvas = new TRootEmbeddedCanvas ("Ecanvas",fMain, 480,100);
	TImage *img = TImage::Open("banner.jpg");
 	if (!img) {
      printf("Could not create an image... exit\n");
      return;
   }	
	img->SetConstRatio(0);
 	img->SetImageQuality(TAttImage::kImgBest);
	TCanvas *fCanvas = fEcanvas->GetCanvas();
	fCanvas->cd();
	img->Draw();		 
	fMain->AddFrame(fEcanvas, new TGLayoutHints(kLHintsCenterX,0,0,0,0));
*/
//  TGImageMap *fImageMap = new TGImageMap(fMain, "banner.jpg");
//  fMain->AddFrame(fImageMap, new TGLayoutHints(kLHintsCenterX,0,0,0,0) );


///////////////////CONTAINER ORIZZONTALE 1


//////////////////CONTAINER ORIZZONTALE 2

//////////////////plot BOX
 fEcanvas = new TRootEmbeddedCanvas ("Ecanvas",fMain, 800,250);
	fCanvas = fEcanvas->GetCanvas();
	fCanvas->cd();
	TStyle *mystyle = new TStyle("Default","Default Style");
	mystyle->SetPalette(1);
	//mystyle->SetOptStat("111111");
	
	
	//////////////////////////////////////////////WARNING rimuovi il  kMATRIX_Y_SIZE/2
	char title[100];
 	snprintf(title,99,"Hit dispersion%s","prova");
 	///////////////////////////////////////////////////////////////////////////////////PER MICROBO:
// 	spread = new TH2I("spread",title, kMATRIX_X_SIZE, -0.5, kMATRIX_X_SIZE-0.5, kMATRIX_Y_SIZE/2, -0.5, kMATRIX_Y_SIZE-0.5);
 	///////////////////////////////////////////////////////////////////////////////////////////////
 	
 	
 	spread = new TH2I("spread",title, kMATRIX_X_SIZE, -0.5, kMATRIX_X_SIZE-0.5, kMATRIX_Y_SIZE, -0.5, kMATRIX_Y_SIZE-0.5);
//	myCanvas = new TRootEmbeddedCanvas(fMain, 400, 240);
//	fMain->AddFrame(myCanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,0, 0, 2, 2));
//	(myCanvas->GetCanvas)->cd();
//	spread->Draw();
	
	
	spread->SetOption("COLZ"); 
	spread->SetAxisRange(0,50,"Z");
	spread->Draw();	
	fCanvas->Update();
fMain->AddFrame(fEcanvas, new TGLayoutHints(kLHintsCenterX,0,0,0,0));

	
	
	//	pxl[4][4]->ChangeBackground(TColor::RGB2Pixel((250&255),0,0));
	
	
	TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain, 200,40);
	TGHorizontalFrame *hframe2 = new TGHorizontalFrame(fMain, 200,40);
	TGHorizontalFrame *hframe3 = new TGHorizontalFrame(fMain, 200,40);
	
	//hframe
	TGTextButton *plotReset		= new TGTextButton(hframe,"PlotReset"); 
	TGTextButton *eventRewind	= new TGTextButton(hframe,"Event Rewind"); 
											integ = new TGCheckButton(hframe, "Integral",1);


	hframe->AddFrame(plotReset, new TGLayoutHints(kLHintsRight,5,5,3,4));
	hframe->AddFrame(eventRewind, new TGLayoutHints(kLHintsRight,5,5,3,4));
	hframe->AddFrame(integ, new TGLayoutHints(kLHintsRight,5,5,3,4));


	//hframe2
	TGTextButton *prevOne		 = new TGTextButton(hframe2,"Prev"); 
	TGTextButton *plotOne		 = new TGTextButton(hframe2,"Next");
	hframe2->AddFrame(prevOne, new TGLayoutHints(kLHintsLeft,5,5,3,4));
	hframe2->AddFrame(plotOne, new TGLayoutHints(kLHintsLeft,5,5,3,4));
	
	//hframe3
	TGTextButton *plotSpread = new TGTextButton(hframe3,"PlotSpread"); 
	
	Nevent = new TGNumberEntry(hframe3,0, 5, 1, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 90000);

	Delay = new TGNumberEntry(hframe3,0, 5, 1, TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMinMax, 0, 1000);

	hframe3->AddFrame(plotSpread, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,5,5,3,4));		
	hframe3->AddFrame(Nevent, new TGLayoutHints(kLHintsLeft| kLHintsCenterY,40,10,10,1));
	hframe3->AddFrame(new TGLabel(hframe3, "Events to plot"), new TGLayoutHints(kLHintsLeft | kLHintsCenterY,1,10,10,1));
	hframe3->AddFrame(Delay, new TGLayoutHints(kLHintsLeft| kLHintsCenterY,40,10,10,1));
	hframe3->AddFrame(new TGLabel(hframe3, "Delay"), new TGLayoutHints(kLHintsLeft | kLHintsCenterY,1,10,10,1));
	
	
	
//	TGTextButton *plotSingle = new TGTextButton(hframe,"PlotSingle"); 
	
	//TGTextButton *plotOneInteg = new TGTextButton(hframe,"PlotOneInteg"); 


	
	plotSpread->Connect("Clicked()","TestMainFrame",this,"PlotSpread()");
//	plotSingle->Connect("Clicked()","TestMainFrame",this,"PlotSingle()");
	prevOne->Connect("Clicked()","TestMainFrame",this,"RevOne()");
	plotOne->Connect("Clicked()","TestMainFrame",this,"PlotOne()");
	
//	plotOneInteg->Connect("Clicked()","TestMainFrame",this,"PlotOneInteg()");
	plotReset->Connect("Clicked()","TestMainFrame",this,"PlotReset()");	
	eventRewind->Connect("Clicked()","TestMainFrame",this,"MDRewind()");
	
	//ParseFile->Connect("Clicked()","TestMainFrame",this,"ParseFile()");
	
	
	//oppure
	//exit->SetCommand("gApplication->Terminate");
	//oppure gli setti "gApplication->Terminate" come terzo parametro quando lo crei.
	

//	hframe->AddFrame(plotSingle, new TGLayoutHints(kLHintsCenterX,5,5,3,4));
	
//	hframe->AddFrame(plotOneInteg, new TGLayoutHints(kLHintsCenterX,5,5,3,4));
	
	//hframe->AddFrame(ParseFile, new TGLayoutHints(kLHintsCenterX,5,5,3,4));
	
	fMain->AddFrame(hframe,new TGLayoutHints(kLHintsRight,2,2,2,2));
	fMain->AddFrame(hframe2,new TGLayoutHints(kLHintsLeft,2,2,2,2));
	fMain->AddFrame(hframe3,new TGLayoutHints(kLHintsLeft,2,2,2,2));
	
	//fMain->Print();
	
	std::cout<<"======================================="<<std::endl;
	std::cout<<"EventViewer v0.2 by F.M. Giorgi				 "<<std::endl;
	std::cout<<"======================================="<<std::endl;
	fMain->SetWindowName("Event Viewer V0.2");
	fMain->MapSubwindows();
	

	fMain->Resize(fMain->GetDefaultSize());
	fMain->MapWindow();
	
	/////////////////////////////DATA ANALYSIS

 	mymatrixdata = new matrixData();
 	myparser		 = new parser();
 	
										 
 	myparser->setDataType(DAQLayerDump);							//////////////////////////////////////QUI si seleziona il TIPO DI DATI 
 	
	myparser->setVerbosity(kSILENT);
	char file_name[40]="hit_out.txt";  ////////////////////////E QUI SI SELEZIONA IL FILE DA LEGGERE

	ifstream datafile;
//	std::cout<<"BKPNT"<<std::endl;
	datafile.open(file_name);
	if(!datafile.good()) std::cout<<"ERROR: file not found"<<std::endl;
	else if (myparser->attachOutputFile(datafile)) std::cout<<"INFO: file attached"<<std::endl;
	_EVENT myevent;
 	//myparser->fileRewind();
 	//mymatrixdata->Init();	
 	myparser->fillMatrixData(mymatrixdata);
 	mymatrixdata->Print(10);	
	

	
}
  
TestMainFrame::~TestMainFrame() {
	fMain->Cleanup();
	delete fMain;
}


void TestMainFrame::PlotSpread(){
  _EVENT myevent;
  int i=0;
  int x=0,y=0,z=0;
  //int bin=0;
  int N_event = int(Nevent->GetNumber());
  if (N_event == 0) N_event = mymatrixdata->GetSize();
  for (i=0; i<N_event;i++){
    if (mymatrixdata->GetNextEvent(myevent)){
      if (integ->GetState()==kButtonUp) spread->Reset();
      for (unsigned int j=0; j<myevent.hits.size(); j++){
        if ( myevent.hits[j].X_address < kMATRIX_X_SIZE && myevent.hits[j].Y_address < kMATRIX_Y_SIZE){
          spread->Fill(myevent.hits[j].X_address,myevent.hits[j].Y_address);
        }
        else std::cout<<"!!!Invalid pixel address found!!!"<<std::endl;
      }     
      if (i%100==0||i==(N_event-1)){
        fCanvas = fEcanvas->GetCanvas();
        fCanvas->cd();
        spread->SetAxisRange(0,spread->GetBinContent(spread->GetMaximumBin(x,y,z)),"Z");
        //spread->SetAxisRange(0, 50, "z");
        spread->Draw();
        fCanvas->Update();
        usleep(int(Delay->GetNumber())*1000);
      }
    }
  }
}

void TestMainFrame::RevOne(){
	_EVENT myevent;
	int x=0,y=0,z=0;
	if (mymatrixdata->GetPrevEvent(myevent)){
		if (integ->GetState()==kButtonUp) spread->Reset();
		for (unsigned int i =0; i<myevent.hits.size(); i++){
			if ( myevent.hits[i].X_address < kMATRIX_X_SIZE && myevent.hits[i].Y_address < kMATRIX_Y_SIZE){
				spread->Fill(myevent.hits[i].X_address,myevent.hits[i].Y_address);
				}
			else std::cout<<"!!!Invalid pixel address found!!!"<<std::endl;
		}


	fCanvas = fEcanvas->GetCanvas();
	fCanvas->cd();
	//spread->SetAxisRange(0,50,"Z");
	spread->SetAxisRange(0,spread->GetBinContent(spread->GetMaximumBin(x,y,z)),"Z");
	spread->Draw();
	fCanvas->Update();
	}
}



void TestMainFrame::PlotOne(){
	_EVENT myevent;
	int x=0,y=0,z=0;
	if (mymatrixdata->GetNextEvent(myevent)){
		if (integ->GetState()==kButtonUp) spread->Reset();
		for (unsigned int i =0; i<myevent.hits.size(); i++){
			if ( myevent.hits[i].X_address < kMATRIX_X_SIZE && myevent.hits[i].Y_address < kMATRIX_Y_SIZE){
				spread->Fill(myevent.hits[i].X_address,myevent.hits[i].Y_address);
				}
			else std::cout<<"!!!Invalid pixel address found!!!"<<std::endl;
		}


	fCanvas = fEcanvas->GetCanvas();
	fCanvas->cd();
	//spread->SetAxisRange(0,50,"Z");
	spread->SetAxisRange(0,spread->GetBinContent(spread->GetMaximumBin(x,y,z)),"Z");
	spread->Draw();
	fCanvas->Update();
	}
}

/*
void TestMainFrame::PlotOneInteg(){
	_EVENT myevent;
	if (mymatrixdata->GetNextEvent(myevent)){
		for (unsigned int i =0; i<myevent.hits.size(); i++){
			if ( myevent.hits[i].X_address < kMATRIX_X_SIZE && myevent.hits[i].Y_address < kMATRIX_Y_SIZE){
				spread->Fill(myevent.hits[i].X_address,myevent.hits[i].Y_address);
				}
			else std::cout<<"!!!Invalid pixel address found!!!"<<std::endl;
		}


	fCanvas = fEcanvas->GetCanvas();
	fCanvas->cd();
	spread->Draw();
	fCanvas->Update();
	}
}
*/

void TestMainFrame::PlotReset(){
	spread->Reset();
	fCanvas = fEcanvas->GetCanvas();
	fCanvas->cd();
	spread->Draw();
	fCanvas->Update();
}

	/* 


//implementazione con il parse di un singolo evento dal file ad ogni pressione del pulsante	
	_EVENT myevent;
	
	//myparser->setVerbosity(kDEBUG);
	myparser->setDataType(2);
	//std::cout<<myparser->fileIsAttached()<<std::endl;
	//myparser->m_file->clear();
	//myparser->fileRewind();
	if (myparser->findEvent(myevent)) mymatrixdata->AddEvent(myevent); else std::cout<<"no event found to add"<<std::endl;
	myevent.Print();
*/


//void TestMainFrame::ParseFile(){
//	myparser->fileRewind();
//	mymatrixdata->Init();	
// 	myparser->fillMatrixData(mymatrixdata);
//	mymatrixdata->Print();	
//}


void TestMainFrame::MDRewind(){
	mymatrixdata->ResetReadPointer();
}




void TestMainFrame::CloseWindow()
{
   // Got close message for this MainFrame. Terminates the application.

   gApplication->Terminate();
}


void example() {
 /*	_HIT myhit;
 	myhit.X_address=2;
 	myhit.Y_address=4;
 	_EVENT myevent;
 	myevent.TimeStamp=144;
 	for (int i =0; i < 20; i++){
	 	myevent.hits.push_back(myhit);
	 	myhit.X_address++;
	 	myhit.Y_address++;
  }
  matrixData *mymatrixdata = new matrixData();
 	mymatrixdata->AddEvent(myevent);
 	_EVENT readout_event;
 	if ( mymatrixdata->GetNextEvent(readout_event)) std::cout<<"time stamp: "<<readout_event.TimeStamp<<" readout_event address "<<&readout_event<<" myevent address "<<&myevent<<std::endl;
 	if ( mymatrixdata->GetNextEvent(readout_event)) std::cout<<"time stamp: "<<readout_event.TimeStamp<<"readout_event address "<<&readout_event<<" myevent address "<<&myevent<<std::endl;
 
	ifstream datafile;
	datafile.open("mydata.txt");
	std::cout<<datafile.tellg()<<std::endl;

	parser *myparser = new parser();
	if (myparser->attachFile(datafile))std::cout<<"file attached"<<std::endl;  
	myparser->setDataType(2);
	myparser->setVerbosity(kSILENT);
	myparser->fillMatrixData(mymatrixdata);

	myparser->fileRewind();	
	if (myparser->findEvent(myevent)) mymatrixdata->AddEvent(myevent); else std::cout<<"no event found to add"<<std::endl;
	myparser->fileRewind();	
	
	
	//if (myparser->findEvent(myevent)) mymatrixdata->AddEvent(myevent); else std::cout<<"no event found to add"<<std::endl;
	//if (myparser->findEvent(myevent)) mymatrixdata->AddEvent(myevent); else std::cout<<"no event found to add"<<std::endl;
	//if (myparser->findEvent(myevent)) mymatrixdata->AddEvent(myevent); else std::cout<<"no event found to add"<<std::endl;
	//if (myparser->findEvent(myevent)) mymatrixdata->AddEvent(myevent); else std::cout<<"no event found to add"<<std::endl;
	//if (myparser->findEvent(myevent)) mymatrixdata->AddEvent(myevent); else std::cout<<"no event found to add"<<std::endl;
	
	 
	mymatrixdata->Print();
 	datafile.close();
  //data = new DataParser("data_file.txt", );
*/
 new TestMainFrame(gClient->GetRoot(), 1024, 400);


}

int main(int argc, char **argv) {

	TApplication theApp("App",&argc,argv);
	example();
	theApp.Run();
	return 0;
}

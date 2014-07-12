#######################################################
## Comment this out to compile without the ROOT  support.
## Some applications will not be compiled
## ROOT = 1
## IMPORTANT!! If you change this, remember to clean the 
## direcotory to recompile all. 
########################################################

########################################################
## Select your system architecture
mARCH = -m32
## -m32 for 32 bit OS(i686, x86), -m64 for 64 bit OS(x86_64, x64)
## use "uname -m" command to find out your system type
########################################################

ifdef ROOT
   RS= -DROOT_SUPPORT
   ROOTLIB=`root-config --libs`  -lNew
   ROOTINC=  `root-config --cflags`  $(RS)
   ROOTGLIB= `root-config  --glibs` -lNew
else	
	 RS =
   ROOTLIB=
   ROOTINC= 
   ROOTGLIB=
endif


L= $(ROOTLIB) -lm -lstdc++ $(mARCH)
LG= $(ROOTGLIB)  -lm -lstdc++ $(mARCH)
CC=g++
RC=rootcint
CFLAGS = -Wall $(ROOTINC) 
CFLAGSGLIB = -Wall $(ROOTINCGLIB)
DICT = exa2aDict.cxx



RULE_LIST =
##Library functionsc
ifdef ROOT 
  RULE_LIST += dict
  RULE_LIST += EventManager.o
	RULE_LIST += Slim5OutputParser.o
endif

RULE_LIST += matrixData.o
RULE_LIST += parser.o
RULE_LIST += analyzer.o 
RULE_LIST += clusters.o 



##top applications
ifdef ROOT 
  RULE_LIST += microBoAnalyzer.o 
  RULE_LIST += SPX0Calibrator.o
  RULE_LIST += INMAPSCalibrator.o
  RULE_LIST += APSEL3DCalibrator.o
  RULE_LIST += dump.o
  RULE_LIST += EventViewer.o
endif
RULE_LIST += MatSimAnalysis.o


##linking rules
ifdef ROOT 
  RULE_LIST += microBoAnalyzer
  RULE_LIST += SPX0Calibrator 
  RULE_LIST += INMAPSCalibrator
  RULE_LIST += APSEL3DCalibrator
  RULE_LIST += dump
  RULE_LIST += EventViewer
endif
RULE_LIST += MatSimAnalysis


all:  $(RULE_LIST)


##Compile Libraries
dict: EventViewer.h 
	$(RC) -f exa2aDict.cxx -c EventViewer.h exa2aLinkDef.h

EventManager.o: EventManager.cpp EventManager.h  bit_operations.h error_management.h
	$(CC) $(CFLAGS) -c $<

Slim5OutputParser.o: Slim5OutputParser.cpp Slim5OutputParser.h EventManager.h bit_operations.h error_management.h 
	$(CC) $(CFLAGS) -c $<

matrixData.o: matrixData.cpp matrixData.h
	$(CC) $(CFLAGS) -c $<

clusters.o : clusters.cpp clusters.h matrixData.h
	$(CC) $(CFLAGS) -c $<

parser.o : parser.cpp parser.h matrixData.h
	$(CC) $(CFLAGS) -c $<

analyzer.o : analyzer.cpp analyzer.h parser.h clusters.h matrixData.h
	$(CC) $(CFLAGS) -c $<


#Compile top applications
MatSimAnalysis.o: MatSimAnalysis.cpp analyzer.h parser.h matrixData.h
	$(CC) $(CFLAGS) -c $<

microBoAnalyzer.o: microBoAnalyzer.cpp analyzer.h parser.h clusters.h matrixData.h
	$(CC) $(CFLAGS) -c $<	

SPX0Calibrator.o: SPX0Calibrator.cpp bit_operations.h Slim5OutputParser.h 
	$(CC) $(CFLAGS) -c $<

INMAPSCalibrator.o:  INMAPSCalibrator.cpp bit_operations.h Slim5OutputParser.h 
	$(CC) $(CFLAGS) -c $<

APSEL3DCalibrator.o:  APSEL3DCalibrator.cpp bit_operations.h Slim5OutputParser.h 
	$(CC) $(CFLAGS) -c $<

dump.o:  dump.cpp bit_operations.h Slim5OutputParser.h
	$(CC) $(CFLAGS) -c $<

EventViewer.o: EventViewer.cpp EventViewer.h parser.h matrixData.h
	$(CC) $(CFLAGS) -c $< exa2aDict.cxx



#Link top applications
microBoAnalyzer: microBoAnalyzer.o  matrixData.o parser.o analyzer.o clusters.o
	$(CC) $^ $(L) -o $@ 

SPX0Calibrator: SPX0Calibrator.o  Slim5OutputParser.o EventManager.o
	$(CC) $^ $(L) -o $@ 

INMAPSCalibrator: INMAPSCalibrator.o  Slim5OutputParser.o EventManager.o
	$(CC) $^ $(L) -o $@ 

APSEL3DCalibrator: APSEL3DCalibrator.o Slim5OutputParser.o EventManager.o
	$(CC) $^ $(L) -o $@ 

dump: dump.o Slim5OutputParser.o EventManager.o
	$(CC) $^ $(L) -o $@ 

MatSimAnalysis: matrixData.o parser.o analyzer.o clusters.o MatSimAnalysis.o
	$(CC) $^ $(L) -o $@ 

EventViewer: EventViewer.o matrixData.o parser.o 
	$(CC) $^ $(LG) -o $@


clean:
	rm *.o

cleanest: clean
	rm MatSimAnalysis
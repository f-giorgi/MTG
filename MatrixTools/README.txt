-------------------------
-- Matrix Tools
-------------------------

Software analysis libraries and tools for pixel chip data analysis.

makefile          : make all applications. Root support can be disabled. In this case only the applications
                    with no root support (or optional root support) are built. Look in the makefile for details.
commit_cleaner.sh : clean the directory of compiled files. Must be kept up to date. 

------------------
-- APPLICATION TOOLS:

/APSEL3DCalibrator.cpp   -> Tool for the APSEL3D chip calibration. Root Histo production from Raw TDAQ Data.
                            Additional information in source comments. 
/dump                    -> Tool for dumping TDAQ Raw Data with textual decodings.
/EventViewer             -> Root GUI application for visualization of pixel frames/events from Raw TDAQ Data.
/INMAPSCalibrator.cpp    -> Tool for the INMAPS32x32 chip calibration. Root Histo production from Raw TDAQ Data.
                            Additional information in source comments. 
/MatSimAnalysis.cpp      -> Main Tool for the analysis of data produced with "svn.cern.ch/reps/vipix/chipReadout"
                            simulation package.
/microBoAnalyzer.cpp     -> Tool for the analysis of TEM data.
/SPX0Calibrator          -> Tool for the SPX0 chip calibration. Root Histo production from Raw TDAQ Data.
                            Additional information in source comments. 



-------------------
-- LIBRARIES:
/bit_operation.h            -> Definitions for decoding of specific data types.
/ConsoleOut.h               -> First attempt to provide a common output facility, not really finished.
/error_management.h         -> Error management library.
/EventManager (.h, .cpp)    -> TDAQ event management
/exa2aLinkDef               -> pragma for Root GUI applications.
/matrixData (.h, .cpp)      -> General data structure for a typical pixel chip acquisition:
                               - a matriData class is made of a vector of events (+ additional info)
                               - an EVENT is made of a vector of HIT and a TimeStamp
                               - a HIT is made of an X and Y coordinates. 
/parser (.h, .cpp)           -> Library for parsing different types of raw data into matrixData structures. 
/Slim5OutpuParser (.h, .cpp) -> Original Parser of TDAQ Raw Data.
/analyzer (.h, .cpp)         -> Library for common analysis and comparison on matrixData structures.
/clusters (.h, .cpp)         -> Cluster data structure for cluster analysis with analyzer.




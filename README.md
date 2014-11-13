MTG - MatrixTools GUI
=====================
 
A Graphical User Interface for MatrixTools C++ Library
------------------------------------------------------
 
### TOC

* 1 Introduction 
* 2 Description 
* 3 Developers list 





### 1 - Introduction

The MatrixTools C++ library and its Qt Graphical User Interface.
The MatrixTools is a library providing help to the developers of pixel-sensor readout architectures, and in general to those who want to perfom common analysis on raw pixel data (e.g. pattern match, sequence checks, cluster analysis etc...).
The MTG project arises from a collection of algorithms and codes the original developer (F Giorgi) mainly used to test his readout architecture models, written in VHDL. The first goal of this project was to unify all the code developed during the years of research. This tiding-up process generated the first version of the MatrixTool library. Now the code is still far from being "clean & tide" to the eyes of a pure software developer though, it is shaped enough to be a reasonable starting point for further reorganizations and extensions. 
A first extension has already been started by giving the libraries a fancy GUI under the Qt environment. The hope of the main developer was to encourage the use of his software by other researchers. This led also to the choice of making it an open source project on the net to promote its diffusion and development. 

*******************************************************************************************************************
To be mentioned that the project is provided as is without any warranties. Anyone interested in re-using the software is welcome in doing that, provided that no specific help is due by the developers team in customization of the interfaces to the user needs. Keep in mind that generally help is also provided "as is".  
The library and its GUI make use of third party software libraries such as Qt 4 environment and CERN Root v5. By the way, the MatrixTools library comes with no source of any third party software. Anyone who wants to build the MatrixTools software enabling the third party features needs to fetch the required libraries/sources/environments from their respective official mirror sites.

http://root.cern.ch

http://qt-project.org/

*******************************************************************************************************************
The first commit is still a work in progress so some part of the code may result obscure. Consult the list of contributorst if you need demistifications.
*******************************************************************************************************************


### 2 - Description

The library is based on the class matrixData, which provide a common data structure for basically any binary pixel detector (i.e. a collection of hits (x,y) with an absolute time stamp t organized in "events")
The real "Tools" are the classes that screw on matrixData, for instance those filling it by parsing data files, or those performing cluster analysis. At the moment no other documentation is provided but the source comments (a doc or man page has high priority in the todo list after writing this README).
The GUI is realized within the Qt environment and some plot features requires the CERN root library. Qt is for the user program GUI based on the MatrixTools library, while CERN Root library calls are present in the MT library itself.
The Makefile in the MatrixTools directory (building some command line user-program examples based on the MT lib) show how to enable/disable the Root features.
The Makefile in the MTG directory build the GUI user program exploiting the MT libs.




### 3 - Developers Team
  * FGiorgi
  * AMandrake

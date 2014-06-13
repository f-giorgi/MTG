--------------------------------------
-- MTG - MatrixTools GUI software
--------------------------------------

GUI tool based on the matrixTools library.
Under development, up to now used to inspect events from TEM experiments.

Developed on Qt 4. 
Required packages: qwt

+--------------------------------------+
+ WARNING:                             +
+ requires source from:                +
+ ../MatrixTools                       +
+                                      +
+--------------------------------------+


- Install and compile QWT in the $QWT_INST_DIR (follow qwt instructions)
- Add this environmental variable to your system:
      QMAKEFEATURES = $QWT_INST_DIR/features
- Add your Qt mingw/bin directory to your $PATH env. variable.
- Build source files in the MTG directory:
      qmake MTG.pro
      make


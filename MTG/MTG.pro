CONFIG += qwt

HEADERS         = mainwindow.h \
    ../MatrixTools/parser.h \
    Dialogs/importdilaog.h \
    MVDataManager/mvdatamanager.h \
    ../MatrixTools/matrixData.h \
    MVDataManager/datamodel.h \
    plot/plot.h \
    graphics/pixelswitch.h \
    plot/plotwidget.h \
    EventViewer/eventviewer.h \
    maskEditor/checkablematrix.h \
    maskManager/maskmanager.h
SOURCES         = main.cpp \
                  mainwindow.cpp \
    ../MatrixTools/parser.cpp \
    Dialogs/importdilaog.cpp \
    MVDataManager/mvdatamanager.cpp \
    ../MatrixTools/matrixData.cpp \
    MVDataManager/datamodel.cpp \
    plot/plot.cpp \
    graphics/pixelswitch.cpp \
    plot/plotwidget.cpp \
    EventViewer/eventviewer.cpp \
    maskEditor/checkablematrix.cpp \
    maskManager/maskmanager.cpp
RESOURCES       = \
    MTG.qrc

##RC_FILE = MTG.rc

FORMS += \
    Dialogs/importdilaog.ui

OTHER_FILES += \
    styles.qss

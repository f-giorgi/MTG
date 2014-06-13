/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>

//#include "graphics/pixelswitch.h"
#include "mainwindow.h"
#include "../matrixTools/parser.h"
#include "MVDataManager/datamodel.h"
#include "plot/plotwidget.h"
#include "EventViewer/eventviewer.h"
#include "maskEditor/checkablematrix.h"
#include "maskManager/maskmanager.h"

MainWindow::MainWindow()
{
    outputLog = new QTextEdit(this);
    outputLog->setReadOnly(true);
    outputLog->append("MTG log \n-------\n");



    //to be created a central widget
    //setCentralWidget(new QWidget());
    setCentralWidget(outputLog);
    setWindowIcon(QIcon(":/images/MTG.png"));
    //creating the menu titles
    fileMenu = menuBar()->addMenu(tr("&File"));
    //editMenu = menuBar()->addMenu(tr("&Edit"));
    analyzerMenu = menuBar()->addMenu(tr("Analyzer"));
    DMmenu = menuBar()->addMenu(tr("DataManager"));
    MMmenu = menuBar()->addMenu(tr("MaskManager"));
    viewMenu = menuBar()->addMenu(tr("&View"));
    menuBar()->addSeparator();
    helpMenu = menuBar()->addMenu(tr("&Help"));

    // creating stuff
    //warning: take care of the order you call these functions
    createDockWindows();
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    QFile file(":/styles.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);

    setWindowTitle(tr("Matrix Tool GUI"));
    setIconSize(QSize(35,35));

    desktop = qApp->desktop();

    connect(desktop, SIGNAL(resized(int)), this, SLOT(centerMainWindowGeometry()));

    //connect(dataManager, SIGNAL(plotHisto(QPair<matrixData*, QString>)), this, SLOT(raisePlotDock()));
    connect(dataManager, SIGNAL(plotHisto(QPair<matrixData*, QString>)), plotWidget, SLOT(histoMatrix(QPair<matrixData*, QString>)));

    //connect(dataManager, SIGNAL(inspectMatrixData(QPair<matrixData*, QString>)), this, SLOT(raiseEVDock()));
    connect(dataManager, SIGNAL(inspectMatrixData(QPair<matrixData*, QString>)),eventViewer, SLOT(attachMatrixData(QPair<matrixData*, QString>)));

    //raise the involved dock when action is triggered
    connect(newDataSetAct, SIGNAL(triggered()), this, SLOT(raiseDMDock()));
    connect(rmDataSetAct, SIGNAL(triggered()), this, SLOT(raiseDMDock()));
    connect(rmAllDataSetAct, SIGNAL(triggered()), this, SLOT(raiseDMDock()));
    connect(plotDataSetAct, SIGNAL(triggered()), this, SLOT(raiseDMDock()));
        connect(plotDataSetAct, SIGNAL(triggered()), this, SLOT(raisePlotDock()));
    connect(saveDataSetAct, SIGNAL(triggered()), this, SLOT(raiseDMDock()));
    connect(inspectEventAct, SIGNAL(triggered()), this, SLOT(raiseDMDock()));
        connect(inspectEventAct, SIGNAL(triggered()), this, SLOT(raiseEVDock()));
    connect(renameDataSetAct, SIGNAL(triggered()), this, SLOT(raiseDMDock()));
    //
    connect(newMaskSetAct, SIGNAL(triggered()), this, SLOT(raiseMMDock()));
    connect(printMaskAct, SIGNAL(triggered()), this, SLOT(raiseMMDock()));
    connect(rmMaskSetAct, SIGNAL(triggered()), this, SLOT(raiseMMDock()));
    connect(renameMaskSetAct, SIGNAL(triggered()), this, SLOT(raiseMMDock()));
    connect(editMaskAct, SIGNAL(triggered()), this, SLOT(raiseMMDock()));


    //raise the involved dock when made visible
    connect(DMviewAct, SIGNAL(triggered()), this, SLOT(raiseDMDock()));
    connect(MMviewAct, SIGNAL(triggered()), this, SLOT(raiseMMDock()));
    connect(plotViewAct, SIGNAL(triggered()), this, SLOT(raisePlotDock()));
    connect(EVviewAct, SIGNAL(triggered()), this, SLOT(raiseEVDock()));

    //connect the log output signals
    connect(dataManager, SIGNAL(outputLog(QString)), outputLog, SLOT(append(QString)));
    connect(maskManager, SIGNAL(outputLog(QString)), outputLog, SLOT(append(QString)));
    connect(plotWidget, SIGNAL(outputLog(QString)), outputLog, SLOT(append(QString)));
    connect(eventViewer, SIGNAL(outputLog(QString)), outputLog, SLOT(append(QString)));


    setUnifiedTitleAndToolBarOnMac(true);

}

void MainWindow::showWelcome()
{
    QMessageBox::about(this, tr("Welcome to MTG"),
             tr("<b>WELCOME TO MTG</b> <br> The <b>Matrix Tool GUI</b> provides "
                "a Graphic User Interface to the matrixTools library, "
                "A command-line toolset for pixel data analysis."));
}

//invent a file structure for saving imported data in a common way.
void MainWindow::save()
{


    //statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);
}

//ask if you really want to quit.
void MainWindow::tryClose()
{
    QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Leaving Matrix Tool GUI"),
                                    tr("Are you sure you want to quit?"),
                                    QMessageBox::Ok | QMessageBox::Cancel,
                                    QMessageBox::Cancel);
    switch (ret) {
      case QMessageBox::Ok:
          // Ok was clicked
          close();
          break;
      case QMessageBox::Cancel:
          // Cancel was clicked
          break;
      default:
          // should never be reached
          break;
    }

}

void MainWindow::about()
{
   QMessageBox::about(this, tr("About MTG"),
            tr("The <b>Matrix Tool GUI</b> provides "
               "a Graphic User Interface to the matrixTools library, "
               "A command-line toolset for pixel data analysis."));
}

void MainWindow::openFile(){
    //    QFileDialog *openDialog = new QFileDialog();
    //openDialog->exec();
    QString filePath;
    filePath = QFileDialog::getOpenFileName(this,tr("Chose the file you want to open"),".","All Files (*.*)");
    statusBar()->showMessage(filePath, 2000);
}


//import a data structure
void MainWindow::import(){
    if (dataManager->m_model->rowCount() ==0)
    {
        QMessageBox::warning(this, tr("Import"),
                           tr("Create first a new empty data set"));
        return;
    }
    ImportDilaog * dialog = new ImportDilaog();
    dialog->fillDataSets(dataManager->m_model);
    dialog->setWindowTitle("Import data from file");
    dialog->setWindowIcon(QIcon(":/images/import.png"));
    if (dialog->exec() == QDialog::Accepted) {
        QString fileName(dialog->getPath());
        _DATATYPE dataType = parser::getDataTypeName(dialog->getType());
        _VERBOSITY verbosity = parser::getVerbosityLevelName(dialog->getVerbosityIndex());
        QModelIndex  modelIndex =  dataManager->m_model->index(dialog->getDataSetIndex(), dataModel::DATASET_NAME);
        //QModelIndex * pmodelIndex = & modelIndex;
        dataManager->importFile(fileName, modelIndex, dataType, verbosity);
    }
    delete dialog;
    statusBar()->showMessage("Import Dialog Terminated", 2000);
}

void MainWindow::raiseEVDock()
{
    tabBars = this->findChildren<QTabBar*>();
     if(!tabBars.isEmpty())
     {
      tabBars.at(1)->setCurrentIndex(1);
     }

}
void MainWindow::raisePlotDock()
{
    tabBars = this->findChildren<QTabBar*>();
     if(!tabBars.isEmpty())
     {
      tabBars.at(1)->setCurrentIndex(0);
     }

}

void MainWindow::raiseDMDock()
{
    tabBars = this->findChildren<QTabBar*>();
     if(!tabBars.isEmpty())
     {
      tabBars.first()->setCurrentIndex(0);
     }
}

void MainWindow::raiseMMDock()
{
    tabBars = this->findChildren<QTabBar*>();
     if(!tabBars.isEmpty())
     {
      tabBars.first()->setCurrentIndex(1);
     }
}

//tabBars = this->findChildren<QTabBar*>();
// if(!tabBars.isEmpty())
// {
//  tabBars.first()->setCurrentIndex(0);
//  tabBars.at(1)->setCurrentIndex(0);
// }


void MainWindow::createActions()
{
    //newDataSetAct = new QAction(QIcon(":/images/new.png"), tr("&New Data Set"),
      //                         this);
    //newDataSetAct->setShortcuts(QKeySequence::New);
    //newDataSetAct->setStatusTip(tr("Create a new blank set of data"));


    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save..."), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the current form letter"));
    saveAct->setEnabled(false);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));


    quitAct = new QAction(tr("&Quit"), this);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(tryClose()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));


    openFileAct = new QAction(tr("Open File..."),this);
    openFileAct->setStatusTip(tr("Open a Data Set File"));
    openFileAct->setEnabled(false);
    connect(openFileAct, SIGNAL(triggered()), this, SLOT(openFile()));

    importAct = new QAction(QIcon(":/images/import.png"),tr("Import..."), this);
    importAct->setStatusTip(tr("Import data from different file formats"));
    connect(importAct, SIGNAL(triggered()), this, SLOT(import()));


    newDataSetAct    = dataManager->getAction("newDataSetAct");
    rmDataSetAct     = dataManager->getAction("rmDataSetAct");
    rmAllDataSetAct  = dataManager->getAction("rmAllDataSetAct");
    plotDataSetAct   = dataManager->getAction("plotDataAct");
    saveDataSetAct   =  dataManager->getAction("saveDataSetAct");
    inspectEventAct  = dataManager->getAction("inspectEventAct");
    renameDataSetAct = dataManager->getAction("renameDataSetAct");

    newMaskSetAct    = maskManager->getAction("newMaskSetAct");
    renameMaskSetAct = maskManager->getAction("renameMaskSetAct");
    editMaskAct      = maskManager->getAction("editMaskAct");
    printMaskAct     = maskManager->getAction("printMaskAct");
    rmMaskSetAct     = maskManager->getAction("rmMaskSetAct");

    //creates the toggle view actions with icons for the dock widgets
    DMviewAct = DMdock->toggleViewAction();
    DMviewAct->setIcon(QIcon(":/images/DMicon.png"));
    MMviewAct = MMDock->toggleViewAction();
    MMviewAct->setIcon(QIcon(":/images/mask3.png"));
    plotViewAct = PlotDock->toggleViewAction();
    plotViewAct->setIcon(QIcon(":/images/PlotIcon.png"));
    EVviewAct = eventViewerDock->toggleViewAction();
    EVviewAct->setIcon(QIcon(":/images/inspectorIcon.png"));



}

void MainWindow::createMenus()
{
    fileMenu->addAction(openFileAct);
    fileMenu->addAction(importAct);
    fileMenu->addAction(saveAct);

    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

    //analyzerMenu->addAction();

    DMmenu->addAction(newDataSetAct);
    DMmenu->addAction(renameDataSetAct);
    DMmenu->addAction(plotDataSetAct);
    DMmenu->addAction(inspectEventAct);
    DMmenu->addAction(saveDataSetAct);
    DMmenu->addAction(rmDataSetAct);
    DMmenu->addAction(rmAllDataSetAct);

    MMmenu->addAction(newMaskSetAct);
    MMmenu->addAction(renameMaskSetAct);
    MMmenu->addAction(editMaskAct);
    MMmenu->addAction(printMaskAct);
    MMmenu->addAction(rmMaskSetAct);

    //toggle view actions
    viewMenu->addAction(DMviewAct);
    viewMenu->addAction(MMviewAct);
    viewMenu->addAction(plotViewAct);
    viewMenu->addAction(EVviewAct);



    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(saveAct);
    //    fileToolBar->addAction(saveDataSetAct);
    fileToolBar->addAction(importAct);


    //editToolBar = addToolBar(tr("Edit"));

    DMtoolBar = addToolBar(tr("DataManager"));
    DMtoolBar->addAction(newDataSetAct);
    DMtoolBar->addAction(renameDataSetAct);
    DMtoolBar->addAction(plotDataSetAct);
    DMtoolBar->addAction(inspectEventAct);
    DMtoolBar->addAction(saveDataSetAct);
    DMtoolBar->addAction(rmDataSetAct);
    DMtoolBar->addAction(rmAllDataSetAct);

    MMtoolBar = addToolBar(tr("MaskManager"));
    MMtoolBar->addAction(newMaskSetAct);
    MMtoolBar->addAction(renameMaskSetAct);
    MMtoolBar->addAction(editMaskAct);
    MMtoolBar->addAction(printMaskAct);
    MMtoolBar->addAction(rmMaskSetAct);

    viewToolBar = addToolBar(tr("View"));
    viewToolBar->addAction(DMviewAct);
    viewToolBar->addAction(MMviewAct);
    viewToolBar->addAction(plotViewAct);
    viewToolBar->addAction(EVviewAct);


}


void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}


void MainWindow::createDockWindows()
{
    //creating the Data Manager Dock
    DMdock = new QDockWidget(tr("Data Manager"), this);
    dataManager = new MVdataManager(DMdock);
    DMdock->setWidget(dataManager);
    addDockWidget(Qt::TopDockWidgetArea, DMdock);

    //creating the Mask Manager Dock
    MMDock = new QDockWidget(tr("Mask Manager"), this);
    maskManager = new MaskManager(MMDock);
    MMDock->setWidget(maskManager);
    addDockWidget(Qt::TopDockWidgetArea, MMDock);

    //creating the Plot Dock
    PlotDock = new QDockWidget(tr("Plot"), this);
    plotWidget = new PlotWidget(PlotDock);
    PlotDock->setWidget(plotWidget);
    addDockWidget(Qt::TopDockWidgetArea, PlotDock);

    eventViewerDock = new QDockWidget(tr("Data Inspector"), this);
    eventViewer = new EventViewer(eventViewerDock);
    eventViewerDock->setWidget(eventViewer);
    addDockWidget(Qt::TopDockWidgetArea, eventViewerDock);


//    maskEditorDock = new QDockWidget(tr("MaskEdit"),this);
//    maskMatrix = new checkableMatrix(this);
//    maskEditorDock->setWidget(maskMatrix);
//    addDockWidget(Qt::BottomDockWidgetArea,maskEditorDock);

    tabifyDockWidget(DMdock, MMDock);
    tabifyDockWidget(PlotDock, eventViewerDock);
//    tabifyDockWidget(eventViewerDock, maskEditorDock);

    //workaround: there is no way at the moment to select the
    //desired tab. You must guess which is the index of the internal
    //tab structure.
    //ref: http://qt-project.org/forums/viewthread/2835
    raisePlotDock();
    raiseDMDock();
}

void MainWindow::updateOutput(){

    QByteArray bytes = process->readAllStandardOutput();
    outputLog->insertPlainText(bytes);
}

void MainWindow::centerMainWindowGeometry()
{
 QRect screen = desktop->screenGeometry(1);
 QRect mainWindowGeometry = this->frameGeometry();

 QSize screenSize = screen.size();
    qDebug()<<screenSize;
 QSize mainWindowSize = mainWindowGeometry.size();
    qDebug()<< mainWindowSize;
 QPoint destination;
 int destX = (int)((screenSize.width()-mainWindowSize.width())/2);
 int destY = (int)((screenSize.height()-mainWindowSize.height())/2);
    qDebug()<<destX;
    qDebug()<<destY;
 if (destX >= 0) destination.setX( destX);
 if (destY >= 0) destination.setY( destY);

 move(destination);
 //resize();

}

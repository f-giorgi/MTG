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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MVDataManager/mvdatamanager.h"
#include "../matrixTools/parser.h"
#include "Dialogs/importdilaog.h"

QT_BEGIN_NAMESPACE
class QAction;
class QListWidget;
class QMenu;
class QTextEdit;
class PlotWidget;
class EventViewer;
class MaskManager;
QT_END_NAMESPACE

//! [0]
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    void showWelcome();

public slots:
    void centerMainWindowGeometry();

private slots:
    void tryClose();
    void save();
    void about();
    void openFile();
    void import();
    void raiseEVDock();
    void raiseDMDock();
    void raisePlotDock();
    void raiseMMDock();
    void updateOutput();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();

    QDesktopWidget *desktop;
    QProcess *process;
    QTextEdit *outputLog;
    MVdataManager *dataManager;
    QDockWidget * DMdock;
    QDockWidget * PlotDock;
    QDockWidget * eventViewerDock;
    //QDockWidget * maskEditorDock;
    MaskManager * maskManager;
    QDockWidget * MMDock;
    PlotWidget * plotWidget;
    EventViewer * eventViewer;
   // checkableMatrix * maskMatrix;

    QList<QTabBar*> tabBars;

    QMenu *fileMenu;
    //QMenu *editMenu;
    QMenu *analyzerMenu;
    QMenu *DMmenu;
    QMenu * MMmenu;
    QMenu *viewMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *DMtoolBar;
    QToolBar *MMtoolBar;
    QToolBar *viewToolBar;
    QToolBar *editToolBar;

    //actions from DataManager
    QAction *newDataSetAct;
    QAction *rmDataSetAct;
    QAction *rmAllDataSetAct;
    QAction *plotDataSetAct;
    QAction *saveDataSetAct;
    QAction *inspectEventAct;
    QAction *renameDataSetAct;

    //actions from MaskManager
    QAction * newMaskSetAct;
    QAction * renameMaskSetAct;
    QAction * editMaskAct;
    QAction * printMaskAct;
    QAction * rmMaskSetAct;

    //action toggle view
    QAction *DMviewAct;
    QAction *MMviewAct;
    QAction *plotViewAct;
    QAction *EVviewAct;

    QAction *saveAct;
    QAction *undoAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *quitAct;
    QAction *openFileAct;
    QAction *importAct;

};
//! [0]

#endif

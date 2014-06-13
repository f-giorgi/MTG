#include "mvdatamanager.h"
#include "../../matrixTools/matrixData.h"
#include "datamodel.h"

// PUBLIC
MVdataManager::MVdataManager(QWidget *parent) :
    QWidget(parent)
{
    setMinimumSize(470,200);

    m_model = new dataModel();
    m_view = new QTreeView(this);
    m_view->setModel(m_model);
    m_view->setIndentation(0);
    m_view->setSelectionMode(QAbstractItemView::ContiguousSelection);
    m_view->setColumnWidth(dataModel::DATASET_NAME, 120);
    m_view->setColumnWidth(dataModel::EVENT_COUNT, 60);
    m_view->setColumnWidth(dataModel::HIT_COUNT, 60);
    m_view->setColumnWidth(dataModel::TYPE_NAME, 80);
    m_view->setColumnWidth(dataModel::MTX_WIDTH, 60);
    m_view->setColumnWidth(dataModel::MTX_HEIGHT, 60);

    //connect(m_model, SIGNAL(columnDataChanged(int)), m_view, SLOT(resizeColumnToContents(int)));

    createActions();
    setupModel();



    layout = new QVBoxLayout(this);


    m_find = new QLineEdit(this);
    findLabel = new QLabel("Find: ");
    findLayout = new QHBoxLayout();
    findLayout->addWidget(findLabel);
    findLayout->addWidget(m_find);

    layout->addLayout(findLayout);

    layout->addWidget(m_view);



   // connect(m_view, SIGNAL(clicked(QModelIndex)), this, SLOT());
    connect(m_find, SIGNAL(textChanged(QString)), this, SLOT(onFindChanged(QString)));

}

MVdataManager::~MVdataManager()
{
     delete layout;
}

void MVdataManager::setupModel()
{
    addEmptyDataSet();
}

QAction * MVdataManager::getAction(const QString &actionName)
{
    return actionList[actionName];
}

bool MVdataManager::importFile(QString fileName, QModelIndex & itemIndex, _DATATYPE dataType, _VERBOSITY verbosity)
{
    matrixData * m_data =  (matrixData*) m_model->mdata(itemIndex);
    bool ret = false;
    int hitFound = 0;
    int eventFound = 0;
    char typeName[30];
    parser::getDataTypeName((int)dataType, typeName);

    std::ifstream file;

    file.open(fileName.toLocal8Bit());
    if (!file.is_open()){
        QMessageBox::critical(this, "Invalid File Name", "The selected file could not be opened");
        emit outputLog(QString("Import file \"%1\" in Data Set %2 FAILED: file not found").arg(fileName).arg(  m_model->data( m_model->index(itemIndex.row(), dataModel::DATASET_NAME),Qt::DisplayRole).toString()));
        return false;
    }

    if(dataType == INVALID_DataType){
        QMessageBox::critical(this, "Invalid Data Type", "Invalid Data Type when trying to import dataset");
        emit outputLog(QString("Import file \"%1\" in Data Set %2 FAILED: Bad Data Type").arg(fileName).arg(  m_model->data( m_model->index(itemIndex.row(), dataModel::DATASET_NAME),Qt::DisplayRole).toString()));
        return false;
    }

    if(verbosity == INVALID_VerbosityLevel){
        QMessageBox::critical(this, "Invalid Verbosity", "Invalid Verbosity when trying to import dataset");
        emit outputLog(QString("Import file \"%1\" in Data Set %2 FAILED: Bad Verbosity Level").arg(fileName).arg(  m_model->data( m_model->index(itemIndex.row(), dataModel::DATASET_NAME),Qt::DisplayRole).toString()));
        return false;
    }


    parser * myParser = new parser();
    myParser->attachOutputFile(file);
    myParser->setDataType(dataType);
    myParser->setVerbosity(verbosity);

    emit outputLog(QString("Checking Data File"));
    qDebug() << "now will check Data File";

    if (myParser->CheckOutputFile(dataType))
        emit outputLog("Check Passed");
    else{
        QMessageBox::critical(this, "Parsing Failed", "Parser failed to import data");
        emit outputLog("Check Failed");
        return false;
    }
    if(myParser->fillMatrixData(m_data)){
        eventFound = m_data->GetSize();
        hitFound = myParser->GetHitsFound();
        m_model->setEventSize(itemIndex.row(), eventFound);
        m_model->setHitSize(itemIndex.row(), hitFound);

        m_model->setTypeName(itemIndex.row(), typeName);
        m_model->setMatrixSize(itemIndex.row(), myParser->getMatrixWidth(), myParser->getMatrixHeight());

        emit outputLog(QString("Imported file \"%1\" in Data Set %2").arg(fileName).arg(  m_model->data( m_model->index(itemIndex.row(), dataModel::DATASET_NAME),Qt::DisplayRole).toString()));
        emit outputLog(QString("The imported Matrix Data Size is %1").arg(m_data->GetSize()));
        QMessageBox::information(this, "SUCCESS", "Import Successful!");
        file.close();
        return true;
    }else{
       QMessageBox::critical(this, "Parsing Failed", "Parser failed to import data. Check file and associated data type");
       emit outputLog(QString("Import file \"%1\" in Data Set %2 FAILED: Data file did not match the selected Data Type").arg(fileName).arg(  m_model->data( m_model->index(itemIndex.row(), dataModel::DATASET_NAME),Qt::DisplayRole).toString()));
       file.close();
       return false;
    }

}

////////PUBLIC SLOTS//////////////
void MVdataManager::addEmptyDataSet()
{
     m_model->appendRow();
     updateContextMenu();
}

void MVdataManager::rmDataSet()
{
     QModelIndexList itemsToBeRemoved = m_view->selectionModel()->selectedRows();
     if (itemsToBeRemoved.isEmpty()) return;
     //for all selected items...
     int firstRow = itemsToBeRemoved.front().row();
     int lastRow = itemsToBeRemoved.back().row();
     int n = lastRow - firstRow + 1;
     emit outputLog(QString("%1 rows deleted. From %2 to %3.").arg(n).arg(firstRow).arg(lastRow));
     m_model->removeRows(firstRow, n);
     updateContextMenu();
 }

void MVdataManager::renameDataSet()
{
    bool ok;
    QString text;

    QModelIndexList itemsToBeRenamed = m_view->selectionModel()->selectedRows();
    if (itemsToBeRenamed.isEmpty()) return;
    QString baseName = itemsToBeRenamed.at(0).data(Qt::DisplayRole).toString();
    text = QInputDialog::getText(this, tr("Rename Data Set"),
                                 tr("New Name:"), QLineEdit::Normal,
                                 baseName, &ok);
    if (ok && !text.isEmpty())
        m_model->setData(itemsToBeRenamed.at(0), text, Qt::EditRole);

}

void MVdataManager::rmAllDataSet()
{
  m_view->selectAll();
  rmDataSet();
}



void MVdataManager::inspect()
{
    QModelIndexList itemsToBeInspected = m_view->selectionModel()->selectedRows();
    if (itemsToBeInspected.isEmpty()) return;

    {   QPair<matrixData*,QString> inspectData;
        inspectData.first = m_model->mdata(itemsToBeInspected.at(0));
        inspectData.second = m_model->dataName(itemsToBeInspected.at(0));
        emit inspectMatrixData(inspectData);
    }

}


QStringList MVdataManager::getDsList()
{
    return m_model->getDsList();
}


////////PRIVATE///////////////////
 void MVdataManager::createActions(){
     newDataSetAct = new QAction(QIcon(":/images/boxIcon.png"), tr("&New Data Set"),
                                this);
     newDataSetAct->setShortcuts(QKeySequence::New);
     newDataSetAct->setStatusTip(tr("Create a new empty Data Set"));
     connect(newDataSetAct, SIGNAL(triggered()), this, SLOT(addEmptyDataSet()));

     rmDataSetAct = new QAction(QIcon(":/images/bin.png"),tr("Delete Selected Data Set"), this);
     rmDataSetAct->setShortcut(QKeySequence::Delete);
     rmDataSetAct->setStatusTip(tr("Delete selected Data Set"));
     connect(rmDataSetAct, SIGNAL(triggered()), this, SLOT(rmDataSet()));

         renameDataSetAct = new QAction(QIcon(":/images/renameBox.png"),tr("Rename Data Set"), this);
         renameDataSetAct->setStatusTip(tr("Rename selected Data Set"));
         connect(renameDataSetAct,  SIGNAL(triggered()), this, SLOT(renameDataSet()));

     rmAllDataSetAct = new QAction(QIcon(":/images/binAll.png"),tr("Remove all Data Sets"), this);
     rmAllDataSetAct->setStatusTip(tr("Remove all selected Data Set"));
     rmAllDataSetAct->setShortcut(QKeySequence::DeleteEndOfWord);
     connect(rmAllDataSetAct,  SIGNAL(triggered()), this, SLOT(rmAllDataSet()));

     inspectEventAct = new QAction(QIcon(":/images/inspect.png"),tr("Inspect Data Set"), this);
     inspectEventAct->setStatusTip(tr("Inspect single events of the selected data set in Event Inspector"));
     connect(inspectEventAct, SIGNAL(triggered()), this, SLOT(inspect()));

     plotDataAct = new QAction(QIcon(":/images/graph.png"),tr("Plot Selected data"), this);
     plotDataAct->setStatusTip(tr("Plot the spread distribution of selected data"));
     connect(plotDataAct, SIGNAL(triggered()), this, SLOT(plotSelectedData()));

     saveDataSetAct = new QAction(QIcon(":/images/saveDataSet.png"),tr("Save Data"), this);
     saveDataSetAct->setStatusTip(tr("Save selected data set to disk"));
     connect(saveDataSetAct, SIGNAL(triggered()), this, SLOT(saveDataSet()));

     actionList.insert("newDataSetAct", newDataSetAct);
     actionList.insert("rmDataSetAct", rmDataSetAct);
     actionList.insert("rmAllDataSetAct", rmAllDataSetAct);
     actionList.insert("plotDataAct", plotDataAct);
     actionList.insert("saveDataSetAct", saveDataSetAct);
     actionList.insert("inspectEventAct",inspectEventAct);
     actionList.insert("renameDataSetAct",renameDataSetAct);


 }

 void MVdataManager::updateContextMenu()
 {
     //m_model->rowCount() >0 ? rmDataSetAct->setDisabled(false) : rmDataSetAct->setDisabled(true);
     if(m_model->rowCount() ==0)
     {
         rmDataSetAct->setDisabled(true);
         rmAllDataSetAct->setDisabled(true);
         plotDataAct->setDisabled(true);
         renameDataSetAct->setDisabled(true);
         inspectEventAct->setDisabled(true);
         saveDataSetAct->setDisabled(true);
     } else
     {
         rmDataSetAct->setDisabled(false);
         rmAllDataSetAct->setDisabled(false);
         plotDataAct->setDisabled(false);
         renameDataSetAct->setDisabled(false);
         inspectEventAct->setDisabled(false);
         saveDataSetAct->setDisabled(false);
     }

 }

 int MVdataManager::getNextAvailableIndex(){return 0;}


////////PRIVATE SLOTS/////////////

 void  MVdataManager::saveDataSet()
 {
     QModelIndexList toBeSaved = m_view->selectionModel()->selectedRows();

    if (toBeSaved.isEmpty()) {
        QMessageBox::warning(this, tr("Saving to File"),
                             tr("None of the data set is selected "));
        return;
    }
    if (toBeSaved.size()>1){
        QMessageBox::warning(this, tr("Saving to File"),
                             tr("More than one data set selected. \n "
                                " Only the first will be saved.\n "
                                " Selected N:%1 ").arg(toBeSaved.size()));

    }


     QString fileName = QFileDialog::getSaveFileName(this,
                         tr("Choose a file name"), ".",
                         tr("data (*.dat)"));
     if (fileName.isEmpty())
         return;
     QFile file(fileName);
     if (!file.open(QFile::WriteOnly | QFile::Text)) {
         QMessageBox::warning(this, tr("Saving to File"),
                              tr("Cannot write file %1:\n%2.")
                              .arg(fileName)
                              .arg(file.errorString()));
         return;
     }

     QDataStream out(&file);
     matrixData *mdata = m_model->mdata(toBeSaved.at(0));
     if (mdata == 0) return;
     mdata->ResetReadPointer();
     _EVENT event;
     event.Reset();
     QApplication::setOverrideCursor(Qt::WaitCursor);
     out<<0x11223344;
     out<<(uint32_t)mdata->GetSize();
     out<<0x44332211;
     while(mdata->GetNextEvent(event))
     {
         out << event.idx;
         out << event.TimeStamp;
         out << event.AbsTime;
         out << (uint32_t) event.hits.size();
         for (int i =0; i< event.hits.size(); i++)
         {
             out << event.hits[i].X_address;
             out << event.hits[i].Y_address;
         }
     }
     out<<0x11223344;
     out<<0x44332211;
     QApplication::restoreOverrideCursor();
 }

 void MVdataManager::plotSelectedData()
 {  QModelIndexList index = m_view->selectionModel()->selectedRows();
    //QModelIndex index = m_view->selectionModel()->currentIndex();
    if (index.isEmpty()) return;
    if (index.at(0).isValid()){
        QPair<matrixData*,QString> plotData;
        plotData.first = m_model->mdata(index.at(0));
        plotData.second = m_model->dataName(index.at(0));
        emit plotHisto(plotData);
    }
}

void MVdataManager::onFindChanged(const QString &text)
{
    if(text.isEmpty()) {
        m_view->selectionModel()->clear();
        return;
    }
    m_view->selectionModel()->clear();
    for (int i = 0; i < m_model->rowCount(); i ++ ){
        if (m_model->data(m_model->index(i, dataModel::DATASET_NAME), Qt::DisplayRole).toString().contains(text)){
            m_view->selectionModel()->select(m_model->index(i, dataModel::DATASET_NAME), QItemSelectionModel::Select);
        }
    }
}

void MVdataManager::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    menu->addAction(newDataSetAct);
    menu->addAction(renameDataSetAct);
    menu->addAction(plotDataAct);
    menu->addAction(inspectEventAct);
    menu->addAction(saveDataSetAct);
    menu->addAction(rmDataSetAct);
    menu->addAction(rmAllDataSetAct);

    menu->exec(event->globalPos());
    delete menu;
}

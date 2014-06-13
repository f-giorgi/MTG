#include <QtGui>
#include "maskmanager.h"

#include "maskEditor/checkablematrix.h"

MaskManager::MaskManager(QWidget *parent) :
    QWidget(parent)
{
    setMinimumSize(470,200);

    //create model
    mask_model = new QStandardItemModel();
    Nmasks = 0;
    //create view
    mask_view = new QTreeView(this);
    //set model to view
    mask_view->setModel(mask_model);
    mask_view->setIndentation(0);
    mask_view->setSelectionMode(QAbstractItemView::ContiguousSelection);


    mainLayout = new QVBoxLayout(this);

    mask_find = new QLineEdit(this);
    connect(mask_find, SIGNAL(textChanged(QString)), this, SLOT(onFindChanged(QString)));
    findLabel = new QLabel("Find: ");
    findLayout = new QHBoxLayout();
    findLayout->addWidget(findLabel);
    findLayout->addWidget(mask_find);

    mainLayout->addLayout(findLayout);
    mainLayout->addWidget(mask_view);
    createActions();
    setupModel();

    mask_view->setColumnWidth(MASKSET_NAME, 120);
    mask_view->setColumnWidth(MASK_COUNT, 60);
    mask_view->setColumnWidth(MAT_WIDTH, 60);
    mask_view->setColumnWidth(MAT_HEIGHT, 60);

}

void MaskManager::setupModel()
{
    //create the header labels for the tree view
    QStringList maskLabelList;
    maskLabelList << "Mask Set" << "Pix Masked" << "Width" << "Height";
    mask_model->setHorizontalHeaderLabels(maskLabelList);

    //creating the root item
    parentItem = mask_model->invisibleRootItem();

    //add a mask data set row
    addEmptyMaskSet();

    //make the address column hidden to the user
    mask_view->setColumnHidden(MASKSET_ADDR,true);

}

void MaskManager::addEmptyMaskSet()
{

    QList<QStandardItem *> row;
    _EVENT *evt_ptr = new _EVENT;
    evt_ptr->Reset();
    row << new QStandardItem(QString("Mask_%1").arg(Nmasks++))<< new QStandardItem("0")<< new QStandardItem("0")<< new QStandardItem("0") << new QStandardItem(QString("%1").arg(QString::number((uint64_t)evt_ptr,16)));
    foreach (QStandardItem * item, row) item->setEditable(false);
    parentItem->appendRow(row);
    updateContextMenu();
}

void MaskManager::createActions()
{
    newMaskSetAct = new QAction(QIcon(":/images/mask.png"), tr("&New Mask Set"), this);
    //newMaskSetAct->setShortcuts(QKeySequence::New);
    newMaskSetAct->setStatusTip(tr("Create a new empty Mask Set"));
    connect(newMaskSetAct, SIGNAL(triggered()), this, SLOT(addEmptyMaskSet()));

    renameMaskSetAct = new QAction(QIcon(":/images/renameMask2.png"),tr("Rename Mask Set"), this);
    renameMaskSetAct->setStatusTip(tr("Rename the selected Mask Set"));
    connect(renameMaskSetAct, SIGNAL(triggered()), this, SLOT(renameMaskSet()));

    editMaskAct = new QAction(QIcon(":/images/autobot.png"),tr("Edit Mask Set"), this);
    editMaskAct->setStatusTip(tr("Transform the selected Mask Set"));
    connect(editMaskAct, SIGNAL(triggered()), this, SLOT(editMask()));


    printMaskAct = new QAction(QIcon(":/images/print.png"),tr("Print Mask Set"), this);
    // printMaskAct->setShortcuts(QKeySequence::...);
    printMaskAct->setStatusTip(tr("Print the current Mask Set"));
    connect(printMaskAct, SIGNAL(triggered()), this, SLOT(printMaskSet()));

    rmMaskSetAct = new QAction(QIcon(":/images/deleteMask.png"),tr("Delete Mask Sets"), this);
    //rmMaskSetAct->setShortcuts(QKeySequence::Delete);
    rmMaskSetAct->setStatusTip(tr("Remove the selected Mask Sets"));
    connect(rmMaskSetAct, SIGNAL(triggered()), this, SLOT(rmMaskSet()));



    actionList.insert("newMaskSetAct", newMaskSetAct);
    actionList.insert("renameMaskSetAct",renameMaskSetAct);
    actionList.insert("editMaskAct",editMaskAct);
    actionList.insert("printMaskAct", printMaskAct);
    actionList.insert("rmMaskSetAct",rmMaskSetAct);

}

void MaskManager::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    menu->addAction(newMaskSetAct);
    menu->addAction(renameMaskSetAct);
    menu->addAction(editMaskAct);
    menu->addAction(printMaskAct);
    menu->addAction(rmMaskSetAct);
    menu->exec(event->globalPos());
    delete menu;
}

void MaskManager::printMaskSet()
{
    QModelIndexList index = mask_view->selectionModel()->selectedRows(MASKSET_ADDR);
        //QModelIndex index = m_view->selectionModel()->currentIndex();
        if (index.isEmpty()) return;
        bool * ok = new bool;
        if (index.at(0).isValid())
        {
            _EVENT * evt_ptr = reinterpret_cast<_EVENT *> (index.at(0).data(Qt::DisplayRole).toString().toUInt(ok, 16)) ;
            if (*ok) evt_ptr->Print();
        }
        delete ok;
}

void MaskManager::rmMaskSet()
{
     QModelIndexList itemsToBeRemoved = mask_view->selectionModel()->selectedRows(MASKSET_ADDR);
     _EVENT * evt_ptr = NULL;
     bool * ok = new bool;
     if (itemsToBeRemoved.isEmpty()) return;
     //for all selected items...
     int firstRow = itemsToBeRemoved.front().row();
     int lastRow = itemsToBeRemoved.back().row();
     int n = lastRow - firstRow + 1;
     for (int i = 0; i < n; i++)
     {
        evt_ptr = reinterpret_cast<_EVENT *> (itemsToBeRemoved.at(i).data(Qt::DisplayRole).toString().toUInt(ok, 16));
        if (*ok && evt_ptr)
            delete evt_ptr;
     }
     delete ok;
     mask_model->removeRows(firstRow, n);
     updateContextMenu();
 }

void MaskManager::updateContextMenu()
{
    //m_model->rowCount() >0 ? rmDataSetAct->setDisabled(false) : rmDataSetAct->setDisabled(true);
    if(mask_model->rowCount() == 0)
    {
       rmMaskSetAct->setDisabled(true);
       printMaskAct->setDisabled(true);
       renameMaskSetAct->setDisabled(true);
       editMaskAct->setDisabled(true);
    } else
    {
        rmMaskSetAct->setDisabled(false);
        printMaskAct->setDisabled(false);
        renameMaskSetAct->setDisabled(false);
        editMaskAct->setDisabled(false);
    }
}

void MaskManager::renameMaskSet()
{
    bool ok;
    QString text;

    QModelIndexList itemsToBeRenamed = mask_view->selectionModel()->selectedRows();
    if (itemsToBeRenamed.isEmpty()) return;
     QString baseName = itemsToBeRenamed.at(0).data(Qt::DisplayRole).toString();
    text = QInputDialog::getText(this, tr("Rename Mask Set"),
                                 tr("New Name:"), QLineEdit::Normal,
                                 baseName, &ok);
    if (ok && !text.isEmpty())
        mask_model->setData(itemsToBeRenamed.at(0), text, Qt::EditRole);
}

void MaskManager::onFindChanged(const QString &text)
{
    if(text.isEmpty()) {
        mask_view->selectionModel()->clear();
        return;
    }
    mask_view->selectionModel()->clear();
    for (int i = 0; i < mask_model->rowCount(); i ++ ){
        if (mask_model->data(mask_model->index(i, MASKSET_NAME), Qt::DisplayRole).toString().contains(text)){
            mask_view->selectionModel()->select(mask_model->index(i, MASKSET_NAME), QItemSelectionModel::Select);
        }
    }
}

QAction * MaskManager::getAction(const QString &actionName)
{
    return actionList[actionName];
}

void MaskManager::editMask()
{
   // checkableMatrix * maskEditor = new checkableMatrix(this);
   // maskEditor->show();
}

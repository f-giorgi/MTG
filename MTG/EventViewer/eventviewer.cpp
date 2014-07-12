#include "eventviewer.h"
#include <QtGui>
#include "plot/plot.h"

EventViewer::EventViewer(QWidget *parent) :
    QWidget(parent)
{
    EVLayout = new QVBoxLayout(this);
    m_data = 0;
    setMinimumHeight(500);

    titleLabel = new QLabel;
    setTitleLabel("DEMO");
    EVLayout->addWidget(titleLabel);


    evt_plot = new Plot(this);
    rasterData = (SpectrogramData*)  evt_plot->getSpectrogram()->data();
    rasterData->resetData();
    evt_plot->setMaximumSize(640,160);
    evt_plot->setMinimumSize(640,160);
    evt_plot->showContour(false);
    evt_plot->updateScale();

    EVLayout->addWidget(evt_plot);

    btnLayout = new QHBoxLayout();

    evtN_lab = new QLabel("Event N:",this);
    evtN_lcd = new QLCDNumber(6,this);
    integrate_chbx = new QCheckBox("Integrate",this);
    integrate_chbx->setProperty("isPixel",false);

    nextEvent_pbtn = new QPushButton(QIcon(":/images/next.png"),"Next",this);
    prevEvent_pbtn= new QPushButton(QIcon(":/images/prev.png"),"Prev",this);;
    rwd_pbtn = new QPushButton(QIcon(":/images/rew.png"),"RWD",this);
    fwd_pbtn = new QPushButton(QIcon(":/images/fwd.png"),"FWD",this);

    connect(nextEvent_pbtn, SIGNAL(clicked()), this, SLOT(nextEvent()));
    connect(prevEvent_pbtn, SIGNAL(clicked()), this, SLOT(prevEvent()));
    connect(rwd_pbtn, SIGNAL(clicked()), this, SLOT(rewindEvent()));
    connect(fwd_pbtn, SIGNAL(clicked()), this, SLOT(forwardEvent()));


    btnLayout->addWidget(evtN_lab);
    btnLayout->addWidget(evtN_lcd);
    btnLayout->addWidget(integrate_chbx);
    btnLayout->addWidget(prevEvent_pbtn);
    btnLayout->addWidget(nextEvent_pbtn);
    btnLayout->addWidget(rwd_pbtn);
    btnLayout->addWidget(fwd_pbtn);
    EVLayout->addLayout(btnLayout);

    evtView = new QTreeView(this);
    setupModel();
    evtView->setColumnWidth(0 , 180);
    EVLayout->addWidget(evtView);



}

bool EventViewer::matrixDataIsAvailable()
{
    bool ret = m_data == 0 ? false : true;
    return ret;
}
void EventViewer::setTitleLabel(const QString &text)
{
    QString title("Data Set : ");
    title.append(text);
    titleLabel->setText(title);
}

void EventViewer::setupModel()
{
    model = new QStandardItemModel(this);
    QStandardItem *parentItem = model->invisibleRootItem();

    QStringList headerLabels;
    headerLabels << " Item " << "Value";
    model->setHorizontalHeaderLabels(headerLabels);


    matrixDataItem = new QStandardItem("Matrix Data");
    matrixDataItem->setEditable(false);
    parentItem->appendRow(matrixDataItem);


    evtView->setModel(model);


}

void EventViewer::deleteAllChildren(QStandardItem * parent)
{
    //delete all children of eventsItem;
    QStandardItem * loopItem = parent; //main loop item
    QList<QStandardItem *> carryItems; //Last In First Out stack of items
    QList<QStandardItem *> itemsToBeDeleted; //List of items to be deleted
    while (loopItem->rowCount())
    {
        itemsToBeDeleted << loopItem->takeRow(0);
        //if the row removed has children:
        if (itemsToBeDeleted.at(0)->hasChildren())
        {
            carryItems << loopItem;                 //put on the stack the current loopItem
            loopItem = itemsToBeDeleted.at(0);      //set the row with children as the loopItem
        }
        //if current loopItem has no more rows but carryItems list is not empty:
        if (!loopItem->rowCount() && !carryItems.isEmpty()) loopItem = carryItems.takeFirst();

    }
    qDeleteAll(itemsToBeDeleted);

}

void EventViewer::attachMatrixData(QPair<matrixData*, QString> inspectData)
{

    m_data = inspectData.first;
    if (!matrixDataIsAvailable()) return;
    m_data->ResetReadPointer();

    rasterData = new SpectrogramData(m_data->GetMatrixXSize(),m_data->GetMatrixYSize());

    //setData delete previous rasterData if different
    evt_plot->getSpectrogram()->setData(rasterData);

    setTitleLabel(inspectData.second);

    //delete all children of matrixDataItem;
    deleteAllChildren(matrixDataItem);

    //set MData on model
    QList<QStandardItem *> row;

    row << new QStandardItem("X size")<< new QStandardItem(QString::number(m_data->GetMatrixXSize()));
    foreach (QStandardItem * item, row) item->setEditable(false);
    matrixDataItem->appendRow(row);
    row.clear();

    row << new QStandardItem("Y size")<< new QStandardItem(QString::number(m_data->GetMatrixYSize()));
    foreach (QStandardItem * item, row) item->setEditable(false);
    matrixDataItem->appendRow(row);
    row.clear();

    row << new QStandardItem("Events")<< new QStandardItem(QString::number(m_data->GetSize()));
    foreach (QStandardItem * item, row) item->setEditable(false);
    matrixDataItem->appendRow(row);
    eventsItem = row.at(0);
    row.clear();


    nextEvent();

    evtView->expandAll();
}

void EventViewer::updatePlot(_EVENT evt)
{
    if (!integrate_chbx->isChecked()) rasterData->resetData();
    foreach (_HIT hit, evt.hits) rasterData->increment(hit.X_address, hit.Y_address);
    //for (int i =0; i < evt.hits.size(); i++)rasterData->increment(evt.hits[i].X_address, evt.hits[i].Y_address);
    rasterData->setInterval( Qt::ZAxis, QwtInterval( 0.0, (double)rasterData->getMaxBin().first ));
    evt_plot->updateScale();
    evt_plot->replot();
}

void EventViewer::updateModel(_EVENT evt)
{
    if (!integrate_chbx->isChecked())
    {
        //delete all children of eventsItem;
        deleteAllChildren(eventsItem);
    }

    QList<QStandardItem *> row;
    QStandardItem *event = new QStandardItem(QString("Event %1").arg(evt.idx));

    row << new QStandardItem("idx")<< new QStandardItem(QString::number(evt.idx));
    foreach (QStandardItem * item, row) item->setEditable(false);
    event->appendRow(row);
    row.clear();

    row << new QStandardItem("Time Stamp")<< new QStandardItem(QString::number(evt.TimeStamp));
    foreach (QStandardItem * item, row) item->setEditable(false);
    event->appendRow(row);
    row.clear();

    row << new QStandardItem("Abs Time")<< new QStandardItem(QString::number(evt.AbsTime));
    foreach (QStandardItem * item, row) item->setEditable(false);
    event->appendRow(row);
    row.clear();

    hitsItem = new QStandardItem("Hits");
    row << hitsItem << new QStandardItem(QString::number(evt.hits.size()));
    foreach (QStandardItem * item, row) item->setEditable(false);
    event->appendRow(row);
    row.clear();

    for (int i = 0; i< evt.hits.size(); i++){
        QString hitDisplay(QString("X: %1 Y: %2").arg(evt.hits[i].X_address).arg(evt.hits[i].Y_address));
        row << new QStandardItem(QString("hit ").append(QString::number(i))) << new QStandardItem(hitDisplay);
        foreach (QStandardItem * item, row) item->setEditable(false);
        hitsItem->appendRow(row);
        row.clear();
    }

    eventsItem->appendRow(event);
    evtView->expandAll();
    //evtView->expand(event->index());
}

void EventViewer::nextEvent()
{
    if (!matrixDataIsAvailable()) return;
    _EVENT evt;
    evt.Reset();

    if (m_data->GetNextEvent(evt))
    {
        //treeView model update
        updateModel(evt);
        //update spectrogram data
        updatePlot(evt);
    }
}

void EventViewer::prevEvent()
{
    if (!matrixDataIsAvailable()) return;
    _EVENT evt;
    evt.Reset();

    if (m_data->GetPrevEvent(evt))
    {
        //treeView model update
        updateModel(evt);
        //update spectrogram data
        updatePlot(evt);
    }
}

void EventViewer::rewindEvent()
{
    if (!matrixDataIsAvailable()) return;
    m_data->ResetReadPointer();
    nextEvent();
}

void EventViewer::forwardEvent()
{
    if (!matrixDataIsAvailable()) return;
    _EVENT evt;
    evt.Reset();

    if (m_data->GetLastEvent(evt))
    {
        //treeView model update
        updateModel(evt);
        //update spectrogram data
        updatePlot(evt);
    }

}

#include <QtGui>

#include "datamodel.h"
#include "../matrixTools/matrixData.h"

dataModel::dataModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    headers << "Data Set Name" << "N Events" << "N Hits" << "Type" <<"Width" << "Height";
    Ndata = 0;
}

dataModel::~dataModel()
{
    foreach (matrixData* mD, dataSets){
        delete mD;
    }
}

 Qt::ItemFlags dataModel::flags(const QModelIndex & /*index*/) const
 {
     //  return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled ;
 }


 QVariant dataModel::data(const QModelIndex &index, int role) const
 {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    int row = index.row();
    int col = index.column();

    switch (role){
    case Qt::DisplayRole:
        return display.at(row).at(col);
    }
    return QVariant();
 }

QVariant dataModel::headerData(int section, Qt::Orientation orientation,
                    int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal && section < headers.size()) {
            return headers.at(section);
        }
    }
    return QVariant();
}

int dataModel::rowCount(const QModelIndex &parent) const
{
    return dataSets.size();
}

int dataModel::columnCount(const QModelIndex &parent) const
{
    return headers.size();
}

bool dataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
       return false;
    if (role == Qt::EditRole)
    {
        display[index.row()][index.column()] = value.toString();
        return true;
    }
    return false;
}

bool dataModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{bool ret = true;
    if (ret) emit headerDataChanged(orientation, section, section);
    return ret;
}

bool dataModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    return true;
}

bool dataModel::removeColumns(int position, int columns, const QModelIndex &parent )
{
    return true;
}

bool dataModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    if (position < 0 || position > dataSets.size())
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    for (int row = 0; row < rows; row++) {
        matrixData *mdata = new matrixData();
            qDebug()<<"created matrixData at address: " << mdata;
        dataSets.insert(position + row, mdata);
        QStringList items;
        QString itemName = QString("Data_%1").arg(Ndata);
        items << itemName << "0" << "0" << "UNKNOWN" << "0" << "0";
        display.insert(position + row, items);
        Ndata ++;
    }
    endInsertRows(); 
    return true;
}

bool dataModel::appendRow(const QModelIndex &parent)
{
   bool ret =  insertRows(dataSets.size(), 1, parent);
   emit columnDataChanged(DATASET_NAME);
   emit columnDataChanged(EVENT_COUNT);
   emit columnDataChanged(HIT_COUNT);
   emit columnDataChanged(TYPE_NAME);
   emit columnDataChanged(MTX_WIDTH);
   emit columnDataChanged(MTX_HEIGHT);
   return ret;
}

bool dataModel::removeRows(int position, int rows, const QModelIndex &parent )
{
    if (position <0 || position >= dataSets.size())
        return false;

    beginRemoveRows(QModelIndex(), position, position+rows-1);
    for (int row = 0; row < rows; ++row) {
         qDebug()<<"deleting matrixData class";
         matrixData * mdata = dataSets.at(position);
         qDebug()<< "retrieved matrixData pointer is: " << mdata;
         if (mdata)
         {
            delete mdata;
            qDebug()<<"pop back data set";
            dataSets.takeAt(position);
            qDebug()<<"pop back display";
            display.takeAt(position);
         }
}
        endRemoveRows();
        return true;

}

bool dataModel::hasChildren ( const QModelIndex & parent ) const
{
    if (parent.isValid()) return false;
    return true;
}

matrixData *  dataModel::mdata(const QModelIndex &index)
{
    return dataSets.at(index.row());
}

QString dataModel::dataName(const QModelIndex &index)
{
    return display.at(index.row()).at(DATASET_NAME);
}

QModelIndex dataModel::index(int row, int column, const QModelIndex &parent) const
{
  return createIndex(row,column);
}

QModelIndex dataModel::parent(const QModelIndex &index) const
{
  return QModelIndex();
}


QStandardItem *dataModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        QStandardItem *item = static_cast<QStandardItem*>(index.internalPointer());
        if (item) return item;
    }
    return 0;
}

void dataModel::setEventSize(int row, int size)
{
    display[row][EVENT_COUNT] = QString::number(size);
    emit columnDataChanged(EVENT_COUNT);
}
void dataModel::setHitSize(int row, int size)
{
    display[row][HIT_COUNT] = QString::number(size);
    emit columnDataChanged(HIT_COUNT);
}

void dataModel::setMatrixSize(int row, int width, int height)
{
 display[row][MTX_WIDTH] = QString::number(width);
  emit columnDataChanged(MTX_WIDTH);
 display[row][MTX_HEIGHT] = QString::number(height);
  emit columnDataChanged(MTX_HEIGHT);
}

void dataModel::setTypeName(int row, QString name)
{
    display[row][TYPE_NAME] = name;
    emit columnDataChanged(TYPE_NAME);
}

QStringList dataModel::getDsList()
{
    QStringList list;

    for (int i=0; i< display.size(); i++)
    {
        list << display.at(i);
    }
    return list;
}

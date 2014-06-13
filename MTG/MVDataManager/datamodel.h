#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QVector>
#include <QStringList>

class matrixData;
class QStandardItem;

class dataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    dataModel(QObject *parent = 0);
    ~dataModel();
    enum ColumnIndex{
        DATASET_NAME = 0,
        EVENT_COUNT = 1,
        HIT_COUNT = 2,
        TYPE_NAME = 3,
        MTX_WIDTH = 4,
        MTX_HEIGHT = 5
    };

  //ITEM DATA HANDLING
    //reimplemented functions for read-only access
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    //reimplemented functions for editable items in model
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole);

    //reimplemented functions for resizable models

    bool insertColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex());
    bool removeColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex());
    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());
    bool appendRow(const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex());

    bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;

    matrixData * mdata(const QModelIndex &index);
    QString dataName(const QModelIndex &index);
    void setEventSize(int row, int size);
    void setHitSize(int row, int size);
    void setMatrixSize(int row, int width, int height);
    void setTypeName(int row, QString name);

  //NAVIGATION AND INDEX CREATION --maybe not required for table model;
    //parents and children
    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const ;
    QModelIndex parent(const QModelIndex &index) const;
    QStandardItem * getItem(const QModelIndex &index) const;
    QStringList getDsList();
signals:
     void columnDataChanged(int);

public slots:

private:
    QList< matrixData*> dataSets;
    QStringList headers;
    QList<QStringList> display;
    int Ndata;
};

#endif // DATAMODEL_H

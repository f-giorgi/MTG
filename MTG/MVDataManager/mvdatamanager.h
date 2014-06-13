#ifndef MVDATAMANAGER_H
#define MVDATAMANAGER_H

#include <QtGui>
#include "../../matrixTools/parser.h"

typedef QStandardItemModel ItemModel;

class dataModel;


class MVdataManager : public QWidget
{
    Q_OBJECT
public:


    explicit MVdataManager(QWidget *parent = 0);
    ~MVdataManager();
    void setupModel();
    QAction * getAction(const QString &actionName);
    bool importFile(QString fileName, QModelIndex & itemIndex, _DATATYPE dataType, _VERBOSITY verbosity);
    dataModel *m_model;


public slots:
    void addEmptyDataSet();
    void rmDataSet();
    void renameDataSet();
    void rmAllDataSet();
    void inspect();
    QStringList getDsList();

signals:
    void plotHisto(QPair<matrixData*, QString>);
    void inspectMatrixData(QPair<matrixData*, QString>);
    void outputLog(QString);


private:

    QTreeView *m_view;
    QLineEdit *m_find;
    QLabel *findLabel;
    QVBoxLayout *layout;
    QHBoxLayout *findLayout;

    void createActions();
    void updateContextMenu();
    int getNextAvailableIndex();
    QVBoxLayout *vlayout;
    QString headerLabels;
    QAction *newDataSetAct;
    QAction *rmDataSetAct;
    QAction *renameDataSetAct;
    QAction *rmAllDataSetAct;
    QAction *inspectEventAct;
    QAction *plotDataAct;
    QAction *saveDataSetAct;
    QMap<QString, QAction *> actionList;


private slots:
    void saveDataSet();
    void plotSelectedData();
    void onFindChanged(const QString &text);
    void contextMenuEvent(QContextMenuEvent *event);

};

#endif // MVDATAMANAGER_H

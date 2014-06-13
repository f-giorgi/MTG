#ifndef MASKMANAGER_H
#define MASKMANAGER_H

#include <QtGui>
#include "../matrixTools/matrixData.h"
//QT_BEGIN_NAMESPACE
//class QTreeView;
//class QLineEdit;
//class QLabel;
//class QVBoxLayout;
//class QHBoxLayout;
//class QStandardItemModel;
//class QStandardItem;
//class QAction;
//class QString;
//QT_END_NAMESPACE

class MaskManager : public QWidget
{
    Q_OBJECT
public:
    explicit MaskManager(QWidget *parent = 0);
    enum ColumnIndex{
        MASKSET_NAME = 0,
        MASK_COUNT = 1,
        MAT_WIDTH = 2,
        MAT_HEIGHT = 3,
        MASKSET_ADDR = 4
    };
    QAction * getAction(const QString &actionName);


private:
    int Nmasks;
    QTreeView *mask_view;
    QLineEdit *mask_find;
    QHBoxLayout *findLayout;
    QLabel *findLabel;
    QVBoxLayout *mainLayout;
    QStandardItemModel *mask_model;
    QStandardItem *parentItem;
    QAction * newMaskSetAct;
    QAction * printMaskAct;
    QAction * rmMaskSetAct;
    QAction * renameMaskSetAct;
    QAction * editMaskAct;
    QMap<QString, QAction *> actionList;
    void setupModel();
    void createActions();
    void updateContextMenu();


private slots:
    void contextMenuEvent(QContextMenuEvent *event);
    void printMaskSet();
    void onFindChanged(const QString &text);
    void editMask();

signals:
      void outputLog(QString);
public slots:
     void addEmptyMaskSet();
     void rmMaskSet();
     void renameMaskSet();
};

#endif // MASKMANAGER_H

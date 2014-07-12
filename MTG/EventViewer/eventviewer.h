#ifndef EVENTVIEWER_H
#define EVENTVIEWER_H

#include <QWidget>
#include "../matrixTools/matrixData.h"

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLCDNumber;
class QLabel;
class QCheckBox;
class QTreeView;
class Plot;
class SpectrogramData;
class QStandardItemModel;
class QStandardItem;
QT_END_NAMESPACE

class EventViewer : public QWidget
{
    Q_OBJECT
public:
    explicit EventViewer(QWidget *parent = 0);
    bool matrixDataIsAvailable();
signals:
      void outputLog(QString);
public slots:
    void setTitleLabel(const QString &text);
    void attachMatrixData(QPair<matrixData *, QString> inspectData);
    void updateModel(_EVENT evt);
    void updatePlot(_EVENT evt);
    void nextEvent();
    void prevEvent();
    void rewindEvent();
    void forwardEvent();

private:
    QVBoxLayout * EVLayout;
    QHBoxLayout * btnLayout;
    QCheckBox * integrate_chbx;
    QPushButton * nextEvent_pbtn;
    QPushButton * prevEvent_pbtn;
    QPushButton * rwd_pbtn;
    QPushButton * fwd_pbtn;
    QLCDNumber * evtN_lcd;
    QLabel * evtN_lab;
    QLabel * titleLabel;
    QTreeView * evtView;
    QStandardItemModel *model;
    QStandardItem *eventsItem;
    QStandardItem *matrixDataItem;
    QStandardItem *hitsItem;
    Plot *evt_plot;
    SpectrogramData *rasterData;
    matrixData *m_data;

    void setupModel();
    void deleteAllChildren(QStandardItem * parent);

};

#endif // EVENTVIEWER_H

#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QwtSlider;
class QVBoxLayout;
class QHBoxLayout;
class Plot;
class matrixData;
class QLineEdit;
class QLabel;
class QGroupBox;
class QCheckBox;
QT_END_NAMESPACE

class PlotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlotWidget(QWidget *parent = 0);
    
signals:
    void outputLog(QString);

public slots:
    void histoMatrix(QPair<matrixData *, QString> plotData);
    void setTitleLabel(const QString &text);

private:
    int maxBin;
    QVBoxLayout *plotWidgetLayout;
    QVBoxLayout *slidersLayout;
    QGroupBox *slidersGB;
    QCheckBox *microBoPatchChbx;
    Plot *m_plot;
    QwtSlider *zScaleSliderTOP;
    QwtSlider *zScaleSliderBOTTOM;


    QLabel * titleLabel;

    void updateSliderRange();
private slots:
    void setZscaleTOP(double value);
    void setZscaleBOTTOM(double value);

};

#endif // PLOTWIDGET_H

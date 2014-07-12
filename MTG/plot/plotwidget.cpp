#include <QtGui>
#include <qwt_slider.h>
#include "plotwidget.h"
#include "plot.h"

PlotWidget::PlotWidget(QWidget *parent) :
    QWidget(parent)
{
    plotWidgetLayout = new QVBoxLayout(this);

    titleLabel = new QLabel();
    setTitleLabel("DEMO");
    plotWidgetLayout->addWidget(titleLabel);

    m_plot = new Plot(this);
    m_plot->setMaximumSize(640,160);
    m_plot->setMinimumSize(640,160);
    m_plot->showContour(false);
    plotWidgetLayout->addWidget(m_plot);


    zScaleSliderTOP = new QwtSlider(this, Qt::Horizontal);
    zScaleSliderBOTTOM = new QwtSlider(this, Qt::Horizontal);

    maxBin = m_plot->getMaxBin().first;
    zScaleSliderTOP->setRange(0.0, (double)maxBin);
    zScaleSliderTOP->fitValue((double)maxBin);

    zScaleSliderBOTTOM->setRange(0.0, (double)maxBin);
    zScaleSliderBOTTOM->fitValue(0.0);

    connect(zScaleSliderTOP, SIGNAL(valueChanged(double)), this, SLOT(setZscaleTOP(double)));
    connect(zScaleSliderBOTTOM, SIGNAL(valueChanged(double)), this, SLOT(setZscaleBOTTOM(double)));

    slidersGB = new QGroupBox("Z Scale Range",this);
    slidersGB->setMaximumWidth(640);
    slidersGB->setMinimumWidth(640);


    slidersLayout = new QVBoxLayout();

    slidersLayout->addWidget(new  QLabel("Max Value"));
    slidersLayout->addWidget(zScaleSliderTOP);
    slidersLayout->addWidget(new  QLabel("Min Value"));
    slidersLayout->addWidget(zScaleSliderBOTTOM);

    slidersGB->setLayout(slidersLayout);
    plotWidgetLayout->addWidget(slidersGB);

    microBoPatchChbx = new QCheckBox("MicroBo Plot Patch");
    microBoPatchChbx->setChecked(false);
    plotWidgetLayout->addWidget(microBoPatchChbx);


    QWidget * spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    plotWidgetLayout->addWidget(spacer);

}

void PlotWidget::updateSliderRange()
{
    int maxBin = m_plot->getMaxBin().first;
    zScaleSliderTOP->setRange(0.0, (double)maxBin);
    zScaleSliderTOP->fitValue((double)maxBin);
    zScaleSliderBOTTOM->setRange(0.0, (double)maxBin);
    zScaleSliderBOTTOM->fitValue(0.0);

}

void PlotWidget::setTitleLabel(const QString &text)
{
    QString title("Data Set : ");
    title.append(text);
    titleLabel->setText(title);
}


void PlotWidget::histoMatrix(QPair<matrixData*, QString> plotData)
{
    m_plot->histoMatrix(plotData.first, microBoPatchChbx->isChecked());
    setTitleLabel(plotData.second);
    updateSliderRange();
}


void PlotWidget::setZscaleBOTTOM(double value)
{
    //adjust sliders
    m_plot->setZscale(value, zScaleSliderTOP->value());
}

void PlotWidget::setZscaleTOP(double value)
{
    //adjust sliders
    m_plot->setZscale(zScaleSliderBOTTOM->value(), value);
}

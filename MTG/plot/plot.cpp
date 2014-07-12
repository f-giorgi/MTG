#include <qprinter.h>
#include <qprintdialog.h>
//#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
//#include <qwt_plot_zoomer.h>
//#include <qwt_plot_panner.h>
//#include <qwt_plot_layout.h>
//#include <qwt_plot_renderer.h>
#include "plot.h"
#include "../matrixTools/matrixData.h"




Plot::Plot(QWidget *parent, uint16_t MatXSize, uint16_t MatYSize):
    QwtPlot(parent)
{
    d_spectrogram = new QwtPlotSpectrogram();
    d_spectrogram->setRenderThreadCount(0); // use system specific thread count

    d_spectrogram->setColorMap(new ColorMap());

    d_spectrogram->setData(new SpectrogramData(MatXSize, MatYSize));
    SpectrogramData * rasterData = (SpectrogramData *)d_spectrogram->data();
    rasterData->fillDemoPixelMap();

    d_spectrogram->attach(this);

    QList<double> contourLevels;
    for ( double level = 0.5; level < 1000.0; level += 100.0 )
        contourLevels += level;
    d_spectrogram->setContourLevels(contourLevels);

    // A color bar on the right axis
    rightAxis = axisWidget(QwtPlot::yRight);
    rightAxis->setTitle("Intensity");
    rightAxis->setColorBarEnabled(true);

    //updateZScale();

    //setAxisScale(QwtPlot::yLeft,-0.5,31.5);
    //setAxisScale(QwtPlot::xBottom,-0.5,127.5);
    updateScale();
    plotLayout()->setAlignCanvasToScales(false);

    replot();
    // LeftButton for the zooming
    // MidButton for the panning
    // RightButton: zoom out by 1
    // Ctrl+RighButton: zoom out to full size

    QwtPlotZoomer* zoomer = new MyZoomer(canvas());
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
        Qt::RightButton, Qt::ControlModifier);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
        Qt::RightButton);

    QwtPlotPanner *panner = new QwtPlotPanner(canvas());
    panner->setAxisEnabled(QwtPlot::yRight, false);
    panner->setMouseButton(Qt::MidButton);

    // Avoid jumping when labels with more/less digits
    // appear/disappear when scrolling vertically

    const QFontMetrics fm(axisWidget(QwtPlot::yLeft)->font());
    QwtScaleDraw *sd = axisScaleDraw(QwtPlot::yLeft);
    sd->setMinimumExtent( fm.width("100.00") );

    const QColor c(Qt::darkBlue);
    zoomer->setRubberBandPen(c);
    zoomer->setTrackerPen(c);
}

QwtPlotSpectrogram * Plot::getSpectrogram()
{
    return  d_spectrogram;
}

QPair<int, QPair<int, int> > Plot::getMaxBin()
{
    SpectrogramData * rasterData = (SpectrogramData *)d_spectrogram->data();
    return rasterData->getMaxBin();
}


void Plot::showContour(bool on)
{
    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, on);
    replot();
}

void Plot::showSpectrogram(bool on)
{
    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, on);
    d_spectrogram->setDefaultContourPen(on ? QPen() : QPen(Qt::NoPen));
    replot();
}

//void Plot::resizeEvent ( QResizeEvent * event )
//{


//}

#ifndef QT_NO_PRINTER

void Plot::printPlot()
{
#if 1
    QPrinter printer;
#else
    QPrinter printer(QPrinter::HighResolution);
#endif
    printer.setOrientation(QPrinter::Landscape);
    printer.setOutputFileName("spectrogram.pdf");
    QPrintDialog dialog(&printer);
    if ( dialog.exec() )
    {
        QwtPlotRenderer renderer;

        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, false);
        renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames, true);

        renderer.renderTo(this, printer);
    }
}

void Plot::setZscale(double MINvalue, double MAXvalue)
{
    QwtInterval interval;
    interval.setMaxValue(MAXvalue);
    interval.setMinValue(MINvalue);
    d_spectrogram->data()->setInterval( Qt::ZAxis,interval);
    updateZScale();
    replot();
}

void Plot::histoMatrix(matrixData *mdata, bool patch)
{
    qDebug()<<"Filling Histogram";
    _EVENT evt;
    evt.Reset();
    mdata->ResetReadPointer();

    uint16_t x,y;

    //controlla che matrix data esista e che abbia  dei dati!!
    SpectrogramData * rasterData = new SpectrogramData(mdata->GetMatrixXSize(),mdata->GetMatrixYSize());
    d_spectrogram->setData(rasterData);

    //rasterData = (SpectrogramData *)d_spectrogram->data();
    //rasterData->resetData();
    while(mdata->GetNextEvent(evt))
    {
        for (int i=0; i<evt.GetSize(); i++)
        {   x = evt.hits[i].X_address;
            y = evt.hits[i].Y_address;
            //rasterData->pixels[x*rasterData->height+y]++;
            rasterData->increment(x,y);
            if (patch) rasterData->increment(x,y+1);
        }
    }
    rasterData->setInterval( Qt::ZAxis, QwtInterval( 0.0, (double)rasterData->getMaxBin().first ));
    updateScale();
    replot();
}

void  Plot::updateZScale()
{
    zInterval = d_spectrogram->data()->interval( Qt::ZAxis );
    rightAxis->setColorMap(zInterval, new ColorMap());

    setAxisScale(QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue());
    enableAxis(QwtPlot::yRight);


}
void  Plot::updateScale()
{   xInterval = d_spectrogram->data()->interval( Qt::XAxis );
    yInterval = d_spectrogram->data()->interval( Qt::YAxis );
    zInterval = d_spectrogram->data()->interval( Qt::ZAxis );

    rightAxis->setColorMap(zInterval, new ColorMap());

    setAxisScale(QwtPlot::xBottom, xInterval.minValue(), xInterval.maxValue());
    setAxisScale(QwtPlot::yLeft, yInterval.minValue(),yInterval.maxValue());
    setAxisScale(QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue());
    enableAxis(QwtPlot::yRight);

}

#endif

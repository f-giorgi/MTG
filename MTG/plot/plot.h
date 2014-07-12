#include <qwt_plot.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <stdint.h>

#define MATX_SIZE 128
#define MATY_SIZE 32


class matrixData;
class QwtScaleWidget;
class QwtInterval;

class MyZoomer: public QwtPlotZoomer
{
public:
    MyZoomer(QwtPlotCanvas *canvas):
        QwtPlotZoomer(canvas)
    {
        setTrackerMode(AlwaysOn);
    }

    virtual QwtText trackerTextF(const QPointF &pos) const
    {
        QColor bg(Qt::white);
        bg.setAlpha(200);

        QwtText text = QwtPlotZoomer::trackerTextF(pos);
        text.setBackgroundBrush( QBrush( bg ));
        return text;
    }
};

class SpectrogramData: public QwtRasterData
{
public:

    uint32_t *pixels;
    //uint32_t **ppixels;

    uint16_t width;
    uint16_t height;
    ~SpectrogramData(){ delete[] pixels; }
    SpectrogramData(uint16_t Xsize, uint16_t Ysize)
    {
        //set internal reference matrix size
        width = Xsize;
        height = Ysize;



        //creating the 1D arraY
        pixels = new uint32_t[width*height];
        //ppixels = &pixels;
        resetData();
        //fillDemoPixelMap();
        //qDebug()<< ppixels[0][0];
        updateIntervals();
    }

    QPair<int, QPair<int, int> > getMaxBin()
    {   QPair<int, QPair<int, int> > ret;
        QPair<int, int> maxBin;
        maxBin.first = 0;
        maxBin.second = 0;
        int max = 0;
        for (int i =0; i<width; i++){
            for (int j = 0; j < height; j++){
                if (pixels[height*i+j] > max)
                {
                    maxBin.first = i;
                    maxBin.second = j;
                    max = pixels[height*i+j];
                }
            }
        }
        ret.first = max;
        ret.second = maxBin;
        return ret;
    }

    void fillDemoPixelMap()
    {

        for (int i =0; i<width; i++){
            for (int j = 0; j < height; j++){
                pixels[height*i+j]= i*j;
            }
        }
        updateIntervals();
    }

    void resetData(){
        for (int i =0; i<width; i++){
            for (int j = 0; j < height; j++){
                pixels[height*i+j]= 0;
            }
        }
        updateIntervals();
    }

    void updateIntervals(){
        setInterval( Qt::XAxis, QwtInterval( 0.0, (double)width-0.01  ) );
        setInterval( Qt::YAxis, QwtInterval( 0.0, (double)height-0.01  ) );
        //this has to be set dynamically
        setInterval( Qt::ZAxis, QwtInterval( 0.0, (double)getMaxBin().first ));

    }

    virtual double value(double x, double y) const
    {
        if (x<width && y <height && x>=0 && y>=0)
            return (double)pixels[((int)x*height)+(int)y];
        else
            qDebug()<<"invalid value requested"<< x <<" "<<y;
        return -1.;
    }

    bool increment(uint16_t x, uint16_t y)
    {
        if (x<0 || y<0 || x >= width || y >= height) return false;
        pixels[x*height+y]++;
        return true;
    }

    uint32_t getBinValue(uint16_t x, uint16_t y)
    {
        if (x<0 || y<0 || x >= width || y >= height) return 0;
        return pixels[(x*height)+y];
    }

};

class ColorMap: public QwtLinearColorMap
{
public:
    ColorMap():
        QwtLinearColorMap(Qt::darkCyan, Qt::red)
    {
        addColorStop(0.1, Qt::cyan);
        addColorStop(0.6, Qt::green);
        addColorStop(0.95, Qt::yellow);
    }
};



class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget *parent = NULL, uint16_t MatXSize = MATX_SIZE, uint16_t MatYSize = MATY_SIZE);
    QwtPlotSpectrogram *getSpectrogram();
    QPair<int, QPair<int, int> > getMaxBin();

public Q_SLOTS:
    void showContour(bool on);
    void showSpectrogram(bool on);
    void setZscale(double MINvalue, double MAXvalue);
    void histoMatrix(matrixData *mdata, bool patch);
    void updateZScale();
    void updateScale();


#ifndef QT_NO_PRINTER
    void printPlot();
#endif
protected:
 // void resizeEvent ( QResizeEvent * event );

private:
    QwtPlotSpectrogram *d_spectrogram;
    QwtScaleWidget *rightAxis;
    QwtInterval zInterval;
    QwtInterval xInterval;
    QwtInterval yInterval;

};

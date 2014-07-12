#ifndef CHECKABLEMATRIX_H
#define CHECKABLEMATRIX_H
#define MATX_SIZE 128
#define MATY_SIZE 32

#include <QWidget>
#include <stdint.h>
QT_BEGIN_NAMESPACE
class QCheckBox;
class QGridLayout;
QT_END_NAMESPACE

class checkableMatrix : public QWidget
{
    Q_OBJECT
public:
    explicit checkableMatrix(QWidget *parent = 0, uint16_t X_size= MATX_SIZE, uint16_t Y_size= MATY_SIZE);
    uint16_t width;
    uint16_t height;

signals:
    
public slots:
    
private:
    QCheckBox ** pixels;
    QGridLayout * gridLayout;
    void setPixel(uint16_t x, uint16_t y, QCheckBox * pixel);
    QCheckBox* getPixel(uint16_t x, uint16_t y);
    void setChecked(uint16_t x, uint16_t y,bool val);
    bool isChecked(uint16_t x, uint16_t y);
    bool coordAreValid(uint16_t x, uint16_t y);
};

#endif // CHECKABLEMATRIX_H

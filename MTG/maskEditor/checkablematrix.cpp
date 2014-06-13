#include "checkablematrix.h"
#include <QtGui>

checkableMatrix::checkableMatrix(QWidget *parent, uint16_t X_size, uint16_t Y_size) :
    QWidget(parent)
{
    width = X_size;
    height = Y_size;
    pixels = new  QCheckBox*[width*height];
    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(0);
    for(int i =0; i< width; i++)
        for (int j=0; j< height; j++){
            setPixel(i,j, new QCheckBox(this));
            gridLayout->addWidget(getPixel(i,j),j,i,1,1);
        }
    setMinimumSize(width*10,height*10);
    setMaximumSize(width*10,height*10);

}

void checkableMatrix::setPixel(uint16_t x, uint16_t y, QCheckBox * pixel)
{
    pixels[x*height+y]= pixel;
    pixel->setProperty("isPixel",true);
}

QCheckBox* checkableMatrix::getPixel(uint16_t x, uint16_t y)
{
    if (coordAreValid(x,y))
        return pixels[x*height+y];
    return NULL;

}

void checkableMatrix::setChecked(uint16_t x, uint16_t y,bool val)
{
    if (coordAreValid(x,y))
    getPixel(x,y)->setChecked(val);
}

bool checkableMatrix::isChecked(uint16_t x, uint16_t y)
{
    if (coordAreValid(x,y))
        return getPixel(x,y)->isChecked();
    return false;
}

bool checkableMatrix::coordAreValid(uint16_t x, uint16_t y)
{
    if (x>=0 && y>=0 && x<width && y<height)
        return true;
    return false;
}

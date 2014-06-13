#include "pixelswitch.h"

pixelSwitch::pixelSwitch(QWidget *parent) :
    QCheckBox(parent)
{
}
QSize pixelSwitch::sizeHint() const{
    QString c = checkState()==Qt::Checked ? ":/images/pix_checked.png" : ":/images/pix_unchecked.png";
    return QPixmap(c).size();
}

void pixelSwitch::paintEvent(QPaintEvent *){
    QPainter p(this);
    QString c = checkState() == Qt::Checked ? ":/images/pix_checked.png" : ":/images/pix_unchecked.png";
    p.drawPixmap(QPoint(0,0), QPixmap(c));
}

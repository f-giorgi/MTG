#ifndef PIXELSWITCH_H
#define PIXELSWITCH_H

#include <QtGui>

class pixelSwitch : public QCheckBox
{
    Q_OBJECT
public:
    explicit pixelSwitch(QWidget *parent = 0);
    QSize sizeHint() const;
protected:
void paintEvent(QPaintEvent *);

signals:
    
public slots:
    
};

#endif // PIXELSWITCH_H

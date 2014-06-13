#ifndef IMPORTDILAOG_H
#define IMPORTDILAOG_H
#include <QtGui>
#include <QDialog>
//#include "../matrixTools/qmatrixdata.h"

#include "../matrixTools/parser.h"


class dataModel;

namespace Ui {
class ImportDilaog;
}

class ImportDilaog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ImportDilaog(QWidget *parent = 0);
    ~ImportDilaog();
    void fillDataSets(dataModel *model);

    QString getPath();
    int getType();
    int getDataSetIndex();
    int getVerbosityIndex();

private:
    Ui::ImportDilaog *ui;


private slots:
    void setFilePath();
    void filePathHasChanged();
};

#endif // IMPORTDILAOG_H

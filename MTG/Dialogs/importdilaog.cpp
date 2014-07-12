#include "importdilaog.h"
#include "ui_importdilaog.h"
#include "../MVDataManager/datamodel.h"

ImportDilaog::ImportDilaog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDilaog)
{
    ui->setupUi(this);

    //set by default the OK button disabled
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    //set the file type
    char itemText[20];
    for (int i =1; i <INVALID_DataType; i ++){
        parser::getDataTypeName(i,itemText);
        ui->dataType_cmbx->addItem(itemText);
    }

    //set the verbosity
    for (int i =1; i <INVALID_VerbosityLevel; i ++){
        parser::getVerbosityLevelName(i,itemText);
        ui->verbosity_cmbx->addItem(itemText);
    }


//    parser::getDataTypeName(parser::APSEL4DmicroBOLOGNA,itemText);
//    ui->dataType_cmbx->addItem(itemText);
//    parser::getDataTypeName(parser::DAQLayerDump,itemText);
//    ui->dataType_cmbx->addItem(itemText);
//    parser::getDataTypeName(parser::FE4DtestPISA,itemText);
//    ui->dataType_cmbx->addItem(itemText);
//    parser::getDataTypeName(parser::simuGenList,itemText);
//    ui->dataType_cmbx->addItem(itemText);
//    parser::getDataTypeName(parser::simuSORTEX200x256,itemText);
//    ui->dataType_cmbx->addItem(itemText);
//    parser::getDataTypeName(parser::simuSQARE200x256,itemText);
//    ui->dataType_cmbx->addItem(itemText);
//    parser::getDataTypeName(parser::simuSQARE2x16x32,itemText);
//    ui->dataType_cmbx->addItem(itemText);
//    parser::getDataTypeName(parser::StripFEBuffer,itemText);
//    ui->dataType_cmbx->addItem(itemText);


    connect(ui->selectFile_pbtn, SIGNAL(clicked()), this, SLOT(setFilePath()));
    connect(ui->filePath_ledt, SIGNAL(textChanged(QString)), this, SLOT(filePathHasChanged()));
}

ImportDilaog::~ImportDilaog()
{
    delete ui;
}

void ImportDilaog::setFilePath()
{
    ui->filePath_ledt->setText(QFileDialog::getOpenFileName(this,tr("Select the file to import"),"."));

}

void ImportDilaog::filePathHasChanged()
{
    if (ui->filePath_ledt->text().length()!=0){
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void ImportDilaog::fillDataSets(dataModel *model)
{
    ui->slctDataSet_cmbx->setModel(model);
}

//void ImportDilaog::fillDataSets(QList<QStandardItem *> list)
//{
//    foreach (QStandardItem *item, list ){
////    for (int i=0; i < list.length();i++)
//       ui->slctDataSet_cmbx->addItem(item->text(),item->index());
//    }
//}

QString ImportDilaog::getPath()
{
   return  ui->filePath_ledt->text();
}

int ImportDilaog::getType()
{
    return ui->dataType_cmbx->currentIndex()+1;//!! Type index starts from 1
}

int ImportDilaog::getDataSetIndex()
{
    return ui->slctDataSet_cmbx->currentIndex();//Dataset index starts from 0
}

int ImportDilaog::getVerbosityIndex()
{
    return ui->verbosity_cmbx->currentIndex()+1;//!! Verbosity index starts from 1
}




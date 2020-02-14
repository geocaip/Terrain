#include "cpgeoinfo.h"
#include "ui_cpgeoinfo.h"
#include"QMessageBox"
#include"ogr_spatialref.h"
CPGeoInfo::CPGeoInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CPGeoInfo)
{
    ui->setupUi(this);
    ui->type->setCurrentIndex(0);
	ui->label_2->setEnabled(false);
	ui->proj->setEnabled(false);
}

CPGeoInfo::~CPGeoInfo()
{
    delete ui;
}
void CPGeoInfo::getData(QMap<QString,vector<pair<int,QString>>>prjInfo)
{
    myprjInfo=prjInfo;
    if(prjInfo.size()<=0)
    {
        return;
    }
    ui->sphere->clear();
    for(int i=0;i<prjInfo.size();i++)
    {
         ui->sphere->addItem(prjInfo.keys()[i]);
    }
    ui->sphere->setCurrentIndex(0);
    ui->proj->clear();
	EPSGs.clear();
    for(int i=0;i<prjInfo.values()[0].size();i++)
    {
         ui->proj->addItem(prjInfo.values()[0][i].second);
		 EPSGs[i] = prjInfo.values()[0][i].first;
    }
	

}
void CPGeoInfo::on_cancel_clicked()
{
    this->close();
}

void CPGeoInfo::on_ok_clicked()
{

	int  EPSG;
	if (ui->type->currentIndex()==1)
	{
		EPSG = EPSGs[ui->proj->currentIndex()];
	}
	else
	{
		QString str1 = "WGS84";
		QString str2 = "CGCS2000";
		QString str = ui->sphere->currentText();
		if (str.compare(str1) == 0)
		{
			EPSG = 4326;
		}
		else
		{
			if (str.compare(str2) == 0)
			{
				EPSG = 4490;
			}
			else
			{
				return;
			}
		}
	}
	QString CPrunPath = QCoreApplication::applicationDirPath();
	CPrunPath.append("/data");
	CPLSetConfigOption("GDAL_DATA", CPrunPath.toLocal8Bit().data());
	OGRSpatialReference*myOSG=new OGRSpatialReference();
	myOSG->importFromEPSG(EPSG);
	char*str;
	myOSG->exportToWkt(&str);
	sendprj(str);
	this->close();
}

void CPGeoInfo::on_type_currentIndexChanged(int index)
{
    ui->type->setCurrentIndex(index);
    if(index==0){
         ui->label_2->setEnabled(false);
         ui->proj->setEnabled(false);
    }
    else {
        ui->label_2->setEnabled(true);
         ui->proj->setEnabled(true);
    }

}

void CPGeoInfo::on_sphere_currentIndexChanged(int index)
{
    ui->proj->clear();
    EPSGs.clear();
    for(int i=0;i<myprjInfo.values()[index].size();i++)
    {
         ui->proj->addItem(myprjInfo.values()[index][i].second);
         EPSGs[i] = myprjInfo.values()[index][i].first;
    }
}

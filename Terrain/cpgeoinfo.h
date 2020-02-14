#ifndef CPGEOINFO_H
#define CPGEOINFO_H

#include <QWidget>
#include<Windows.h>
#include"QMap"
#include"vector"
#include "gdal_priv.h"  
#include "ogrsf_frmts.h" //for ogr  
#include "gdal_alg.h"  //for GDALPolygonize  
#include <io.h>
#include<malloc.h>
#include <string>
#include <vector>
#include <fstream>
#include<iostream>
#include"ogr_core.h"
#include<map>
#include<set>
#include<list>
#include"qmap.h"
#include"qstring.h"
#include "gdalwarper.h"
#include"qmessagebox.h"
#include"QFileDialog"
using namespace std;
namespace Ui {
class CPGeoInfo;
}

class CPGeoInfo : public QWidget
{
    Q_OBJECT

public:
    explicit CPGeoInfo(QWidget *parent = nullptr);
    void getData(QMap<QString,vector<pair<int,QString>>>prjInfo);
    ~CPGeoInfo();

private slots:
    void on_cancel_clicked();

    void on_ok_clicked();

    void on_type_currentIndexChanged(int index);

    void on_sphere_currentIndexChanged(int index);

private:
	//void getRunPathDir(QString &CPrunPath);
	
    Ui::CPGeoInfo *ui;
    QMap<QString,vector<pair<int,QString>>>myprjInfo;
	QMap<int, int>EPSGs;
signals:
    void sendprj(QString);
};

#endif // CPGEOINFO_H

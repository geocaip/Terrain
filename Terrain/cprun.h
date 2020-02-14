#ifndef CPRUN_H
#define CPRUN_H

#include <QWidget>
#include <QThread>
#include"QList"
#include "gdal_priv.h"  
#include "ogrsf_frmts.h" //for ogr  
#include "gdal_alg.h"  //for GDALPolygonize  
#include"vector"
#include <io.h>
#include<malloc.h>
#include <string>
#include <vector>
#include <fstream>
#include<iostream>
#include "ogr_core.h"
#include"cpgeoinfo.h"
using namespace std;
class CPRun : public QThread
{
    Q_OBJECT
public:
    explicit CPRun(QWidget *parent = nullptr);

    void run();//只有run()里面在新的线程里
    void work();
signals:
    void currentProgress();
public slots:
public:
	void CPRun::setData(QString mycmd);
private:
	QString cmd;
};

#endif // CPRUN_H




#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"QFileDialog"
#include"QMessageBox"
#include"QProgressBar"
#include"QElapsedTimer"
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
#include"QTableWidgetItem"
using namespace std;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    //����
    QStringList srcLases;
    QVector<pair<QString,double>> dstImags;
    QStringList dstSpatials;
    //////////////////////////
    QString runPath;
    QMap<QString, vector<pair<int,QString>>>PrjInfo;
    QProgressBar*my_progressbar;
    QLabel*progressInfo;
    int progressIndex;
    QElapsedTimer mstimer;
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
	void process_dem_tsk(QStringList srcPaths, QVector<pair<QString, double>> dstImags, QStringList mySpatials, QStringList&tskPaths);
	
private slots:
	void S_Progress();
	void show_menu(QPoint point);
    void on_Button_Add_clicked();
    void on_Button_Delete_clicked();
    void on_pushButton_clicked();
    void on_Button_Cancel_clicked();
    void on_Button_Ok_clicked();
	void S_Fill_2();
	void S_Fill_4();
	void S_Open_2();
	void S_Open_3();
	void S_Fill_3();
	void S_Set_3();
	void S_setPrj_3(QString str);
private:
    Ui::MainWindow *ui;
private:
	int positionX;
	int positionY;
private:
	QStringList tskPaths;
	QStringList iniPaths;
	QStringList tskPaths2;
	int tskSize;
};

#endif // MAINWINDOW_H

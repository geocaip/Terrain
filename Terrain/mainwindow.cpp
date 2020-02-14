#include "mainwindow.h"
#include "ui_mainwindow.h"
#include"omp.h"
#include"cpgeoinfo.h"
#include"cprun.h"
#include "lasreader.hpp"
#include"QTextCodec"
#ifdef _DEBUG
#pragma comment(lib, "Debug/LASlib.lib")
#else
#pragma comment(lib, "Release/LASlib.lib")
#endif // DEBUG
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	QString runPath = QCoreApplication::applicationDirPath();
	PrjInfo.clear();
	FILE*fp;
	QString iniPath = runPath;
	iniPath.append("/CGT.ini");
	fopen_s(&fp, iniPath.toLocal8Bit().data(), "rt");
	if (!fp)
	{
		QMessageBox::warning(this, "警告", "配置文件读取失败", QMessageBox::Ok, QMessageBox::Ok);
	}
	else
	{
		PrjInfo.clear();
		int Num;
		fscanf_s(fp, "%d", &Num);
		for (int i = 0; i < Num; i++) {
			char ref[256];
			fscanf_s(fp, "%s", ref, 256);
			int myNum;
			fscanf_s(fp, "%d", &myNum);
			vector<pair<int, QString>>tempVector; tempVector.clear();
			for (int j = 0; j < myNum; j++)
			{
				pair<int, QString>tempPair;
				char tempInfo[256];
				fscanf_s(fp, "%d %s", &tempPair.first, tempInfo, 256);
				tempPair.second = tempInfo;
				tempVector.push_back(tempPair);
			}
			PrjInfo[ref] = tempVector;
		}
		fclose(fp);
	}
	ui->tableWidget_Img->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tableWidget_Img, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(show_menu(QPoint)));
	FILE*fq;
	QString iniCore = runPath;
	iniCore.append("/multiCore.ini");
	fopen_s(&fq, iniCore.toLocal8Bit().data(), "rt");
	int coreNum = 0;
	if (!fq)
	{
		QMessageBox::warning(this, "警告", "配置文件读取失败", QMessageBox::Ok, QMessageBox::Ok);
	}
	else
	{
		fscanf_s(fq, "%d", &coreNum);
		fclose(fq);
	}
	int coreMaxNum = omp_get_num_procs();
	if (coreNum == 0)
	{
		coreNum = coreMaxNum;
	}
	if (coreNum == 0)
	{
		coreNum = 4;
	}
	ui->multiCore->clear();
	for (int i = 0; i < coreNum; i++)
	{
		if ((i) == coreNum / 2)
		{
			ui->multiCore->addItem(QString::number(i + 1).append("(推荐)"));
		}
		else
		{
			ui->multiCore->addItem(QString::number(i + 1));
		}

	}
	ui->multiCore->setCurrentIndex(coreNum / 2);
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_Button_Add_clicked()
{
    QString DataType= "las Files(*.las);;LAS Files(*.LAS);;laz Files(*.laz);;LAZ Files(*.LAZ)";
        QStringList filenames = QFileDialog::getOpenFileNames(this, "打开数据", NULL, DataType);
        if (!filenames.isEmpty()) {
            ui->tableWidget_Img->clear();
        }
        else
        {
            QMessageBox::warning(this, "警告", "文件打开失败", QMessageBox::Ok, QMessageBox::Ok);
            return;
        }
        QStringList headerc;
        headerc.clear();
        headerc << "ID"  << "PointCloud Path"<<"Output Raster Path"<<"Output Coordinate System"<<"PixelSize(m)";   //表头
        ui->tableWidget_Img->setColumnCount(headerc.length());
        ui->tableWidget_Img->setHorizontalHeaderLabels(headerc);
        ui->tableWidget_Img->setRowCount(filenames.size());
        QTableWidgetItem *check = new QTableWidgetItem;
        check->setCheckState(Qt::Checked);
        ui->tableWidget_Img->setItem(0, 0, check);
        GDALAllRegister();
        CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
		QTextCodec* coder = QTextCodec::codecForName("GBK");
        for (int i = 0; i < filenames.length(); i++)
        {
            ui->tableWidget_Img->setColumnWidth(0, 25);
            ui->tableWidget_Img->setRowHeight(i, 20);
            ui->tableWidget_Img->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
            ui->tableWidget_Img->setItem(i, 1, new QTableWidgetItem(filenames[i]));
            ui->tableWidget_Img->setItem(i, 2, new QTableWidgetItem(""));
            ui->tableWidget_Img->setItem(i, 3, new QTableWidgetItem(""));
            ui->tableWidget_Img->setItem(i, 4, new QTableWidgetItem(""));
        }
		//由点云的信息得到pixelsize的信息//
		for (int i = 0; i < filenames.size(); i++)
		{
			LASreadOpener lasreadopener;
			lasreadopener.set_file_name(filenames[i].toLocal8Bit().data());
			LASreader* lasreader = lasreadopener.open();
			LASheader myheader = lasreader->header;
			int las_pt_num = myheader.number_of_point_records;
			double xx = sqrt((myheader.max_x - myheader.min_x)*(myheader.max_y - myheader.min_y) / las_pt_num);
			lasreader->close();
			ui->tableWidget_Img->item(i,4)->setText(QString::number(xx, 'f', 6));
		}
        ui->tableWidget_Img->resizeColumnsToContents();
        ui->tableWidget_Img->resizeRowsToContents();
}
void MainWindow::on_Button_Delete_clicked()
{
    QList<QTableWidgetItem*> items;
        items = ui->tableWidget_Img->selectedItems();
        QMap<int, int>myIndex;
        myIndex.clear();
        for (int i = 0; i < items.length(); i++)
        {
            int r = items.at(i)->row();
            myIndex[r]++;
        }
        for (int j = myIndex.size()-1; j >=0; j--)
        {
            ui->tableWidget_Img->removeRow(myIndex.keys()[j] );

        }
        //id重新编号
        for (int i = 0; i < ui->tableWidget_Img->rowCount(); i++)
        {
            ui->tableWidget_Img->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        }
}

void MainWindow::on_pushButton_clicked()
{
    QString Path = QFileDialog::getExistingDirectory(this, "打开保存路径", NULL);
        ui->lineEdit->setText(Path);
}

void MainWindow::on_Button_Cancel_clicked()
{
        this->close();
}
void MainWindow::process_dem_tsk(QStringList srcPaths, QVector<pair<QString, double>> dstImags, QStringList mySpatials,QStringList&tskPaths)
{
	tskPaths.clear();
	if (srcPaths.size() != dstImags.size() || srcPaths.size() == 0)
	{
		return;
	}
	//tsk路径//
	//首先找到dir路径//
	QStringList str = dstImags[0].first.split("/");
	QString dirPath; dirPath.clear();
	for (int i = 0; i < str.size() - 1; i++)
	{
		dirPath.append(str[i]).append("/");
	}
	for (int i = 0; i < srcPaths.size(); i++)
	{
		QString tskPath= dirPath;
		tskPath.append(QString::number(i)).append(".tsk");
		tskPaths.append(tskPath);
		FILE*fp;
		fopen_s(&fp, tskPath.toLocal8Bit().data(), "wt");
		if (!fp)
		{
			continue;
		}
		fprintf_s(fp, "%s\n", srcPaths[i].toLocal8Bit().data());
		fprintf_s(fp, "%s\n", dstImags[i].first.toLocal8Bit().data());
		fprintf_s(fp, "%lf %lf\n", dstImags[i].second, dstImags[i].second);
		fprintf_s(fp, "%d\n", 1);
		fclose(fp);


		QString dem_out = dirPath;
		dem_out.append(QString::number(i)).append(".ini");
		iniPaths.append(dem_out);
		FILE*fq;
		fopen_s(&fq, dem_out.toLocal8Bit().data(), "wt");
		if (!fq)
		{
			return;
		}
		fprintf_s(fq, "%s\n", dstSpatials[i].toLocal8Bit().data());
		fclose(fq);
	}
}
void MainWindow::on_Button_Ok_clicked()
{
	//输入路径，输出路径,输出路径proj4,输出分辨率//
        QString pathDir = ui->lineEdit->text();
        if (pathDir.isEmpty())
        {
            QMessageBox::warning(this, "警告", "输出路径为空", QMessageBox::Ok, QMessageBox::Ok);
            return;
        }
        my_progressbar = new QProgressBar(this);
        my_progressbar->setRange(0, ui->tableWidget_Img->rowCount());
        progressIndex = 0;
        my_progressbar->setValue(progressIndex);
        progressInfo = new QLabel();
        progressInfo->setText("处理...");
        statusBar()->addWidget(progressInfo, 0);
        statusBar()->addWidget(my_progressbar,1);
        srcLases.clear();
        dstImags.clear();
        dstSpatials.clear();
        for(int i=0;i<ui->tableWidget_Img->rowCount();i++)
        {
			QString srcPath = ui->tableWidget_Img->item(i, 1)->text();
            QString dstPath = ui->tableWidget_Img->item(i, 2)->text();
            QString dst_Spatial = ui->tableWidget_Img->item(i,3)->text();
            if (srcPath.isEmpty() || dstPath.isEmpty() || dst_Spatial.isEmpty())
            {
                QString info = "the";
                info.append(QString::number(i)).append("th row lacks infomation");
                QMessageBox::warning(this, "警告", info, QMessageBox::Ok, QMessageBox::Ok);
                my_progressbar->setValue(ui->tableWidget_Img->rowCount());
				statusBar()->removeWidget(progressInfo);
                statusBar()->removeWidget(my_progressbar);
                return;
            }
			srcLases.append(srcPath);
            double pixelsize;
            if (!ui->tableWidget_Img->item(i, 4)->text().isEmpty())
            {
                pixelsize = ui->tableWidget_Img->item(i, 4)->text().toDouble();
                if (pixelsize <= 0)
                {
					QMessageBox::warning(this, "警告", "pixelsize设置错误", QMessageBox::Ok, QMessageBox::Ok);
					my_progressbar->setValue(ui->tableWidget_Img->rowCount());
					statusBar()->removeWidget(progressInfo);
					statusBar()->removeWidget(my_progressbar);
					return;
                }
            }
            else
            {
				QMessageBox::warning(this, "警告", "pixelsize为空", QMessageBox::Ok, QMessageBox::Ok);
				my_progressbar->setValue(ui->tableWidget_Img->rowCount());
				statusBar()->removeWidget(progressInfo);
				statusBar()->removeWidget(my_progressbar);
				return;
            }
            pair<QString, double>tempPair; tempPair.first = dstPath; tempPair.second = pixelsize;
            dstImags.append(tempPair);
            dstSpatials.append(dst_Spatial);
        }
        ui->Button_Ok->setEnabled(false);
        ui->Button_Delete->setEnabled(false);
        ui->Button_Add->setEnabled(false);
        ui->lineEdit->setEnabled(false);
        ui->tableWidget_Img->setEnabled(false);
        ui->pushButton->setEnabled(false);
        ui->multiCore->setEnabled(false);
        ui->type->setEnabled(false);
		ui->method->setEnabled(false);
        int coreNum = ui->multiCore->currentIndex()+1;
        mstimer.start();
		//执行核心程序//
		int method = ui->method->currentIndex()+1;
		QString runPath = QCoreApplication::applicationDirPath();
		iniPaths.clear();
		if (method == 1)
		{
			tskPaths; tskPaths.clear();
			process_dem_tsk(srcLases, dstImags, dstSpatials, tskPaths);
			tskPaths2 = tskPaths;
			tskSize = tskPaths.size();
			//dem_out.ini//
			
		for (int i = 0; i < coreNum; i++)
		{
			if (tskPaths.size() > 0) {
				QString cmd = QCoreApplication::applicationDirPath();
				cmd.append("/dem_knl.exe ").append(tskPaths[0]);
				tskPaths.pop_front();
				CPRun*myrun = new CPRun();
				myrun->setData(cmd);
				connect(myrun, SIGNAL(currentProgress()), this, SLOT(S_Progress()),static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
				myrun->start();
			}
		  }
		}
		else
		{
			if (method == 2)
			{
				tskPaths; tskPaths.clear();
				process_dem_tsk(srcLases, dstImags, dstSpatials, tskPaths);
				tskPaths2 = tskPaths;
				tskSize = tskPaths.size();
				//dem_out.ini//
				
				for (int i = 0; i < coreNum; i++)
				{
					if (tskPaths.size() > 0) {
						QString cmd = runPath;
						cmd.append("/dsm_knl.exe ").append(tskPaths[0]);
						tskPaths.pop_front();
						CPRun*myrun = new CPRun();
						myrun->setData(cmd);
						connect(myrun, SIGNAL(currentProgress()), this, SLOT(S_Progress()),static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
						myrun->start();
					}
				}
			}
		}
}
void MainWindow::S_Progress()
{
	progressIndex++;
	my_progressbar->setValue(progressIndex);
	if (ui->method->currentIndex()+1 == 1)
	{
			if (tskPaths.size() > 0) {
				QString cmd = QCoreApplication::applicationDirPath();
				cmd.append("/dem_knl.exe ").append(tskPaths[0]);
				tskPaths.pop_front();
				CPRun*myrun = new CPRun();
				myrun->setData(cmd);
				connect(myrun, SIGNAL(currentProgress()), this, SLOT(S_Progress()));
				myrun->start();
			}
	}
	else
	{
		if (ui->method->currentIndex() + 1 == 2)
		{
				if (tskPaths.size() > 0) {
					QString cmd = QCoreApplication::applicationDirPath();
					cmd.append("/dsm_knl.exe ").append(tskPaths[0]);
					tskPaths.pop_front();
					CPRun*myrun = new CPRun();
					myrun->setData(cmd);
					connect(myrun, SIGNAL(currentProgress()), this, SLOT(S_Progress()));
					myrun->start();
				}
		}
	}
	if (progressIndex == tskSize)
	{
		//删除掉TempDir中的所有文件，包括文件夹//
		double elapsedTime = (double)mstimer.nsecsElapsed() / (double)1000000000;
		QLabel*	progressTime = new QLabel();
		progressTime->setText(QString::number(elapsedTime, 'f', 3).append("s"));
		statusBar()->addWidget(progressTime);
		//this->close();
		for (int i = 0; i < tskPaths2.size(); i++)
		{
			QFile file(tskPaths2[i]);
			if (file.exists())
			{
				file.remove();
			}
			QFile file2(iniPaths[i]);
			if (file2.exists())
			{
				file2.remove();
			}
		}
	}
}

void MainWindow::show_menu(QPoint point)
{
	QModelIndex index = ui->tableWidget_Img->indexAt(point);
	int row = index.row();
	int col = index.column();
	positionX = col;
	positionY = row;
	if (col < 0 || row < 0)
	{
		return;
	}
	if (col == 2)
	{
		QMenu *menu = new QMenu(ui->tableWidget_Img);
		if (row == 0) {
			QAction *pnew = new QAction("Fill", ui->tableWidget_Img);
			connect(pnew, SIGNAL(triggered()), this, SLOT(S_Fill_2()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
			menu->addAction(pnew);
		}
		QAction *pnew1 = new QAction("Open", ui->tableWidget_Img);
		connect(pnew1, SIGNAL(triggered()), this, SLOT(S_Open_2()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
		menu->addAction(pnew1);//就是说只有显示的las才能改变颜色信息
		menu->move(cursor().pos());
		menu->show();
	}
	else
	{
		if (col == 3)
		{
			if (row == 0) {
				QMenu *menu = new QMenu(ui->tableWidget_Img);
				if (row == 0) {
					QAction *pnew = new QAction("Fill", ui->tableWidget_Img);
					connect(pnew, SIGNAL(triggered()), this, SLOT(S_Fill_3()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
					menu->addAction(pnew);
				}
				QAction *pnew1 = new QAction("Config", ui->tableWidget_Img);
				connect(pnew1, SIGNAL(triggered()), this, SLOT(S_Set_3()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
				menu->addAction(pnew1);//就是说只有显示的las才能改变颜色信息
				QAction *pnew2 = new QAction("Reference Img", ui->tableWidget_Img);
				connect(pnew2, SIGNAL(triggered()), this, SLOT(S_Open_3()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
				menu->addAction(pnew2);//就是说只有显示的las才能改变颜色信息
				menu->move(cursor().pos());
				menu->show();
			}
			else
			{
				QMenu *menu = new QMenu(ui->tableWidget_Img);
				QAction *pnew1 = new QAction("Config", ui->tableWidget_Img);
				connect(pnew1, SIGNAL(triggered()), this, SLOT(S_Set_3()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
				menu->addAction(pnew1);//就是说只有显示的las才能改变颜色信息
				QAction *pnew2 = new QAction("Reference Img", ui->tableWidget_Img);
				connect(pnew2, SIGNAL(triggered()), this, SLOT(S_Open_3()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
				menu->addAction(pnew2);//就是说只有显示的las才能改变颜色信息
				menu->move(cursor().pos());
				menu->show();
			}
		}
		else
		{
			if (col == 4)
			{
				if (row == 0) {
					QMenu *menu = new QMenu(ui->tableWidget_Img);
					QAction *pnew = new QAction("Fill", ui->tableWidget_Img);
					connect(pnew, SIGNAL(triggered()), this, SLOT(S_Fill_4()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
					menu->addAction(pnew);
					menu->move(cursor().pos());
					menu->show();//以后可以设置只为数字
				}
			}
			else
			{
			}
		}

	}
}

void MainWindow::S_Fill_2()
{
	QString pathDir = ui->lineEdit->text();
	int type = ui->type->currentIndex();
	if (!pathDir.isEmpty())
	{
		for (int i = 0; i < ui->tableWidget_Img->rowCount(); i++) {
			QString path_in = ui->tableWidget_Img->item(i, 1)->text();
			QStringList templist = path_in.split("/");
			QString name = templist[templist.size() - 1];
			QStringList nameList = name.split(".");
			QString name1; name1.clear();
			for (int i = 0; i < nameList.size() - 1; i++)
			{
				name1.append(nameList[i]).append(".");
			}
			QString path_out = pathDir;
			if (type == 0)
			{
				path_out.append("/").append(name1).append("tif");
			}
			else
			{
				path_out.append("/").append(name1).append("img");
			}
			ui->tableWidget_Img->setItem(i, 2, new QTableWidgetItem(path_out));
		}
		ui->tableWidget_Img->resizeColumnsToContents();
		ui->tableWidget_Img->resizeRowsToContents();
	}
	else
	{
		QMessageBox::information(this, "提示", "please input pathdir first", QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
}
void MainWindow::S_Fill_4()
{
	QString str = ui->tableWidget_Img->item(0, 4)->text();
	if (!str.isEmpty())
	{
		for (int i = 0; i < ui->tableWidget_Img->rowCount(); i++) {
			ui->tableWidget_Img->setItem(i, 4, new QTableWidgetItem(str));
			//ui->tableWidget_Img->item(i, 5)->setText(str);
		}
		ui->tableWidget_Img->resizeColumnsToContents();
		ui->tableWidget_Img->resizeRowsToContents();
	}
	else
	{
		QMessageBox::information(this, "提示", "please input the first row's  resolution", QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
}
void MainWindow::S_Open_2()
{
	QString path = QFileDialog::getOpenFileName(this, "open", NULL, "tif Files(*.tif);; TIF Files(*.TIF);; tiff Files(*.tiff);; TIFF Files(*.TIFF);; img Files(*.img);; IMG Files(*.IMG)");
	//ui->tableWidget_Img->item(positionY, 3)->setText(path);
	ui->tableWidget_Img->setItem(positionY, 2, new QTableWidgetItem(path));
	ui->tableWidget_Img->resizeColumnsToContents();
	ui->tableWidget_Img->resizeRowsToContents();
}
void MainWindow::S_setPrj_3(QString str)
{
	ui->tableWidget_Img->setItem(positionY, 3, new QTableWidgetItem(str));
	ui->tableWidget_Img->resizeColumnsToContents();
	ui->tableWidget_Img->resizeRowsToContents();
}
void  MainWindow::S_Open_3()
{
	QString path = QFileDialog::getOpenFileName(this, "open", NULL, "tif Files(*.tif);; TIF Files(*.TIF);; tiff Files(*.tiff);; TIFF Files(*.TIFF);; img Files(*.img);; IMG Files(*.IMG)");
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	GDALDataset*m_pDataset = (GDALDataset*)GDALOpen(path.toLocal8Bit().data(), GA_ReadOnly);
	if (m_pDataset == NULL)
	{
		printf("cannot open the file %s !", path.toLocal8Bit().data());
		return;
	}
	ui->tableWidget_Img->setItem(positionY, 3, new QTableWidgetItem(m_pDataset->GetProjectionRef()));
	GDALClose(m_pDataset);
	ui->tableWidget_Img->resizeColumnsToContents();
	ui->tableWidget_Img->resizeRowsToContents();
}
void  MainWindow::S_Fill_3()
{
	QString str = ui->tableWidget_Img->item(0, 3)->text();
	if (!str.isEmpty())
	{
		for (int i = 0; i < ui->tableWidget_Img->rowCount(); i++) {
			ui->tableWidget_Img->setItem(i, 3, new QTableWidgetItem(str));
			//ui->tableWidget_Img->item(i, 4)->setText(str);//wkt
		}
		ui->tableWidget_Img->resizeColumnsToContents();
		ui->tableWidget_Img->resizeRowsToContents();
	}
	else
	{
		QMessageBox::information(this, "提示", "please set the first row's reference coordinate system", QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
}
void  MainWindow::S_Set_3()
{
	CPGeoInfo*myinfo = new CPGeoInfo();
	myinfo->getData(PrjInfo);
	connect(myinfo, SIGNAL(sendprj(QString)), this, SLOT(S_setPrj_3(QString)));

	myinfo->setWindowModality(Qt::ApplicationModal);
	myinfo->show();
}

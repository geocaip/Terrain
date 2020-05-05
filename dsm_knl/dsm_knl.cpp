//2091208改进，将dsm_out.ini改进到tsk同目录下的文件//

#include<malloc.h>
#include<time.h>
#include "gdal_priv.h"  
#include "ogrsf_frmts.h" //for ogr  
#include "gdal_alg.h"  //for GDALPolygonize  
#include <io.h>
#include"iostream"
#include"vector"
#include "lasreader.hpp"
#include"laswriter.hpp"
using namespace std;
#define PATH_SIZE 256
#include"opencv2/flann/miniflann.hpp"
using namespace cv;
struct myRegon {
	double xmin;
	double xmax;
	double ymin;
	double ymax;
};
struct point3D {
	double X;
	double Y;
	double Z;
	int classification;
};
struct CloudHeader
{
	double x_offset;
	double y_offset;
	double z_offset;
	double x_scale_factor;
	double y_scale_factor;
	double z_scale_factor;
	double max_y;
	double min_y;
	double max_x;
	double min_x;
	double max_z;
	double min_z;
};
bool InitialProc(point3D*points, int num, CloudHeader mylasHdr, vector<cv::Point2f>&points_after, vector<float>&points_z)
{
	//先将所有点分到格网中;
	//一个格网大概有4-5个点;
	vector<vector<int>>gridPoints;
	int*gridNum;
	if (num < 10)
	{
		return false;
	}
	double scale = (mylasHdr.max_y - mylasHdr.min_y) / (mylasHdr.max_x - mylasHdr.min_x);
	int x_gridNum = sqrt(num / (5 * scale));
	int y_gridNum = (int)x_gridNum*scale;
	if (x_gridNum <= 1 || y_gridNum <= 1)
	{
		return false;
	}
	double xscale = (mylasHdr.max_x - mylasHdr.min_x) / x_gridNum;
	double yscale = (mylasHdr.max_y - mylasHdr.min_y) / y_gridNum;
	gridNum = new int[num];
	for (int i = 0; i < num; i++)
	{
		point3D localPoint;
		double localPointx = points[i].X*mylasHdr.x_scale_factor+ mylasHdr.x_offset;
		double localPointy = points[i].Y*mylasHdr.y_scale_factor + mylasHdr.y_offset;
		double localPointz = points[i].Z*mylasHdr.z_scale_factor + mylasHdr.z_offset;
		int xnum = ceil((localPointx - mylasHdr.min_x) / xscale);
		if (xnum >= x_gridNum)
		{
			xnum = x_gridNum - 1;
		}
		int ynum = ceil((localPointy - mylasHdr.min_y) / yscale);
		if (ynum >= y_gridNum)
		{
			ynum = y_gridNum - 1;
		}
		gridNum[i] = xnum + ynum * x_gridNum;
	}
	gridPoints.clear();
	vector<int>tempVector;
	tempVector.clear();
	for (int i = 0; i < y_gridNum; i++)
		for (int j = 0; j < x_gridNum; j++)
		{
			gridPoints.push_back(tempVector);
		}
	for (int i = 0; i < num; i++)
	{
		gridPoints[gridNum[i]].push_back(i);
	}
	//处理
	vector<int>residualPoints;
	residualPoints.clear();
	for (int i = 0; i < y_gridNum; i++)
		for (int j = 0; j < x_gridNum; j++)
		{
			int GridPointNum = gridPoints[i*x_gridNum + j].size();
			for (int ii = 0; ii < GridPointNum - 1; ii++)
				for (int jj = ii + 1; jj < GridPointNum; jj++)
				{
					if (points[gridPoints[i*x_gridNum + j][ii]].X == points[gridPoints[i*x_gridNum + j][jj]].X&&points[gridPoints[i*x_gridNum + j][ii]].Y == points[gridPoints[i*x_gridNum + j][jj]].Y)
					{
						if (points[gridPoints[i*x_gridNum + j][ii]].Z > points[gridPoints[i*x_gridNum + j][jj]].Z)
						{
							gridPoints[i*x_gridNum + j][jj] = 0;
						}
						else
						{
							gridPoints[i*x_gridNum + j][ii] = gridPoints[i*x_gridNum + j][jj];
							gridPoints[i*x_gridNum + j][jj] = 0;
						}
					}
				}
			for (int ii = 0; ii < GridPointNum; ii++)
			{
				if (gridPoints[i*x_gridNum + j][ii] != 0 && points[gridPoints[i*x_gridNum + j][ii]].classification !=7)
				{
					residualPoints.push_back(gridPoints[i*x_gridNum + j][ii]);
				}
			}
		}
	gridPoints.clear();
	delete[]gridNum;
	points_after.clear();
	for (int i = 0; i < residualPoints.size(); i++)
	{
		cv::Point2f tempPoint2f;
		tempPoint2f.x=points[residualPoints[i]].X*mylasHdr.x_scale_factor + mylasHdr.x_offset;
		tempPoint2f.y = points[residualPoints[i]].Y*mylasHdr.y_scale_factor + mylasHdr.y_offset;
		points_z.push_back(points[residualPoints[i]].Z*mylasHdr.z_scale_factor + mylasHdr.z_offset);
		points_after.push_back(tempPoint2f);
	}
}
//格网法//
bool getRunDir(string&runPathDir)
{
	char runPath[256];//（D:\Documents\Downloads\TEST.exe）
	memset(runPath, 0, 256);
	GetModuleFileNameA(NULL, runPath, 256);
	string myRunPath = runPath;
	int pos = myRunPath.find("/");////查找指定的串
	while (pos != -1)
	{
		//printf("%d\n", pos);
		myRunPath.replace(pos, 1, "/");////用新的串替换掉指定的串

		pos = myRunPath.find("/");/////继续查找指定的串，直到所有的都找到为止
	}
	sprintf_s(runPath, "%s", myRunPath.c_str());
	strcpy_s(strrchr(runPath, '/'), 1, "");
	runPathDir = runPath;
	return true;
}
bool getlasPoints(const char*path, point3D*&points,int&las_pt_num, CloudHeader&myheader)
{
	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(path);
	LASreader* lasreader = lasreadopener.open();
	LASheader tempheader = lasreader->header;
	myheader.x_offset = tempheader.x_offset;
	myheader.y_offset = tempheader.y_offset;
	myheader.z_offset = tempheader.z_offset;
	myheader.x_scale_factor = tempheader.x_scale_factor;
	myheader.y_scale_factor = tempheader.y_scale_factor;
	myheader.z_scale_factor = tempheader.z_scale_factor;

	myheader.min_x = tempheader.min_x;
	myheader.min_y = tempheader.min_y;
	myheader.min_z = tempheader.min_z;
	myheader.max_x = tempheader.max_x;
	myheader.max_y = tempheader.max_y;
	myheader.max_z = tempheader.max_z;
	if (tempheader.number_of_point_records < 20)
	{
		return false;
	}
	las_pt_num = tempheader.number_of_point_records;
	points = new point3D[tempheader.number_of_point_records];
	LASpoint pt;
	pt.init(&tempheader, tempheader.point_data_format, tempheader.point_data_record_length, 0);

	int readNum = 0;
	while (lasreader->read_point())
	{
		pt = lasreader->point;
		points[readNum].X = pt.get_X();
		points[readNum].Y = pt.get_Y();
		points[readNum].Z = pt.get_Z();
		points[readNum].classification = pt.classification;
		readNum++;
	}
	lasreader->close();
}
bool DSM_sin(const char*path, myRegon blockRegion, double xSize, double ySize, float*&DEMData)
{
	point3D* points; int las_pt_num; CloudHeader myheader;
	getlasPoints(path,points, las_pt_num, myheader);
	
	vector<cv::Point2f>points_after; points_after.clear();
	vector<float>points_z; points_z.clear();
	InitialProc(points, las_pt_num, myheader, points_after, points_z);
	if (points_after.size() < 20)
	{
		return false;
	}
	delete[]points;
	//构建KD树//
	cv::Mat source = cv::Mat(points_after).reshape(1);
	cv::flann::KDTreeIndexParams indexParams(2);
	cv::flann::Index kdtree(source, indexParams);

	double xresolution = xSize;
	double yresolution = -1 * ySize;

	double adsTransform[6];
	adsTransform[0] = blockRegion.xmin;
	adsTransform[3] = blockRegion.ymax;
	adsTransform[2] = 0;
	adsTransform[4] = 0;
	adsTransform[1] = xresolution;
	adsTransform[5] = yresolution;
	int m_nwidth = (blockRegion.xmax - blockRegion.xmin) / adsTransform[1];
	int m_nheight = (blockRegion.ymin - blockRegion.ymax) / adsTransform[5];
	DEMData = new float[m_nwidth*m_nheight];
	for (int i = 0; i < m_nwidth*m_nheight; i++)
	{
		DEMData[i] = -9999;
	}
	//开始内插//
	float totalWeight = 0.0;
	for (int i = 0; i < m_nheight; i++) 
	for(int j=0;j< m_nwidth;j++){
		DEMData[i*m_nwidth + j] = 0;
		int queryNum = 6;//用于设置返回邻近点的个数
		vector<float> vecQuery(2);//存放查询点的容器
		vector<int> vecIndex(queryNum);//存放返回的点索引
		vector<float> vecDist(queryNum);//存放距离
		cv::flann::SearchParams params(32);
		vecQuery = { (float)(blockRegion.xmin+j* xresolution),(float)(blockRegion.ymax + i * yresolution) };
		kdtree.knnSearch(vecQuery, vecIndex, vecDist, queryNum, params);
		totalWeight = 0.0;
		for (int k = 0; k < queryNum; k++)
		{
			DEMData[i*m_nwidth + j] += (1 / (vecDist[k]+0.000001))*points_z[vecIndex[k]];
			totalWeight += (1 / (vecDist[k] + 0.000001));
		}
		DEMData[i*m_nwidth + j] /= totalWeight;
	}
	points_after.clear();
	points_z.clear();
	return true;
}
void Pretreatment_seg(const char*srcPath, string &dstPath)
{
	string myRunPath = srcPath;
	int pos = myRunPath.find("\\");////查找指定的串
	while (pos != -1)
	{
		printf("%d\n", pos);
		myRunPath.replace(pos, 1, "/");////用新的串替换掉指定的串

		pos = myRunPath.find("\\");/////继续查找指定的串，直到所有的都找到为止
	}
	char dstPath1[PATH_SIZE];
	sprintf_s(dstPath1, "%s", myRunPath.c_str());
	dstPath.clear();
	dstPath.append(dstPath1);
}
bool IncludeCh(char str[], char ch)
{
	int i;
	bool has = false;
	for (i = 0; str[i]; ++i) {
		if (str[i] == ch)
			return true;
	}
	return false;
}
bool ReadSubFold(const char* FoldPath, vector<string>& SubFoldArray)
{
	if (FoldPath == NULL) return false;
	int fileNum = 0, dirNum = 0;

	char strPath[PATH_SIZE];
	strcpy(strPath, FoldPath);
	strcat(strPath, "/*.*");

	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(strPath, &fd);

	if (hFind != INVALID_HANDLE_VALUE) {
		while (FindNextFile(hFind, &fd)) {
			if (!strcmp(fd.cFileName, "..") || !strcmp(fd.cFileName, ".")) continue;
			if ((fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
				dirNum++;
				char _SubFoldName[PATH_SIZE];
				strcpy(_SubFoldName, FoldPath);
				strcat(_SubFoldName, "/");
				strcat(_SubFoldName, fd.cFileName);
				string SubFoldName = _SubFoldName;
				SubFoldArray.push_back(SubFoldName);
			}
			else fileNum++;
		}
	}
	::FindClose(hFind);
	return ((dirNum > 0) ? true : false);
}
bool ReadFile(const char* FoldPath, vector<string>& inFiles, const char* FileType)
{
	char _FoldPath[PATH_SIZE];
	strcpy(_FoldPath, FoldPath);
	strcat(_FoldPath, "/*");
	WIN32_FIND_DATA FindFileData;
	HANDLE hfile;
	string strFileName;
	string strFileExt;
	char _strFileName[PATH_SIZE];
	char _strFileExt[PATH_SIZE];
	char fn[PATH_SIZE];
	strcpy(fn, _FoldPath);
	hfile = FindFirstFile(fn, &FindFileData);
	if (hfile == NULL) return false;
	FindNextFile(hfile, &FindFileData);
	while (FindNextFile(hfile, &FindFileData)) {
		strcpy(_strFileName, FindFileData.cFileName);
		if (IncludeCh(_strFileName, '.')) {
			char _strTemp[PATH_SIZE];
			strcpy(_strTemp, FoldPath);
			strcat(_strTemp, "/");
			strcat(_strTemp, _strFileName);
			strFileName = _strTemp;
			strcpy(_strFileExt, _strFileName);
			strcpy(_strFileExt, strrchr(_strFileExt, '.'));
			strFileExt = _strFileExt;
			if (strFileExt == FileType)
				inFiles.push_back(strFileName);
		}
	}

	vector<string> SubFoldArray;
	if (ReadSubFold(FoldPath, SubFoldArray)) {
		for (int i = 0; i < SubFoldArray.size(); i++) {
			char SubFold[PATH_SIZE];
			strcpy(SubFold, SubFoldArray[i].c_str());
			if (!ReadFile(SubFold, inFiles, FileType)) {
				cout << "ReadImgFile failed." << endl;
				return false;
			}
		}
	}
	return true;
}
bool ReadFileAll(const char*FoldPath, vector<string>&inFiles)
{
	char _FoldPath[PATH_SIZE];
	strcpy(_FoldPath, FoldPath);
	strcat(_FoldPath, "/*");
	WIN32_FIND_DATA FindFileData;
	HANDLE hfile;
	string strFileName;
	string strFileExt;
	char _strFileName[PATH_SIZE];
	char _strFileExt[PATH_SIZE];
	char fn[PATH_SIZE];
	strcpy(fn, _FoldPath);
	hfile = FindFirstFile(fn, &FindFileData);
	if (hfile == NULL) return false;
	FindNextFile(hfile, &FindFileData);
	while (FindNextFile(hfile, &FindFileData)) {
		strcpy(_strFileName, FindFileData.cFileName);
		if (IncludeCh(_strFileName, '.')) {
			char _strTemp[PATH_SIZE];
			strcpy(_strTemp, FoldPath);
			strcat(_strTemp, "/");
			strcat(_strTemp, _strFileName);
			strFileName = _strTemp;
			strcpy(_strFileExt, _strFileName);
			strcpy(_strFileExt, strrchr(_strFileExt, '.'));
			strFileExt = _strFileExt;
			inFiles.push_back(strFileName);
		}
	}

	return true;
}
bool deleteDocDir(const char*pathDir)
{
	string myRunPath = pathDir;
	int pos = myRunPath.find("\\");////查找指定的串
	while (pos != -1)
	{
		myRunPath.replace(pos, 1, "/");////用新的串替换掉指定的串
		pos = myRunPath.find("\\");/////继续查找指定的串，直到所有的都找到为止
	}
	char dstPath[PATH_SIZE];
	sprintf_s(dstPath, "%s", myRunPath.c_str());
	vector<string>inFiles_all; inFiles_all.clear();
	ReadFileAll(dstPath, inFiles_all);
	for (int i = 0; i < inFiles_all.size(); i++)
	{
		remove(inFiles_all[i].c_str());
	}
	RemoveDirectory(dstPath);
	return true;
}
void GetImgType(const char*path_Img,int&type)
{
	string temp = path_Img;
	if (temp[temp.size() - 1] == 'f' || temp[temp.size() - 1] == 'F')
	{
		type = 1;
	}
	else
	{
		if (temp[temp.size() - 1] == 'g' || temp[temp.size() - 1] == 'G')
		{
			type = 2;
		}
	}
}
int main(int argc, char**argv)
{
	if (argc == 2)
	{
		char TempPath[PATH_SIZE];
		strcpy_s(TempPath, strlen(argv[1]) + 1, argv[1]);
		strcpy_s(strrchr(TempPath, '.'), 1, "");

		FILE*fp;
		fopen_s(&fp, argv[1], "rt");
		char path[256];
		fscanf_s(fp, "%s", path, 256);
		char path_Img[256];
		fscanf_s(fp, "%s", path_Img, 256);
		double xResolution, yResolution;
		int method;
		fscanf_s(fp, "%lf %lf", &xResolution, &yResolution);
		fscanf_s(fp, "%d", &method);
		fclose(fp);
		//分块//
		LASreadOpener lasreadopener_a;
		lasreadopener_a.set_file_name(path);
		LASreader* lasreader_a = lasreadopener_a.open();
		LASheader myheader_a = lasreader_a->header;
		double Xmin = myheader_a.min_x;
		double Xmax = myheader_a.max_x;
		double Ymin = myheader_a.min_y;
		double Ymax = myheader_a.max_y;
		int las_pt_num = myheader_a.number_of_point_records;
		lasreader_a->close();
		vector<string>lasPaths; lasPaths.clear();
		vector<string>borderPaths; borderPaths.clear();
		int AllBlockNum = ceil(1.0*las_pt_num / 10000000);
		char name[PATH_SIZE];
		char name2[PATH_SIZE];
		if (AllBlockNum > 1)
		{
			string tempRunPath;
			Pretreatment_seg(argv[0], tempRunPath);
			char runPath[PATH_SIZE];
			strcpy_s(runPath, strlen(tempRunPath.c_str()) + 1, tempRunPath.c_str());
			strcpy_s(strrchr(runPath, '/'), 1, "");
			char pszDst[PATH_SIZE];
			strcpy_s(pszDst, strlen(path_Img) + 1, path_Img);
			strcpy_s(strrchr(pszDst, '/'), 1, "");
			char cmd[2000];//此处改大一些//
			char outPath[PATH_SIZE];
			
			strcpy_s(name, strlen(path_Img) + 1, path_Img);
			strcpy_s(strrchr(name, '.'), 1, "");
			
			sprintf_s(name2, "%s%s", name, "_b");
			sprintf_s(cmd, "%s%s%s%s%s%s%s", runPath, "/Points_Block.exe ", path," ", name," ", name2);
			system(cmd);
			ReadFile(name,lasPaths, ".las");
			ReadFile(name2, borderPaths, ".las");
		}
		else
		{
			lasPaths.push_back(path);
		}
		//生成tif；
		int m_nwidth = (Xmax - Xmin) / xResolution;
		int m_nheight = (Ymax - Ymin) / yResolution;
		GDALAllRegister();
		CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
		GDALDriver*poDriver; 
		int type = 1;
		GetImgType(path_Img, type);
		if(type==1) {
		poDriver = (GDALDriver*)GDALGetDriverByName("GTiff");
		}
		else
		{
			if (type ==2) {
				poDriver = (GDALDriver*)GDALGetDriverByName("HFA");
			}
		}
		GDALDataset*pDstDS = poDriver->Create(path_Img, m_nwidth, m_nheight, 1, GDT_Float32, NULL);
		if (pDstDS == NULL)
		{
			printf("cannot open the file  !");
			return 0;
		}
		double adsTransform[6]; adsTransform[0] = Xmin; adsTransform[1] = xResolution;
		adsTransform[2] = 0; adsTransform[3] = Ymax; adsTransform[4] = 0; adsTransform[5] = -1 * yResolution;
		pDstDS->SetGeoTransform(adsTransform);
		pDstDS->GetRasterBand(1)->SetNoDataValue(-9999);
	
		char coordinate_Path[PATH_SIZE];
		sprintf_s(coordinate_Path, "%s%s", TempPath, ".ini");
		FILE*fq;
		fopen_s(&fq, coordinate_Path, "rt");
		if (!fq)
		{
			return 0;//写入失败//
		}
		char localProj4[5000];
		fgets(localProj4,5000,fq);
		fclose(fq);
		pDstDS->SetProjection(localProj4);
		//关闭
		GDALClose(pDstDS);
		GDALDataset*poDataset_out = (GDALDataset*)GDALOpen(path_Img, GA_Update);
		if (poDataset_out == NULL)
		{
			printf("cannot open the file %s !", path_Img);
			return 0;
		}
#pragma omp parallel for num_threads(6)  
		for(int i=0;i<lasPaths.size();i++){
			printf("%d\n", i);
				float*DEMData;
				myRegon blockRegion;
				LASreadOpener lasreadopener;
				lasreadopener.set_file_name(lasPaths[i].c_str());
				LASreader* lasreader = lasreadopener.open();
				LASheader myheader = lasreader->header;
				lasreader->close();
				blockRegion.xmin = myheader.min_x;
				blockRegion.xmax = myheader.max_x;
				blockRegion.ymin = myheader.min_y;
				blockRegion.ymax = myheader.max_y;
				int startX = (blockRegion.xmin - Xmin) / xResolution;
				int startY = (blockRegion.ymax - Ymax) / (-1 * yResolution);
				int myWidth = (blockRegion.xmax - blockRegion.xmin) / xResolution;
				int myHeight = (blockRegion.ymax - blockRegion.ymin) / yResolution;
				if (myWidth < 3 || myHeight < 3)
				{
					continue;
				}
				bool mark = DSM_sin(lasPaths[i].c_str(), blockRegion, xResolution, yResolution, DEMData);
				if (!mark)
				{
					continue;
				}
				float*DEMData_temp=new float[myWidth*myHeight];
				poDataset_out->RasterIO(GF_Read, startX, startY, myWidth, myHeight, DEMData_temp, myWidth, myHeight, GDT_Float32, 1, 0, 0, 0, 0);

				for(int i=0;i<myHeight;i++)
					for (int j = 0; j < myWidth; j++)
					{
						if (DEMData[i * myWidth + j] == -9999&& DEMData_temp[i * myWidth + j] != -9999)
						{
							DEMData[i * myWidth + j] = DEMData_temp[i * myWidth + j];
						}
					}
				delete[]DEMData_temp;
				poDataset_out->RasterIO(GF_Write, startX, startY, myWidth, myHeight, DEMData, myWidth, myHeight, GDT_Float32, 1, 0, 0, 0, 0);
				delete[]DEMData;
			}
#pragma omp parallel for num_threads(6)  
		for (int i = 0; i < borderPaths.size(); i++) {
			printf("%d\n", i);
			float*DEMData;
			myRegon blockRegion;
			LASreadOpener lasreadopener;
			lasreadopener.set_file_name(borderPaths[i].c_str());
			LASreader* lasreader = lasreadopener.open();
			LASheader myheader = lasreader->header;
			lasreader->close();
			blockRegion.xmin = myheader.min_x;
			blockRegion.xmax = myheader.max_x;
			blockRegion.ymin = myheader.min_y;
			blockRegion.ymax = myheader.max_y;
			int startX = (blockRegion.xmin - Xmin) / xResolution;
			int startY = (blockRegion.ymax - Ymax) / (-1 * yResolution);
			int myWidth = (blockRegion.xmax - blockRegion.xmin) / xResolution;
			int myHeight = (blockRegion.ymax - blockRegion.ymin) / yResolution;
			if (myWidth < 3 || myHeight < 3)
			{
				continue;
			}
			bool mark = DSM_sin(borderPaths[i].c_str(), blockRegion, xResolution, yResolution, DEMData);
			if (!mark)
			{
				continue;
			}
			float*DEMData_temp = new float[myWidth*myHeight];
			poDataset_out->RasterIO(GF_Read, startX, startY, myWidth, myHeight, DEMData_temp, myWidth, myHeight, GDT_Float32, 1, 0, 0, 0, 0);

			
			//对比检查//
			for (int i = 0; i < myHeight; i++)
				for (int j = 0; j < myWidth; j++)
				{
					if (DEMData[i * myWidth + j] == -9999 && DEMData_temp[i * myWidth + j] != -9999)
					{
						DEMData[i * myWidth + j] = DEMData_temp[i * myWidth + j];
					}
				}
			delete[]DEMData_temp;
			poDataset_out->RasterIO(GF_Write, startX, startY, myWidth, myHeight, DEMData, myWidth, myHeight, GDT_Float32, 1, 0, 0, 0, 0);
			delete[]DEMData;
		}
		GDALClose(poDataset_out);
		if (AllBlockNum > 1)
		{
			deleteDocDir(name);
			deleteDocDir(name2);
		}
	}
	return 0;
}
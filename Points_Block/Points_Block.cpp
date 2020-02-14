//created in 20191220//用于点云分块，同时分块的点云具有重叠区域//不区分blockPoint和borderPoint//不需要border//
#include<map>
#include<set>
#include<vector>
#include<list>
#include"iostream"
#include <io.h>
#include<time.h>
#include "direct.h"
#include"omp.h"
#include "lasreader.hpp"
#include"laswriter.hpp"
using namespace std;
#define PATH_SIZE 256
//#ifdef _DEBUG
//#pragma comment(lib, "Debug/LASlib.lib")
//#else
//#pragma comment(lib, "Release/LASlib.lib")
//#endif // DEBUG
struct myblockInfo
{
	LASheader myheader;
	int X_Block;
	int Y_Block;
	double X_OverLap;
	double Y_OverLap;
};
void Points_Statistics(const char*srcPath, int*BlockPointNum, int X_Block, int Y_Block, double X_length, double Y_length,double X_OverLap,double Y_OverLap)
{
	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(srcPath);
	LASreader* lasreader = lasreadopener.open();
	LASheader myheader = lasreader->header;
	long long las_pt_sum = myheader.number_of_point_records;
	while (lasreader->read_point())
	{
		double x = lasreader->point.get_x();
		double y = lasreader->point.get_y();
		int colX = (x - myheader.min_x) / X_length;
		int rowY = (y - myheader.min_y) / Y_length;
		BlockPointNum[colX + rowY * X_Block]++;
		double localCentreX = myheader.min_x + colX * X_length + X_length / 2;
		double localCentreY = myheader.min_y + rowY * Y_length + Y_length / 2;
		if (fabs(x - localCentreX) > (X_length / 2 - X_OverLap) || fabs(y - localCentreY) > (Y_length / 2 - Y_OverLap))
		{
			if (rowY < Y_Block - 1 && y - localCentreY >(Y_length / 2 - Y_OverLap) && fabs(x - localCentreX) <= (X_length / 2 - X_OverLap))
			{
				BlockPointNum[colX + (rowY + 1)* X_Block]++;
			}//0
			else {
				if (rowY > 0 && y - localCentreY < -(Y_length / 2 - Y_OverLap) && fabs(x - localCentreX) <= (X_length / 2 - X_OverLap))
				{
					BlockPointNum[colX + (rowY - 1)* X_Block]++;
				}//4
				else
				{
					if (colX > 0 && fabs(y - localCentreY) <= (Y_length / 2 - Y_OverLap) && x - localCentreX < -(X_length / 2 - X_OverLap))
					{
						BlockPointNum[colX - 1 + (rowY)* X_Block]++;
					}//6
					else
					{
						if (colX< X_Block - 1 && fabs(y - localCentreY) <= (Y_length / 2 - Y_OverLap) && x - localCentreX >(X_length / 2 - X_OverLap))
						{
							BlockPointNum[colX + 1 + (rowY)* X_Block]++;
						}//2
						else
						{
							if (x - localCentreX > (X_length / 2 - X_OverLap) && y - localCentreY < -(Y_length / 2 - Y_OverLap))
							{
								if (colX < X_Block - 1)
								{
									BlockPointNum[colX + 1 + (rowY)* X_Block]++;
									if (rowY > 0)
									{
										BlockPointNum[colX + 1 + (rowY - 1)* X_Block]++;
									}
								}
								if (rowY > 0) {
									BlockPointNum[colX + (rowY - 1)* X_Block]++;
								}
							}//3
							else
							{
								if (x - localCentreX > (X_length / 2 - X_OverLap) && y - localCentreY > (Y_length / 2 - Y_OverLap))
								{
									if (colX < X_Block - 1)
									{
										BlockPointNum[colX + 1 + (rowY)* X_Block]++;
										if (rowY < Y_Block - 1)
										{
											BlockPointNum[colX + 1 + (rowY + 1)* X_Block]++;
										}
									}
									if (rowY < Y_Block - 1) {
										BlockPointNum[colX + (rowY + 1)* X_Block]++;
									}
								}//1
								else
								{
									if (x - localCentreX <-(X_length / 2 - X_OverLap) && y - localCentreY >(Y_length / 2 - Y_OverLap))
									{
										if (colX > 0)
										{
											BlockPointNum[colX - 1 + (rowY)* X_Block]++;
											if (rowY < Y_Block - 1)
											{
												BlockPointNum[colX - 1 + (rowY + 1)* X_Block]++;
											}
										}
										if (rowY < Y_Block - 1) {
											BlockPointNum[colX + (rowY + 1)* X_Block]++;
										}
									}//7
									else
									{
										if (x - localCentreX < -(X_length / 2 - X_OverLap) && y - localCentreY < -(Y_length / 2 - Y_OverLap))
										{
											if (colX > 0)
											{
												BlockPointNum[colX - 1 + (rowY)* X_Block]++;
												if (rowY > 0)
												{
													BlockPointNum[colX - 1 + (rowY - 1)* X_Block]++;
												}
											}
											if (rowY > 0) {
												BlockPointNum[colX + (rowY - 1)* X_Block]++;
											}
										}//5
									}//else
								}//else
							}//else
						}//else
					}//else
				}//else
			}//else
		}
	}
	lasreader->close();
}
void Points_Block(const char*srcPath, const char*dstPathDir, int X_Block, int Y_Block,double X_OverLap,double Y_OverLap, vector<string>&blockPaths, int*&blockIndex, int*&blockNum, LASheader&lasHdr)
{
	blockIndex = new int[X_Block*Y_Block];
	blockPaths.clear();
	blockNum = new int[X_Block*Y_Block];
	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(srcPath);
	LASreader* lasreader = lasreadopener.open();
	long long las_pt_sum;
	las_pt_sum = lasreader->header.number_of_point_records;
	if (las_pt_sum)
	{
		printf("点的个数:%d\n", las_pt_sum);
		printf("分块:%d*%d\n", X_Block, Y_Block);
		lasHdr = lasreader->header;
		lasreader->close();
		double Xmin = lasHdr.min_x;
		double Xmax = lasHdr.max_x;
		double Ymin = lasHdr.min_y;
		double Ymax = lasHdr.max_y;
		double X_length = (Xmax - Xmin) / X_Block;
		double Y_length = (Ymax - Ymin) / Y_Block;
		X_length += 0.01;
		Y_length += 0.01;//扩张1cm;前闭后开
		////得到各个块范围
		////得到每块点数////
		int*BlockPointNum = new int[X_Block*Y_Block];//不重复的格网点个数//
		for (int i = 0; i < X_Block*Y_Block; i++)
		{
			BlockPointNum[i] = 0;
		}
		int O_Num = 20000000;//每次1000万；
		int O_startNum = 0;
		int ReadNum = ceil(1.0*las_pt_sum / O_Num);
		Points_Statistics(srcPath, BlockPointNum, X_Block, Y_Block, X_length, Y_length,X_OverLap,Y_OverLap);
		int*BlockPoint_Start = new int[X_Block*Y_Block];//不重复的格网点个数//
		for (int i = 0; i < X_Block*Y_Block; i++)
		{
			BlockPoint_Start[i] = 0;
		}
		LASwriteOpener*las_file_Out = new LASwriteOpener[X_Block*Y_Block];
		LASwriter **laswriter = new LASwriter *[X_Block*Y_Block];
		LASpoint pt;
		for (int j = 0; j < Y_Block; j++)
			for (int i = 0; i < X_Block; i++)
			{
				blockIndex[j*X_Block + i] = -1;
				if (BlockPointNum[i + j * X_Block] <= 0)
				{
					blockNum[i + j * X_Block] = 0;
					continue;
				}
				char outPath[PATH_SIZE];
				sprintf_s(outPath, "%s%s%d%s", dstPathDir, "/", i + j * X_Block, ".las");
				LASheader tempHdr;
				tempHdr.x_scale_factor = lasHdr.x_scale_factor;
				tempHdr.y_scale_factor = lasHdr.y_scale_factor;
				tempHdr.z_scale_factor = lasHdr.z_scale_factor;
				tempHdr.x_offset = lasHdr.x_offset;
				tempHdr.y_offset = lasHdr.y_offset;
				tempHdr.z_offset = lasHdr.z_offset;
				tempHdr.point_data_format = 1;
				tempHdr.point_data_record_length = 28;
				tempHdr.min_z = lasHdr.min_z;
				tempHdr.max_z = lasHdr.max_z;
				tempHdr.min_x = max(Xmin, Xmin + i * X_length - X_OverLap);
				tempHdr.max_x = min(Xmax, Xmin + (i + 1) * X_length + X_OverLap);
				tempHdr.min_y = max(Ymin, Ymin + j * Y_length - Y_OverLap);
				tempHdr.max_y = min(Ymax, Ymin + (j + 1) * Y_length + Y_OverLap);
				tempHdr.number_of_point_records = BlockPointNum[i + j * X_Block];
				pt.init(&tempHdr, tempHdr.point_data_format, tempHdr.point_data_record_length, 0);
				las_file_Out[i + j * X_Block].set_file_name(outPath);
				laswriter[i + j * X_Block] = las_file_Out[i + j * X_Block].open(&tempHdr);
				blockIndex[j*X_Block + i] = blockPaths.size();
				blockPaths.push_back(outPath);
				blockNum[i + j * X_Block] = BlockPointNum[i + j * X_Block];
			}
		////循环处理//
		LASreadOpener lasreadopener2;
		lasreadopener2.set_file_name(srcPath);
		LASreader* lasreader2 = lasreadopener2.open();
		lasHdr = lasreader2->header;
		while (lasreader2->read_point())
		{
			pt.set_X(lasreader2->point.X);
			pt.set_Y(lasreader2->point.Y);
			pt.set_Z(lasreader2->point.Z);
			double x = lasreader2->point.get_x();
			double y = lasreader2->point.get_y();
			int colX = (x - lasHdr.min_x) / X_length;
			int rowY = (y - lasHdr.min_y) / Y_length;
			BlockPointNum[colX + rowY * X_Block]++;
			laswriter[colX + rowY * X_Block]->write_point(&pt);
			laswriter[colX + rowY * X_Block]->update_inventory(&pt);
			double localCentreX = lasHdr.min_x + colX * X_length + X_length / 2;
			double localCentreY = lasHdr.min_y + rowY * Y_length + Y_length / 2;
			if (fabs(x - localCentreX) > (X_length / 2 - X_OverLap) || fabs(y - localCentreY) > (Y_length / 2 - Y_OverLap))
			{
				if (rowY < Y_Block - 1 && y - localCentreY >(Y_length / 2 - Y_OverLap) && fabs(x - localCentreX) <= (X_length / 2 - X_OverLap))
				{
					BlockPointNum[colX + (rowY + 1)* X_Block]++;
					laswriter[colX + (rowY + 1)* X_Block]->write_point(&pt);
					laswriter[colX + (rowY + 1)* X_Block]->update_inventory(&pt);
				}//0
				else {
					if (rowY > 0 && y - localCentreY < -(Y_length / 2 - Y_OverLap) && fabs(x - localCentreX) <= (X_length / 2 - X_OverLap))
					{
						BlockPointNum[colX + (rowY - 1)* X_Block]++;
						laswriter[colX + (rowY - 1)* X_Block]->write_point(&pt);
						laswriter[colX + (rowY - 1)* X_Block]->update_inventory(&pt);
					}//4
					else
					{
						if (colX > 0 && fabs(y - localCentreY) <= (Y_length / 2 - Y_OverLap) && x - localCentreX < -(X_length / 2 - X_OverLap))
						{
							BlockPointNum[colX - 1 + (rowY)* X_Block]++;
							laswriter[colX - 1 + (rowY)* X_Block]->write_point(&pt);
							laswriter[colX - 1 + (rowY)* X_Block]->update_inventory(&pt);
						}//6
						else
						{
							if (colX< X_Block - 1 && fabs(y - localCentreY) <= (Y_length / 2 - Y_OverLap) && x - localCentreX >(X_length / 2 - X_OverLap))
							{
								BlockPointNum[colX + 1 + (rowY)* X_Block]++;
								laswriter[colX + 1 + (rowY)* X_Block]->write_point(&pt);
								laswriter[colX + 1 + (rowY)* X_Block]->update_inventory(&pt);
							}//2
							else
							{
								if (x - localCentreX > (X_length / 2 - X_OverLap) && y - localCentreY < -(Y_length / 2 - Y_OverLap))
								{
									if (colX < X_Block - 1)
									{
										BlockPointNum[colX + 1 + (rowY)* X_Block]++;
										laswriter[colX + 1 + (rowY)* X_Block]->write_point(&pt);
										laswriter[colX + 1 + (rowY)* X_Block]->update_inventory(&pt);
										if (rowY > 0)
										{
											BlockPointNum[colX + 1 + (rowY - 1)* X_Block]++;
											laswriter[colX + 1 + (rowY - 1)* X_Block]->write_point(&pt);
											laswriter[colX + 1 + (rowY - 1)* X_Block]->update_inventory(&pt);
										}
									}
									if (rowY > 0) {
										BlockPointNum[colX + (rowY - 1)* X_Block]++;
										laswriter[colX + (rowY - 1)* X_Block]->write_point(&pt);
										laswriter[colX + (rowY - 1)* X_Block]->update_inventory(&pt);
									}
								}//3
								else
								{
									if (x - localCentreX > (X_length / 2 - X_OverLap) && y - localCentreY > (Y_length / 2 - Y_OverLap))
									{
										if (colX < X_Block - 1)
										{
											BlockPointNum[colX + 1 + (rowY)* X_Block]++;
											laswriter[colX + 1 + (rowY)* X_Block]->write_point(&pt);
											laswriter[colX + 1 + (rowY)* X_Block]->update_inventory(&pt);
											if (rowY < Y_Block - 1)
											{
												BlockPointNum[colX + 1 + (rowY + 1)* X_Block]++;
												laswriter[colX + 1 + (rowY + 1)* X_Block]->write_point(&pt);
												laswriter[colX + 1 + (rowY + 1)* X_Block]->update_inventory(&pt);
											}
										}
										if (rowY < Y_Block - 1) {
											BlockPointNum[colX + (rowY + 1)* X_Block]++;
											laswriter[colX + (rowY + 1)* X_Block]->write_point(&pt);
											laswriter[colX + (rowY + 1)* X_Block]->update_inventory(&pt);
										}
									}//1
									else
									{
										if (x - localCentreX <-(X_length / 2 - X_OverLap) && y - localCentreY >(Y_length / 2 - Y_OverLap))
										{
											if (colX > 0)
											{
												BlockPointNum[colX - 1 + (rowY)* X_Block]++;
												laswriter[colX - 1 + (rowY)* X_Block]->write_point(&pt);
												laswriter[colX - 1 + (rowY)* X_Block]->update_inventory(&pt);
												if (rowY < Y_Block - 1)
												{
													BlockPointNum[colX - 1 + (rowY + 1)* X_Block]++;
													laswriter[colX - 1 + (rowY + 1)* X_Block]->write_point(&pt);
													laswriter[colX - 1 + (rowY + 1)* X_Block]->update_inventory(&pt);
												}
											}
											if (rowY < Y_Block - 1) {
												BlockPointNum[colX + (rowY + 1)* X_Block]++;
												laswriter[colX + (rowY + 1)* X_Block]->write_point(&pt);
												laswriter[colX + (rowY + 1)* X_Block]->update_inventory(&pt);
											}
										}//7
										else
										{
											if (x - localCentreX < -(X_length / 2 - X_OverLap) && y - localCentreY < -(Y_length / 2 - Y_OverLap))
											{
												if (colX > 0)
												{
													BlockPointNum[colX - 1 + (rowY)* X_Block]++;
													laswriter[colX - 1 + (rowY)* X_Block]->write_point(&pt);
													laswriter[colX - 1 + (rowY)* X_Block]->update_inventory(&pt);
													if (rowY > 0)
													{
														BlockPointNum[colX - 1 + (rowY - 1)* X_Block]++;
														laswriter[colX - 1 + (rowY - 1)* X_Block]->write_point(&pt);
														laswriter[colX - 1 + (rowY - 1)* X_Block]->update_inventory(&pt);
													}
												}
												if (rowY > 0) {
													BlockPointNum[colX + (rowY - 1)* X_Block]++;
													laswriter[colX + (rowY - 1)* X_Block]->write_point(&pt);
													laswriter[colX + (rowY - 1)* X_Block]->update_inventory(&pt);
												}
											}//5
										}//else
									}//else
								}//else
							}//else
						}//else
					}//else
				}//else
			}

		}//for//
		lasreader2->close();
		for (int i = 0; i < X_Block*Y_Block; i++)
		{
			if (BlockPointNum[i] > 0) {
				laswriter[i]->close();
			}
		}
	}
}
void getBlockInfo(const char*srcPath, myblockInfo&lasBlockInfo)
{
	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(srcPath);
	LASreader*lasreader = lasreadopener.open();
	LASheader myheader = lasreader->header;
	lasreader->close();
	long long las_pt_sum = myheader.number_of_point_records;
	double Xmin = myheader.min_x;
	double Xmax = myheader.max_x;
	double Ymin = myheader.min_y;
	double Ymax = myheader.max_y;
	double Zmin = myheader.min_z;
	double Zmax = myheader.max_z;
	//根据数量来划分格网//
	//每块不超过两千万//
	int blockNum = ceil(1.0*las_pt_sum / 1000000);
	double xyscale = (Ymax - Ymin) / (Xmax - Xmin);
	int xblock = max(1, ceil(sqrt(blockNum / xyscale)));
	int yblock = ceil(1.0*blockNum / xblock);
	double X_length = (Xmax - Xmin) / xblock;
	double Y_length = (Ymax - Ymin) / yblock;
	X_length += 0.01;
	Y_length += 0.01;//扩张1cm;前闭后开
	double xoverlap = min(5.0, X_length / 20);
	double yoverlap = min(5.0, Y_length / 20);
	lasBlockInfo.X_Block = xblock;
	lasBlockInfo.Y_Block = yblock;
	lasBlockInfo.X_OverLap = xoverlap;
	lasBlockInfo.Y_OverLap = yoverlap;
}
//点云分块函数//
void getRunPathDir(string &CPrunPath)
{
	char runPath[PATH_SIZE];//（D:\Documents\Downloads\TEST.exe）
	std::memset(runPath, 0, PATH_SIZE);
	GetModuleFileNameA(NULL, runPath, PATH_SIZE);
	string myRunPath = runPath;
	int pos = myRunPath.find("\\");////查找指定的串
	while (pos != -1)
	{
		printf("%d\n", pos);
		myRunPath.replace(pos, 1, "/");////用新的串替换掉指定的串

		pos = myRunPath.find("\\");/////继续查找指定的串，直到所有的都找到为止
	}
	sprintf_s(runPath, "%s", myRunPath.c_str());
	strcpy_s(strrchr(runPath, '/'), 1, "");
	CPrunPath = runPath;
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

void getCurrentDir(const char*path, string&pathDir)
{
	char pszDst[PATH_SIZE];
	string tempPath1;
	Pretreatment_seg(path, tempPath1);
	strcpy_s(pszDst, strlen(tempPath1.c_str()) + 1, tempPath1.c_str());
	strcpy_s(strrchr(pszDst, '/'), 1, "");
	pathDir = pszDst;
}
void createFold(const char*pathDir, const char*foldName, string&path)
{
	char MosaicPath[PATH_SIZE];
	sprintf_s(MosaicPath, "%s%s%s", pathDir, "/", foldName);
	if (_access(MosaicPath, 0) == 0)
	{
		_rmdir(MosaicPath);
		_mkdir(MosaicPath);
	}
	else
	{
		_mkdir(MosaicPath);
	}
	path = MosaicPath;
}
void getBorderInfo(string path1, string path2, int index, LASheader&lashdr)
{
	if (index == 1)//上下关系
	{
		LASreadOpener lasreadopener1;
		lasreadopener1.set_file_name(path1.c_str());
		LASreader* lasreader1 = lasreadopener1.open();
		LASheader myheader1 = lasreader1->header;
		lasreader1->close();

		LASreadOpener lasreadopener2;
		lasreadopener2.set_file_name(path2.c_str());
		LASreader* lasreader2 = lasreadopener2.open();
		LASheader myheader2 = lasreader2->header;
		lasreader2->close();

		lashdr = myheader1;
		lashdr.max_x = max(myheader1.max_x, myheader2.max_x);
		lashdr.min_x = min(myheader1.min_x, myheader2.min_x);
		lashdr.max_y = (myheader2.min_y + myheader2.max_y) / 2;
		lashdr.min_y = (myheader1.max_y + myheader1.min_y) / 2;
	}
	else {
		if (index == 2)//左右关系
		{
			LASreadOpener lasreadopener1;
			lasreadopener1.set_file_name(path1.c_str());
			LASreader* lasreader1 = lasreadopener1.open();
			LASheader myheader1 = lasreader1->header;
			lasreader1->close();

			LASreadOpener lasreadopener2;
			lasreadopener2.set_file_name(path2.c_str());
			LASreader* lasreader2 = lasreadopener2.open();
			LASheader myheader2 = lasreader2->header;
			lasreader2->close();

			lashdr = myheader1;
			lashdr.max_y = max(myheader1.max_y, myheader2.max_y);
			lashdr.min_y = min(myheader1.min_y, myheader2.min_y);
			lashdr.max_x = (myheader2.min_x + myheader2.max_x) / 2;
			lashdr.min_x = (myheader1.max_x + myheader1.min_x) / 2;
		}
	}
}
void Points_Border(vector<string>blockPaths, int*PathsIndex, myblockInfo lasBlockInfo, const char*BorderPath, vector<string>&borderPaths)
{
	//PathsIndex记录了对应分块的path,为-1，表明这个分块为空//
	//_b结尾//
	//看缝在何处//
	borderPaths.clear();
	int X_Block = lasBlockInfo.X_Block;
	int Y_Block = lasBlockInfo.Y_Block;
	////得到各个块范围
	////得到每块点数////
	//border 共有X_Block*(YBlock-1)(横向), （X_Block-1)*Y_Block(竖向)
   //横向
	for (int i = 0; i < Y_Block - 1; i++)
		for (int j = 0; j < X_Block; j++)
		{
			//i, j;
			//i + 1, j;
			int index1 = PathsIndex[i*X_Block + j];
			int index2 = PathsIndex[(i + 1)*X_Block + j];
			/////////////////////////////
			if (index1 != -1 && index2 != -1)
			{
				//创建一个//
				char outPath[PATH_SIZE];
				sprintf_s(outPath, "%s%s%d_%d%s", BorderPath, "/", index1, index2, ".las");
				LASwriteOpener laswriteropener;
				laswriteropener.set_file_name(outPath);
				borderPaths.push_back(outPath);
				LASheader lasheader;
				getBorderInfo(blockPaths[index1].c_str(), blockPaths[index2].c_str(), 1, lasheader);
				LASwriter *laswriter = laswriteropener.open(&lasheader);
				int rec_xmin = (lasheader.min_x - lasheader.x_offset) / lasheader.x_scale_factor;
				int rec_xmax = (lasheader.max_x - lasheader.x_offset) / lasheader.x_scale_factor;
				int rec_ymin = (lasheader.min_y - lasheader.y_offset) / lasheader.y_scale_factor;
				int rec_ymax = (lasheader.max_y - lasheader.y_offset) / lasheader.y_scale_factor;
				int validPointNum = 0;
				LASreadOpener lasreadopener1;
				lasreadopener1.set_file_name(blockPaths[index1].c_str());
				LASreader* lasreader1 = lasreadopener1.open();
				while (lasreader1->read_point())
				{

					if (lasreader1->point.X <= rec_xmax && lasreader1->point.X >= rec_xmin && lasreader1->point.Y <= rec_ymax && lasreader1->point.Y >= rec_ymin)
					{
						validPointNum++;
						laswriter->write_point(&(lasreader1->point));
						laswriter->update_inventory(&(lasreader1->point));
					}
				}
				lasreader1->close();
				LASreadOpener lasreadopener2;
				lasreadopener2.set_file_name(blockPaths[index2].c_str());
				LASreader* lasreader2 = lasreadopener2.open();
				while (lasreader2->read_point())
				{
					if (lasreader2->point.X <= rec_xmax && lasreader2->point.X >= rec_xmin && lasreader2->point.Y <= rec_ymax && lasreader2->point.Y >= rec_ymin)
					{
						validPointNum++;
						laswriter->write_point(&(lasreader2->point));
						laswriter->update_inventory(&(lasreader2->point));
					}
				}
				lasreader2->close();
				lasheader.number_of_point_records = validPointNum;
				laswriter->update_header(&lasheader, TRUE);
				laswriter->close();
			}
		}
	//纵向
	for (int i = 0; i < Y_Block; i++)
		for (int j = 0; j < X_Block - 1; j++)
		{
			int index1 = PathsIndex[i*X_Block + j];
			int index2 = PathsIndex[(i)*X_Block + j + 1];
			/////////////////////////////
			if (index1 != -1 && index2 != -1)
			{
				//创建一个//
				char outPath[PATH_SIZE];
				sprintf_s(outPath, "%s%s%d_%d%s", BorderPath, "/", index1, index2, ".las");
				LASwriteOpener laswriteropener;
				laswriteropener.set_file_name(outPath);
				borderPaths.push_back(outPath);
				LASheader lasheader;
				getBorderInfo(blockPaths[index1].c_str(), blockPaths[index2].c_str(), 2, lasheader);
				LASwriter *laswriter = laswriteropener.open(&lasheader);
				int rec_xmin = (lasheader.min_x - lasheader.x_offset) / lasheader.x_scale_factor;
				int rec_xmax = (lasheader.max_x - lasheader.x_offset) / lasheader.x_scale_factor;
				int rec_ymin = (lasheader.min_y - lasheader.y_offset) / lasheader.y_scale_factor;
				int rec_ymax = (lasheader.max_y - lasheader.y_offset) / lasheader.y_scale_factor;
				int validPointNum = 0;
				LASreadOpener lasreadopener1;
				lasreadopener1.set_file_name(blockPaths[index1].c_str());
				LASreader* lasreader1 = lasreadopener1.open();
				while (lasreader1->read_point())
				{

					if (lasreader1->point.X <= rec_xmax && lasreader1->point.X >= rec_xmin && lasreader1->point.Y <= rec_ymax && lasreader1->point.Y >= rec_ymin)
					{
						validPointNum++;
						laswriter->write_point(&(lasreader1->point));
						laswriter->update_inventory(&(lasreader1->point));
					}
				}
				lasreader1->close();
				LASreadOpener lasreadopener2;
				lasreadopener2.set_file_name(blockPaths[index2].c_str());
				LASreader* lasreader2 = lasreadopener2.open();
				while (lasreader2->read_point())
				{
					if (lasreader2->point.X <= rec_xmax && lasreader2->point.X >= rec_xmin && lasreader2->point.Y <= rec_ymax && lasreader2->point.Y >= rec_ymin)
					{
						validPointNum++;
						laswriter->write_point(&(lasreader2->point));
						laswriter->update_inventory(&(lasreader2->point));
					}
				}
				lasreader2->close();
				lasheader.number_of_point_records = validPointNum;
				laswriter->update_header(&lasheader, TRUE);
				laswriter->close();
			}
		}
}
int main(int argc, char**argv)
{
	double totaltime;
	myblockInfo lasBlockInfo;
	getBlockInfo(argv[1], lasBlockInfo);
	vector<string>blockPaths;
	blockPaths.clear();
	int*blockNum;
	LASheader lasHdr;
	string pathDir;
	getCurrentDir(argv[1], pathDir);

	char BlockPath[PATH_SIZE];
	sprintf_s(BlockPath, "%s", argv[2]);
	if (_access(BlockPath, 0) == 0)
	{
		_rmdir(BlockPath);
		_mkdir(BlockPath);
	}
	else
	{
		_mkdir(BlockPath);
	}
	int*PathsIndex;
	Points_Block(argv[1], BlockPath, lasBlockInfo.X_Block, lasBlockInfo.Y_Block, lasBlockInfo.X_OverLap,lasBlockInfo.Y_OverLap, blockPaths, PathsIndex, blockNum, lasHdr);

	/*vector<string>borderPaths;
	char BorderPath[PATH_SIZE];
	sprintf_s(BorderPath, "%s", argv[3]);
	if (_access(BorderPath, 0) == 0)
	{
		_rmdir(BorderPath);
		_mkdir(BorderPath);
	}
	else
	{
		_mkdir(BorderPath);
	}
	borderPaths.clear();*/
	//Points_Border(blockPaths, PathsIndex, lasBlockInfo, BorderPath, borderPaths);
	delete[]PathsIndex;
	delete[]blockNum;
}
//2091208改进，将dsm_out.ini改进到tsk同目录下的文件//
#include<map>
#include<set>
#include<list>
#include<malloc.h>
#include<time.h>
#define REAL double
#include "gdal_priv.h"  
#include "ogrsf_frmts.h" //for ogr  
#include "gdal_alg.h"  //for GDALPolygonize  
#include <io.h>
#include"iostream"
#include"vector"
#include "lasreader.hpp"
#include"laswriter.hpp"
using namespace std;
#define ANSI_DECLARATORS 
extern "C"
{
#include "triangle.h"
}
#define PATH_SIZE 256

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
void Triangle(double*points, int Num, vector<vector<int>>&myTriangle)
{
	struct triangulateio in, mid, vorout;
	in.numberofpoints = Num;
	in.numberofpointattributes = 1;

	in.pointlist = (REAL *)malloc(in.numberofpoints * 2 * sizeof(REAL));
	for (int i = 0; i < 2 * Num; i++)
	{
		in.pointlist[i] = points[i];
	}
	in.pointattributelist = (REAL *)malloc(in.numberofpoints *
		in.numberofpointattributes *
		sizeof(REAL));
	in.pointmarkerlist = (int *)malloc(in.numberofpoints * sizeof(int));
	//特征值和标记值都默认为零
	for (int i = 0; i < Num; i++)
	{
		in.pointattributelist[i] = 0.0;
		in.pointmarkerlist[i] = 0;
	}
	in.numberofsegments = 0;
	in.numberofholes = 0;
	in.numberofregions = 0;
	// in.regionlist = (REAL *) malloc(in.numberofregions * 4 * sizeof(REAL));
	//in.regionlist[0] = 0.5;
	//in.regionlist[1] = 5.0;
	// in.regionlist[2] = 7.0;            /* Regional attribute (for whole mesh). */
	//  in.regionlist[3] = 0.1;          /* Area constraint that will not be used. */

	//printf("Input point set:\n\n");
	//report(&in, 1, 0, 0, 0, 0, 0);

	/* Make necessary initializations so that Triangle can return a */
	/*   triangulation in `mid' and a voronoi diagram in `vorout'.  */

	mid.pointlist = (REAL *)NULL;            /* Not needed if -N switch used. */
											 /* Not needed if -N switch used or number of point attributes is zero: */
	mid.pointattributelist = (REAL *)NULL;
	mid.pointmarkerlist = (int *)NULL; /* Not needed if -N or -B switch used. */
	mid.trianglelist = (int *)NULL;          /* Not needed if -E switch used. */
											 /* Not needed if -E switch used or number of triangle attributes is zero: */
	mid.triangleattributelist = (REAL *)NULL;
	mid.neighborlist = (int *)NULL;         /* Needed only if -n switch used. */
											/* Needed only if segments are output (-p or -c) and -P not used: */
	mid.segmentlist = (int *)NULL;
	/* Needed only if segments are output (-p or -c) and -P and -B not used: */
	mid.segmentmarkerlist = (int *)NULL;
	mid.edgelist = (int *)NULL;             /* Needed only if -e switch used. */
	mid.edgemarkerlist = (int *)NULL;   /* Needed if -e used and -B not used. */

	vorout.pointlist = (REAL *)NULL;        /* Needed only if -v switch used. */
											/* Needed only if -v switch used and number of attributes is not zero: */
	vorout.pointattributelist = (REAL *)NULL;
	vorout.edgelist = (int *)NULL;          /* Needed only if -v switch used. */
	vorout.normlist = (REAL *)NULL;    		/*   neighbor list (n).                                              */
	char*p = new char[9];
	strcpy_s(p, 9, "pczAevn");
	triangulate(p, &in, &mid, &vorout);
	vector<int>tempVector;
	tempVector.clear();
	for (int i = 0; i < mid.numberoftriangles; i++) {
		int temp1 = mid.trianglelist[i * mid.numberofcorners + 0];
		int temp2 = mid.trianglelist[i * mid.numberofcorners + 1];
		int temp3 = mid.trianglelist[i * mid.numberofcorners + 2];
		//mid.neighborlist
		tempVector.clear();
		tempVector.push_back(temp1); tempVector.push_back(temp2); tempVector.push_back(temp3);
		myTriangle.push_back(tempVector);
	}
	free(in.pointlist);
	free(in.pointattributelist);
	free(in.pointmarkerlist);
	free(mid.pointlist);
	free(mid.pointattributelist);
	free(mid.pointmarkerlist);
	free(mid.trianglelist);
	free(mid.triangleattributelist);
	free(mid.neighborlist);
	free(mid.segmentlist);
	free(mid.segmentmarkerlist);
	free(mid.edgelist);
	free(mid.edgemarkerlist);
	free(vorout.pointlist);
	free(vorout.pointattributelist);
	free(vorout.edgelist);
	free(vorout.normlist);
}	 
bool InitialProc(point3D*points, int num, CloudHeader mylasHdr, double*&points_after, double*&myZ, int&myNum)
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
	myNum = residualPoints.size();
	points_after = new double[residualPoints.size() * 2];
	myZ = new double[residualPoints.size()];
	for (int i = 0; i < residualPoints.size(); i++)
	{
		double localPointx = points[residualPoints[i]].X*mylasHdr.x_scale_factor + mylasHdr.x_offset;
		double localPointy = points[residualPoints[i]].Y*mylasHdr.y_scale_factor + mylasHdr.y_offset;
		double localPointz = points[residualPoints[i]].Z*mylasHdr.z_scale_factor + mylasHdr.z_offset;
		points_after[2 * i] = localPointx;
		points_after[2 * i + 1] = localPointy;
		myZ[i] = localPointz;
	}
}
//考虑到邻域三角网//
void Triangle_Neighbor(double*points, int Num, int**&myTriangle, int**&nb_Triangle, int&TriNum)
{
	struct triangulateio in, mid, vorout;
	in.numberofpoints = Num;
	in.numberofpointattributes = 1;
	in.pointlist = (REAL *)malloc(in.numberofpoints * 2 * sizeof(REAL));
	for (int i = 0; i < 2 * Num; i++)
	{
		in.pointlist[i] = points[i];
	}
	in.pointattributelist = (REAL *)malloc(in.numberofpoints *
		in.numberofpointattributes *
		sizeof(REAL));
	in.pointmarkerlist = (int *)malloc(in.numberofpoints * sizeof(int));
	//特征值和标记值都默认为零
	for (int i = 0; i < Num; i++)
	{
		in.pointattributelist[i] = 0.0;
		in.pointmarkerlist[i] = 0;
	}
	in.numberofsegments = 0;
	in.numberofholes = 0;
	in.numberofregions = 0;
	// in.regionlist = (REAL *) malloc(in.numberofregions * 4 * sizeof(REAL));
	//in.regionlist[0] = 0.5;
	//in.regionlist[1] = 5.0;
	// in.regionlist[2] = 7.0;            /* Regional attribute (for whole mesh). */
	//  in.regionlist[3] = 0.1;          /* Area constraint that will not be used. */

	//printf("Input point set:\n\n");
	//report(&in, 1, 0, 0, 0, 0, 0);

	/* Make necessary initializations so that Triangle can return a */
	/*   triangulation in `mid' and a voronoi diagram in `vorout'.  */
	mid.pointlist = (REAL *)NULL;  		 /* Not needed if -N switch used or number of point attributes is zero: */
	mid.pointattributelist = (REAL *)NULL;
	mid.pointmarkerlist = (int *)NULL; /* Not needed if -N or -B switch used. */
	mid.trianglelist = (int *)NULL;   	 /* Not needed if -E switch used or number of triangle attributes is zero: */
	mid.triangleattributelist = (REAL *)NULL;
	mid.neighborlist = (int *)NULL;  		/* Needed only if segments are output (-p or -c) and -P not used: */
	mid.segmentlist = (int *)NULL;
	/* Needed only if segments are output (-p or -c) and -P and -B not used: */
	mid.segmentmarkerlist = (int *)NULL;
	mid.edgelist = (int *)NULL;             /* Needed only if -e switch used. */
	mid.edgemarkerlist = (int *)NULL;
	vorout.pointlist = (REAL *)NULL;  		/* Needed only if -v switch used and number of attributes is not zero: */
	vorout.pointattributelist = (REAL *)NULL;
	vorout.edgelist = (int *)NULL;          /* Needed only if -v switch used. */
	vorout.normlist = (REAL *)NULL;    		/*   neighbor list (n).                                              */
	char*p = new char[9];
	strcpy_s(p, 9, "pczAevn");
	triangulate(p, &in, &mid, &vorout);
	int tempVector[3];
	int tempVector1[3];
	TriNum = mid.numberoftriangles;
	myTriangle = new int*[TriNum];
	for (int i = 0; i < TriNum; i++)
	{
		myTriangle[i] = new int[3];
	}
	nb_Triangle = new int*[TriNum];
	for (int i = 0; i < TriNum; i++)
	{
		nb_Triangle[i] = new int[3];
	}
	for (int i = 0; i < TriNum; i++) {
		int temp1 = mid.trianglelist[i * mid.numberofcorners + 0];
		int temp2 = mid.trianglelist[i * mid.numberofcorners + 1];
		int temp3 = mid.trianglelist[i * mid.numberofcorners + 2];
		int temp4 = mid.neighborlist[i * 3 + 0];
		int temp5 = mid.neighborlist[i * 3 + 1];
		int temp6 = mid.neighborlist[i * 3 + 2];
		//mid.neighborlist
		myTriangle[i][0] = temp1; myTriangle[i][1] = temp2; myTriangle[i][2] = temp3;
		nb_Triangle[i][0] = temp4; nb_Triangle[i][1] = temp5; nb_Triangle[i][2] = temp6;
	}
	free(in.pointlist);
	free(in.pointattributelist);
	free(in.pointmarkerlist);
	free(mid.pointlist);
	free(mid.pointattributelist);
	free(mid.pointmarkerlist);
	free(mid.trianglelist);
	free(mid.triangleattributelist);
	free(mid.neighborlist);
	free(mid.segmentlist);
	free(mid.segmentmarkerlist);
	free(mid.edgelist);
	free(mid.edgemarkerlist);
	free(vorout.pointlist);
	free(vorout.pointattributelist);
	free(vorout.edgelist);
	free(vorout.normlist);
}
void Interpola_Neighbor(myRegon blockRegion, double*points_after, double*myZ, int Num_after, int**myTriangle, int**nb_Triangle, int triNum, double xresolution, double yresolution, float*&DEMData)
{
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
	for (int i = 0; i < triNum; i++) {
		int temp1 = myTriangle[i][0];
		int temp2 = myTriangle[i][1];
		int temp3 = myTriangle[i][2];
		double x1 = points_after[2 * temp1];
		double y1 = points_after[2 * temp1 + 1];
		double z1 = myZ[temp1];
		double x2 = points_after[2 * temp2];
		double y2 = points_after[2 * temp2 + 1];
		double z2 = myZ[temp2];
		double x3 = points_after[2 * temp3];
		double y3 = points_after[2 * temp3 + 1];
		double z3 = myZ[temp3];
		double xmin = min(x3, min(x1, x2));
		double xmax = max(x3, max(x1, x2));
		double ymin = min(y3, min(y1, y2));
		double ymax = max(y3, max(y1, y2));
		double zmin = min(z1, min(z2, z3));
		double zmax = max(z1, max(z2, z3));
		int startX = (xmin - adsTransform[0]) / adsTransform[1];
		int endX = (xmax - adsTransform[0]) / adsTransform[1];
		int startY = (ymax - adsTransform[3]) / adsTransform[5];
		int endY = (ymin - adsTransform[3]) / adsTransform[5];
		for (int ii = startX + 1; ii <= endX; ii++)
			for (int jj = startY + 1; jj <= endY; jj++)
			{
				double localX = ii * adsTransform[1] + adsTransform[0];
				double localY = jj * adsTransform[5] + adsTransform[3];
				double area1 = abs((x1 - localX)*(y2 - localY) - (y1 - localY)*(x2 - localX));
				double area2 = abs((x1 - localX)*(y3 - localY) - (y1 - localY)*(x3 - localX));
				double area3 = abs((x2 - localX)*(y3 - localY) - (y2 - localY)*(x3 - localX));
				double area = abs((x2 - x1)*(y3 - y1) - (y2 - y1)*(x3 - x1));
				if (fabs(area1 + area2 + area3 - area) < 0.00001&&ii >= 0 && ii < m_nwidth&&jj >= 0 && jj < m_nheight)
				{
					//插值
					double d1 = sqrt((x1 - localX)*(x1 - localX) + (y1 - localY)*(y1 - localY));
					double d2 = sqrt((x2 - localX)*(x2 - localX) + (y2 - localY)*(y2 - localY));
					double d3 = sqrt((x3 - localX)*(x3 - localX) + (y3 - localY)*(y3 - localY));
					double IPZ = (d2*d3*z1 + d1 * d3*z2 + d1 * d2*z3) / (d2*d3 + d1 * d3 + d1 * d2);
					//DEMData[jj*m_nwidth + ii] = (d2*d3*z1 + d1 * d3*z2 + d1 * d2*z3) / (d2*d3 + d1 * d3 + d1 * d2);
					//temp1//temp2//temp3//
					vector<double>localZ;
					vector<double>localWeight;
					localZ.clear(); localWeight.clear();
					if (nb_Triangle[i][0] != -1)
					{
						double localdistance = area3 / sqrt((y3 - y2)*(y3 - y2) + (x3 - x2)*(x3 - x2));
						localWeight.push_back(1 / localdistance);
						int localtemp1 = myTriangle[nb_Triangle[i][0]][0];
						int localtemp2 = myTriangle[nb_Triangle[i][0]][1];
						int localtemp3 = myTriangle[nb_Triangle[i][0]][2];
						double localx1 = points_after[2 * localtemp1];
						double localy1 = points_after[2 * localtemp1 + 1];
						double localz1 = myZ[localtemp1];
						double localx2 = points_after[2 * localtemp2];
						double localy2 = points_after[2 * localtemp2 + 1];
						double localz2 = myZ[localtemp2];
						double localx3 = points_after[2 * localtemp3];
						double localy3 = points_after[2 * localtemp3 + 1];
						double localz3 = myZ[localtemp3];
						double locald1 = sqrt((localx1 - localX)*(localx1 - localX) + (localy1 - localY)*(localy1 - localY));
						double locald2 = sqrt((localx2 - localX)*(localx2 - localX) + (localy2 - localY)*(localy2 - localY));
						double locald3 = sqrt((localx3 - localX)*(localx3 - localX) + (localy3 - localY)*(localy3 - localY));
						double tempZ = (locald2*locald3*localz1 + locald1 * locald3*localz2 + locald1 * locald2*localz3) / (locald2*locald3 + locald1 * locald3 + locald1 * locald2);
						localZ.push_back(tempZ);
						if (localdistance < 0.00001)
						{
							localZ.pop_back();
							localWeight.pop_back();
						}

					}
					if (nb_Triangle[i][1] != -1)
					{
						double localdistance = area2 / sqrt((y3 - y1)*(y3 - y1) + (x3 - x1)*(x3 - x1));
						localWeight.push_back(1 / localdistance);
						int localtemp1 = myTriangle[nb_Triangle[i][1]][0];
						int localtemp2 = myTriangle[nb_Triangle[i][1]][1];
						int localtemp3 = myTriangle[nb_Triangle[i][1]][2];
						double localx1 = points_after[2 * localtemp1];
						double localy1 = points_after[2 * localtemp1 + 1];
						double localz1 = myZ[localtemp1];
						double localx2 = points_after[2 * localtemp2];
						double localy2 = points_after[2 * localtemp2 + 1];
						double localz2 = myZ[localtemp2];
						double localx3 = points_after[2 * localtemp3];
						double localy3 = points_after[2 * localtemp3 + 1];
						double localz3 = myZ[localtemp3];
						double locald1 = sqrt((localx1 - localX)*(localx1 - localX) + (localy1 - localY)*(localy1 - localY));
						double locald2 = sqrt((localx2 - localX)*(localx2 - localX) + (localy2 - localY)*(localy2 - localY));
						double locald3 = sqrt((localx3 - localX)*(localx3 - localX) + (localy3 - localY)*(localy3 - localY));
						double tempZ = (locald2*locald3*localz1 + locald1 * locald3*localz2 + locald1 * locald2*localz3) / (locald2*locald3 + locald1 * locald3 + locald1 * locald2);
						localZ.push_back(tempZ);
						if (localdistance < 0.00001)
						{
							localZ.pop_back();
							localWeight.pop_back();
						}
					}
					if (nb_Triangle[i][2] != -1)
					{
						double localdistance = area1 / sqrt((y2 - y1)*(y2 - y1) + (x2 - x1)*(x2 - x1));
						localWeight.push_back(1 / localdistance);
						int localtemp1 = myTriangle[nb_Triangle[i][2]][0];
						int localtemp2 = myTriangle[nb_Triangle[i][2]][1];
						int localtemp3 = myTriangle[nb_Triangle[i][2]][2];
						double localx1 = points_after[2 * localtemp1];
						double localy1 = points_after[2 * localtemp1 + 1];
						double localz1 = myZ[localtemp1];
						double localx2 = points_after[2 * localtemp2];
						double localy2 = points_after[2 * localtemp2 + 1];
						double localz2 = myZ[localtemp2];
						double localx3 = points_after[2 * localtemp3];
						double localy3 = points_after[2 * localtemp3 + 1];
						double localz3 = myZ[localtemp3];
						double locald1 = sqrt((localx1 - localX)*(localx1 - localX) + (localy1 - localY)*(localy1 - localY));
						double locald2 = sqrt((localx2 - localX)*(localx2 - localX) + (localy2 - localY)*(localy2 - localY));
						double locald3 = sqrt((localx3 - localX)*(localx3 - localX) + (localy3 - localY)*(localy3 - localY));
						double tempZ = (locald2*locald3*localz1 + locald1 * locald3*localz2 + locald1 * locald2*localz3) / (locald2*locald3 + locald1 * locald3 + locald1 * locald2);
						localZ.push_back(tempZ);
						if (localdistance < 0.00001)
						{
							localZ.pop_back();
							localWeight.pop_back();
						}
					}
					DEMData[jj*m_nwidth + ii] = 0.0;
					double totalWeight = 0.0;
					for (int iii = 0; iii < localZ.size(); iii++)
					{
						totalWeight += localWeight[iii];
						if (localZ[iii] < zmin)
						{
							localZ[iii] = zmin;
						}
						if (localZ[iii] > zmax)
						{
							localZ[iii] = zmax;
						}
						DEMData[jj*m_nwidth + ii] += localZ[iii] * localWeight[iii];
					}
					DEMData[jj*m_nwidth + ii] /= totalWeight;
					DEMData[jj*m_nwidth + ii] = DEMData[jj*m_nwidth + ii] * 0.5 + 0.5 *IPZ;
					if (DEMData[jj*m_nwidth + ii] == -9999)
					{
						printf("ca");
					}
					//邻域起作用//
				}
			}
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
bool DEM_sin(const char*path, myRegon blockRegion, double xSize, double ySize, float*&path_Img)
{
	point3D* points; int las_pt_num; CloudHeader myheader;
	getlasPoints(path,points, las_pt_num, myheader);
	double*points_after;
	double*myZ;
	int Num_after = 0;
	InitialProc(points, las_pt_num, myheader, points_after, myZ, Num_after);
	if (Num_after < 20)
	{
		delete[]points_after;
		delete[]myZ;
		return false;
	}
	delete[]points;
	int**myTriangle;
	int**nb_myTriangle;
	int TriNum = 0;
	//Triangle(points_after, Num_after, myTriangle);
	Triangle_Neighbor(points_after, Num_after, myTriangle, nb_myTriangle, TriNum);
	double xresolution = xSize;
	double yresolution = -1 * ySize;
	//Interpola(lasHdr,points_after, myZ, Num_after, myTriangle,xresolution, yresolution, path_Img);
	Interpola_Neighbor(blockRegion, points_after, myZ, Num_after, myTriangle, nb_myTriangle, TriNum, xresolution, yresolution, path_Img);
	for (int i = 0; i < TriNum; i++)
	{
		delete[]myTriangle[i];
		delete[]nb_myTriangle[i];
	}
	delete[]myTriangle;
	delete[]nb_myTriangle;
	delete[]points_after;
	delete[]myZ;
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
		for(int i=0;i<lasPaths.size();i++){
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
				bool mark = DEM_sin(lasPaths[i].c_str(), blockRegion, xResolution, yResolution, DEMData);
				if (!mark)
				{
					continue;
				}
				float*DEMData_temp=new float[myWidth*myHeight];
				poDataset_out->RasterIO(GF_Read, startX, startY, myWidth, myHeight, DEMData_temp, myWidth, myHeight, GDT_Float32, 1, 0, 0, 0, 0);

				////检查边框//
				//for (int j = 0; j < myWidth; j++)
				//{
				//	if (DEMData[0 * myWidth + j] == -9999)
				//	{
				//		DEMData[0 * myWidth + j] = DEMData[1 * myWidth + j];
				//	}
				//	if (DEMData[(myHeight - 1)* myWidth + j] == -9999)
				//	{
				//		DEMData[(myHeight - 1)* myWidth + j] = DEMData[1 * myWidth + j];
				//	}
				//}
				//for (int j = 0; j < myHeight; j++)
				//{
				//	if (DEMData[j * myWidth + 0] == -9999)
				//	{
				//		DEMData[j * myWidth + 0] = DEMData[j * myWidth + 1];
				//	}
				//	if (DEMData[j * myWidth + myWidth - 1] == -9999)
				//	{
				//		DEMData[j * myWidth + myWidth - 1] = DEMData[j * myWidth + myWidth - 2];
				//	}
				//}
				//对比检查//
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
		
		for (int i = 0; i < borderPaths.size(); i++) {
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
			bool mark = DEM_sin(borderPaths[i].c_str(), blockRegion, xResolution, yResolution, DEMData);
			if (!mark)
			{
				continue;
			}
			float*DEMData_temp = new float[myWidth*myHeight];
			poDataset_out->RasterIO(GF_Read, startX, startY, myWidth, myHeight, DEMData_temp, myWidth, myHeight, GDT_Float32, 1, 0, 0, 0, 0);

			////检查边框//
			//for (int j = 0; j < myWidth; j++)
			//{
			//	if (DEMData[0 * myWidth + j] == -9999)
			//	{
			//		DEMData[0 * myWidth + j] = DEMData[1 * myWidth + j];
			//	}
			//	if (DEMData[(myHeight - 1)* myWidth + j] == -9999)
			//	{
			//		DEMData[(myHeight - 1)* myWidth + j] = DEMData[1 * myWidth + j];
			//	}
			//}
			//for (int j = 0; j < myHeight; j++)
			//{
			//	if (DEMData[j * myWidth + 0] == -9999)
			//	{
			//		DEMData[j * myWidth + 0] = DEMData[j * myWidth + 1];
			//	}
			//	if (DEMData[j * myWidth + myWidth - 1] == -9999)
			//	{
			//		DEMData[j * myWidth + myWidth - 1] = DEMData[j * myWidth + myWidth - 2];
			//	}
			//}
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
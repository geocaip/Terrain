//20190822 添加Down_threshold函数，实现截断降位//
//转换信息包含 波段选择，坐标体系，分辨率，是否降位，前三个决定执行转换函数，最后一个决定执行降位函数//
#include "cprun.h"
#include"omp.h"
#include"qmessagebox.h"
#include"QCoreApplication"
#include"QProcess"
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

CPRun::CPRun(QWidget *parent) : QThread(parent)
{
	cmd.clear();
}
void  CPRun::setData(QString mycmd)
{
	cmd = mycmd;
}
void CPRun::run()
{
	if (!cmd.isEmpty())
	{
		QProcess process;
		process.execute(cmd);
		//WinExec(cmd.toLocal8Bit().data(), SW_HIDE);
		//system(cmd.toLocal8Bit().data());
		currentProgress();
		this->quit();
	}
}
void CPRun::work()
{

}

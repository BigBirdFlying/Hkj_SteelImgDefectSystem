// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\nonfree\features2d.hpp> // 这是surf用的
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp> // Hough在这里
#include <opencv2\imgproc\imgproc_c.h> // Hough在这里
#include <opencv2\gpu\gpu.hpp>
#include <opencv2\legacy\legacy.hpp>
#include "opencv2\objdetect\objdetect.hpp"
#include "opencv2\calib3d\calib3d.hpp"
#include "opencv2\nonfree\nonfree.hpp"
#include "opencv2\legacy\compat.hpp"
#include <iostream>
#include<fstream>
#include <io.h>
#include <atlstr.h> 
#include <windows.h>
#include <direct.h>
#include <ctime>
#include<vector>
#include<set>
#include<string>
#include<algorithm>
#include<math.h>
using namespace std;
using namespace cv;

// TODO: 在此处引用程序需要的其他头文件
//钢板图像数据信息
typedef struct tagSteelImgInfoAndData
{
	int iFileHead;
	char strFask[8];
	int iLenghtPos;
	int iSpeed;
	int iSequnceNo;
	int iImgWidth;
	int iImgHeight;
	short iLeftPos;
	short iRightPos;
	short iLineSpeed;
	short iCameraNo;
	cv::Mat srcImg;
}SteelImgInfoAndData,*PSteelImgInfoAndData;
//
//钢板重复面积
typedef struct tagRepeatArea
{
	int iLeftRepeat;
	int iRightRepeat;
}RepeatArea, PRepeatArea;
//
//钢板重复面积
typedef struct tagCamEffectPos
{
	int iStartPos;
	int iEndPos;
}CamEffectPos, PCamEffectPos;
//
//横纵向分别率
typedef struct tagResolutionXY
{
	float fXRes;
	float fYRes;
}ResolutionXY, PResolutionXY;
//
//各相机图像重合度及图像位置信息
typedef struct tagCamDetectContour
{
	CString strImgPath;
	int iLeftCoincide;
	int iRightCoincide;
	int iImgTh;
}CamDetectContour,*PCamDetectContour;
typedef struct tagCamDetectContourSet
{
	enum{MAX_ITEMNUM=16};
	CamDetectContour Items[MAX_ITEMNUM];
	int iItemNum;
}CamDetectContourSet,*PCamDetectContourSet;
//
//图像处理设置参数
typedef struct tagImgProcessParam
{
	int iCamGrayThIndex;
	int iCamGrayTh[16];
	int iGrayTh;
	float fGradXYTh;
}ImgProcessParam,*PImgProcessParam;
//
//轮廓左右边界
typedef struct tagSteelEdge
{
	int iLength;
	int iLeft;
	int iRight;
	int iWidth;
}SteelEdge,*PSteelEdge;
//
//轮廓剪切线
typedef struct tagCutLine
{
	int iHead;
	int iTail;
	int iLeft;
	int iRight;
}CutLine,*PCutLine;
//
//左右
typedef struct tagCamCutLeftRight
{
	int iLeft;
	int iRight;
}CamCutLeftRight,*PCamCutLeftRight;
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\nonfree\features2d.hpp> // ����surf�õ�
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp> // Hough������
#include <opencv2\imgproc\imgproc_c.h> // Hough������
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

// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
//�ְ�ͼ��������Ϣ
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
//�ְ��ظ����
typedef struct tagRepeatArea
{
	int iLeftRepeat;
	int iRightRepeat;
}RepeatArea, PRepeatArea;
//
//�ְ��ظ����
typedef struct tagCamEffectPos
{
	int iStartPos;
	int iEndPos;
}CamEffectPos, PCamEffectPos;
//
//������ֱ���
typedef struct tagResolutionXY
{
	float fXRes;
	float fYRes;
}ResolutionXY, PResolutionXY;
//
//�����ͼ���غ϶ȼ�ͼ��λ����Ϣ
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
//ͼ�������ò���
typedef struct tagImgProcessParam
{
	int iCamGrayThIndex;
	int iCamGrayTh[16];
	int iGrayTh;
	float fGradXYTh;
}ImgProcessParam,*PImgProcessParam;
//
//�������ұ߽�
typedef struct tagSteelEdge
{
	int iLength;
	int iLeft;
	int iRight;
	int iWidth;
}SteelEdge,*PSteelEdge;
//
//����������
typedef struct tagCutLine
{
	int iHead;
	int iTail;
	int iLeft;
	int iRight;
}CutLine,*PCutLine;
//
//����
typedef struct tagCamCutLeftRight
{
	int iLeft;
	int iRight;
}CamCutLeftRight,*PCamCutLeftRight;
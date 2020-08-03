#include "stdafx.h"

///*******************����ͼ��********************///
void Save_Image_OpenCV(CString strPath,cv::Mat src);
///*******************�����ļ������ļ��б�********************///
void listFiles(const char * dir, vector<vector<string>> &vecFileList);
///*******************�б��ļ����µ�imgͼ��********************///
void listImg(const char * dir, vector<string> &vecFileList);
///*******************��imgͼ���ȡ��Ϣ********************///
int GetSteelImg(CString strImgPath,SteelImgInfoAndData &tSteelImgInfoAndData);
///*******************ɾ��Ŀ¼********************///
int  removeDir(const char*  dirPath);
///*******************���м�ͼ��ı߽�����********************///
void PinJieFuncMiddleAllCamImg(cv::Mat ImgPinJie, cv::Mat ImgPinJie_th,vector<RepeatArea> vecRepeatArea, ImgProcessParam tagImgProcessParam,vector<cv::Point> & contours_left, vector<cv::Point> & contours_right,cv::Mat &dst);
///*******************��β������������********************///
int PinJieFuncTailAllCamImg(vector<cv::Mat> vecImgPinJie, vector<RepeatArea> vecRepeatArea, ImgProcessParam tagImgProcessParam,vector<cv::Point> & contours,cv::Mat &dst);
///*******************��ͷ������������********************///
int PinJieFuncHeadAllCamImg(vector<cv::Mat> vecImgPinJie,vector<RepeatArea> vecRepeatArea,ImgProcessParam tagImgProcessParam,vector<cv::Point> & contours,cv::Mat &dst);
///*******************��������ͬһ��ͼ���ƴ��********************///
 void PinJieMuliCamOneImg(vector<cv::Mat> vecImg, vector<RepeatArea> vecRepeatArea, ImgProcessParam tagImgProcessParam,cv::Mat& PinJieImg,cv::Mat& PinJieImgSrc);
///*******************һ������Ķ���ͼ��ƴ��********************///
void PinJieOneCamMuliImg(vector<string> vecImgPath, int iStart, int iEnd, cv::Mat& PinJieImg2D);
///*******************β����������ͼ���ƴ��********************///
void PinJieFuncTailTwoImg(cv::Mat Left, cv::Mat Right, RepeatArea tagRepeatAreaLeft, RepeatArea tagRepeatAreaRight, ImgProcessParam tagImgProcessParam,cv::Mat& PinJieImg, cv::Mat& PinJieImgSrc);
///*******************ͷ����������ͼ���ƴ��********************///
void PinJieFuncHeadTwoImg(cv::Mat Left, cv::Mat Right, RepeatArea tagRepeatAreaLeft, RepeatArea tagRepeatAreaRight, ImgProcessParam tagImgProcessParam,cv::Mat& PinJieImg, cv::Mat& PinJieImgSrc);
///*******************��ƴ��ͼ���Ԥ����********************///
void PinjiePreProcess(cv::Mat src,ImgProcessParam tagImgProcessParam,cv::Mat &dst);
///*******************��ƴ��ͼ���Ԥ����********************///
void Rectify_Image(cv::Mat src,cv::Mat &dst);
///
bool comp_y(const cv::Point &a, const cv::Point &b);
bool comp_x(const cv::Point &a, const cv::Point &b);
#include "stdafx.h"

///*******************保存图像********************///
void Save_Image_OpenCV(CString strPath,cv::Mat src);
///*******************两层文件夹下文件列表********************///
void listFiles(const char * dir, vector<vector<string>> &vecFileList);
///*******************列表文件夹下的img图像********************///
void listImg(const char * dir, vector<string> &vecFileList);
///*******************从img图像获取信息********************///
int GetSteelImg(CString strImgPath,SteelImgInfoAndData &tSteelImgInfoAndData);
///*******************删除目录********************///
int  removeDir(const char*  dirPath);
///*******************找中间图像的边界轮廓********************///
void PinJieFuncMiddleAllCamImg(cv::Mat ImgPinJie, cv::Mat ImgPinJie_th,vector<RepeatArea> vecRepeatArea, ImgProcessParam tagImgProcessParam,vector<cv::Point> & contours_left, vector<cv::Point> & contours_right,cv::Mat &dst);
///*******************找尾部的轮廓曲线********************///
int PinJieFuncTailAllCamImg(vector<cv::Mat> vecImgPinJie, vector<RepeatArea> vecRepeatArea, ImgProcessParam tagImgProcessParam,vector<cv::Point> & contours,cv::Mat &dst);
///*******************找头部的轮廓曲线********************///
int PinJieFuncHeadAllCamImg(vector<cv::Mat> vecImgPinJie,vector<RepeatArea> vecRepeatArea,ImgProcessParam tagImgProcessParam,vector<cv::Point> & contours,cv::Mat &dst);
///*******************多个相机的同一张图像的拼接********************///
 void PinJieMuliCamOneImg(vector<cv::Mat> vecImg, vector<RepeatArea> vecRepeatArea, ImgProcessParam tagImgProcessParam,cv::Mat& PinJieImg,cv::Mat& PinJieImgSrc);
///*******************一个相机的多张图像拼接********************///
void PinJieOneCamMuliImg(vector<string> vecImgPath, int iStart, int iEnd, cv::Mat& PinJieImg2D);
///*******************尾部横向两张图像的拼接********************///
void PinJieFuncTailTwoImg(cv::Mat Left, cv::Mat Right, RepeatArea tagRepeatAreaLeft, RepeatArea tagRepeatAreaRight, ImgProcessParam tagImgProcessParam,cv::Mat& PinJieImg, cv::Mat& PinJieImgSrc);
///*******************头部横向两张图像的拼接********************///
void PinJieFuncHeadTwoImg(cv::Mat Left, cv::Mat Right, RepeatArea tagRepeatAreaLeft, RepeatArea tagRepeatAreaRight, ImgProcessParam tagImgProcessParam,cv::Mat& PinJieImg, cv::Mat& PinJieImgSrc);
///*******************对拼接图像的预处理********************///
void PinjiePreProcess(cv::Mat src,ImgProcessParam tagImgProcessParam,cv::Mat &dst);
///*******************对拼接图像的预处理********************///
void Rectify_Image(cv::Mat src,cv::Mat &dst);
///
bool comp_y(const cv::Point &a, const cv::Point &b);
bool comp_x(const cv::Point &a, const cv::Point &b);
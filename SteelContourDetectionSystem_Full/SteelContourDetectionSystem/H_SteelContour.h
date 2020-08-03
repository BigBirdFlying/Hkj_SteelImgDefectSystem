#pragma once
#include "HDBOper.h"
#include "HFile.h"

typedef struct tagSteelActualEdge
{
	int iLength;
	int iLeft;
	int iRight;
	int iWidth;
	int iMark;
}SteelActualEdge,*PSteelActualEdge;

typedef struct tagSteelActualCutLine
{
	int iHead;
	int iTail;
	int iLeft;
	int iRight;
}SteelActualCutLine,*PSteelActualCutLine;

class H_SteelContour
{
public:
	H_SteelContour(void);
	~H_SteelContour(void);

	bool	m_bIsOnline;
	bool	m_bIsExternalTrigger;
	float	m_fExternalFresY;
	int		m_iCameraNum;
	int		m_iMaxSteelNum;

	float	m_fResolution_X;
	float	m_fResolution_Y;
	long	m_iCurrProcessSteelSequenceNo;

	long	m_iEfeectHeadPix;
	long	m_iEffectTailPix;

	float	m_AcoodRotatingf;
	vector<SteelActualEdge>		m_vecSteelActualEdge;
	vector<SteelActualCutLine>  m_vecSteelActualCutLine;
	
	CamDetectContourSet		m_CamDetectContourSet;
	vector<RepeatArea>		m_vecRepeatArea;
	ImgProcessParam			m_ImgProcessParam;
	CString					m_strContourDataFilePath;	
	CString					m_strDBIP;
	HDBOper					m_hDBOper_SteelRecord;
	vector<int>				m_vecJoinPix;
			
	int		m_AddWidth;
	bool	m_bEnableExternalLength;

	int GetSteelImg(CString strImgPath,SteelImgInfoAndData &tSteelImgInfoAndData);
	bool LoadConfigFile(CString strPath);
	long GetCurrSeqNo();
	long GetCurrFinishSeqNo();
	void ListImgFile(const char * dir, vector<string> &vecFileList);
	void SaveImageOpenCV(CString strPath,cv::Mat src);
	void ImgProcess(cv::Mat src,cv::Mat &dst);
	void ThPreProcess(cv::Mat src,cv::Mat &dst);
	void SingleCamMosaic(vector<vector<string>> vecCamImgList,vector<cv::Mat> &vecCamMosaicImage);
	int  MulitCamMosaic(vector<cv::Mat> vecCamMosaicImage,cv::Mat& TotalMosaic);
	void ExtractContour(long iSeqNo,cv::Mat TotalMosaic,cv::Mat& EffectSteel,cv::Mat& ContourImage,vector<cv::Point>& Contour_Steel_Precision);
	void GetEffectHeadAndTail(vector<cv::Point> Contour_Steel_Precision,int steel_width_pix,int steel_height_pix);
	float GetImgChangeR(int iSteelNo,cv::Mat src);
	void Coord_Rectify(vector<cv::Point> & Contour_Steel_Precision);
	void SaveContourActualCoord(char * curr_path,char* filename,vector<cv::Point> Contour_Steel_Precision,int steel_width);
	void SaveContourActualEdge(char * curr_path,char* filename,vector<cv::Point> Contour_Steel_Precision,int steel_width);
	void SaveContourActualCutLine(char * curr_path,char* filename);
	void SaveContourPixelCoord(char * curr_path,vector<cv::Point> Contour_Steel_Precision,int steel_width);
	void SaveJoinPix(char * curr_path);
	int otsu(Mat image, double e);
};


class Histogram1D {
private:
	int histSize[1];//定义灰度直方图中类型的数目
	float hrange[2];//定义灰度直方图值的范围
	const float * ranges[1];//定义一个指针数组
	int channels[1];//定义检查的通道数
public:
	Histogram1D() {
		histSize[0] = 256;
		hrange[0] = 0;
		hrange[1] = 256;
		ranges[0] = hrange;
		channels[0] = 0;
	}
	//获取灰度直方图各个类型的频率
	Mat getHistogram(const Mat &image) {
		Mat hist;
		calcHist(&image,1,channels,Mat(),hist,1,histSize,ranges);
		return hist;
	}
	//画图函数
	static Mat getImageOfHistogram(const Mat &hist, int zoom)
	{
		double maxval = 0.0;
		double minval = 0.0;
		minMaxLoc(hist, &minval, &maxval, 0, 0);
		int histSize = hist.rows;
		Mat histImg(histSize*zoom, histSize*zoom, CV_8U, Scalar(255));
		int hpt = static_cast<int>(0.9*histSize);
		for (int h = 0;h < histSize;h++)
		{
			float binVal = hist.at<float>(h);
			if (binVal > 0)
			{
				int intensity = (int)(binVal*hpt / maxval);
				line(histImg, Point(h*zoom, histSize*zoom), Point(h*zoom, (histSize - intensity)*zoom), Scalar(0), zoom);
			}
		}
		return histImg;
	}
	Mat getHistogramImage(const Mat &image, int zoom = 1)
	{
		Mat hist = getHistogram(image);
		return getImageOfHistogram(hist, zoom);
	}
};


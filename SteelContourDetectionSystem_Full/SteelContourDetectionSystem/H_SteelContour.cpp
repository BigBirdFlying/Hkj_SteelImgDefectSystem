#include "StdAfx.h"
#include "H_SteelContour.h"

H_SteelContour::H_SteelContour(void)
{
}

H_SteelContour::~H_SteelContour(void)
{
}

//配置文件载入函数
bool H_SteelContour::LoadConfigFile(CString strPath)
{
	//获取应用程序路径
	TCHAR szFileName[MAX_PATH];
	::GetModuleFileName(NULL, szFileName, MAX_PATH);
	CString strFileName = szFileName;
	int nIndex = strFileName.ReverseFind('\\');
	CString strAppPath = strFileName.Left(nIndex);
	CString strConfigName;

	strConfigName.Format(L"%s\\%s",strAppPath,strPath);//SteelContourDetectionSystem.xml
	HFile_xml hFileXml;
	int iJudge=hFileXml.LoadFile(strConfigName);
	if(1==iJudge)
	{
		CString strValue;

		hFileXml.Read(L"//轮廓检测系统程序配置//基本配置//在线模式",strValue);
		if(_ttoi(strValue)>0)
		{
			m_bIsOnline=true;
		}
		else
		{
			m_bIsOnline=false;
		}

		hFileXml.Read(L"//轮廓检测系统程序配置//基本配置//外触发模式//启用",strValue);
		if(_ttoi(strValue)>0)
		{
			m_bIsExternalTrigger=true;
		}
		else
		{
			m_bIsExternalTrigger=false;
		}

		hFileXml.Read(L"//轮廓检测系统程序配置//基本配置//外触发模式//现场纵向精度",strValue);
		m_fExternalFresY=_wtof(strValue)*2;//纵向分辨率是根据像素拉伸到均衡的，这里先采用固定的分辨率拉伸

		hFileXml.Read(L"//轮廓检测系统程序配置//基本配置//数据库IP地址",strValue);
		m_strDBIP.Format(L"%s",strValue);

		hFileXml.Read(L"//轮廓检测系统程序配置//基本配置//轮廓检测相机数量",strValue);
		m_iCameraNum=_ttoi(strValue);

		hFileXml.Read(L"//轮廓检测系统程序配置//基本配置//最大存储钢板数量",strValue);
		m_iMaxSteelNum=_ttoi(strValue);

		hFileXml.Read(L"//轮廓检测系统程序配置//基本配置//相机横向分辨率",strValue);
		m_fResolution_X=_wtof(strValue)*2;//由于在计算过程中会对图像下采样，所以分辨率下降到一倍

		hFileXml.Read(L"//轮廓检测系统程序配置//基本配置//相机纵向分辨率",strValue);
		m_fResolution_Y=0.5*2;//_wtof(strValue)*2;纵向分辨率是根据像素拉伸到均衡的，这里先采用固定的分辨率拉伸
		
		m_CamDetectContourSet.iItemNum=0;
		for(int i=0;i<m_iCameraNum;i++)
		{
			CString strKey;
			strKey.Format(L"//轮廓检测系统程序配置//基本配置//各相机参数设置//相机%d//图像路径",i);
			hFileXml.Read(strKey,strValue);
			m_CamDetectContourSet.Items[m_CamDetectContourSet.iItemNum].strImgPath=L"";
			m_CamDetectContourSet.Items[m_CamDetectContourSet.iItemNum].strImgPath.Format(L"%s",strValue);

			strKey.Format(L"//轮廓检测系统程序配置//基本配置//各相机参数设置//相机%d//相机左重合像素",i);
			hFileXml.Read(strKey,strValue);
			m_CamDetectContourSet.Items[m_CamDetectContourSet.iItemNum].iLeftCoincide=_ttoi(strValue);

			strKey.Format(L"//轮廓检测系统程序配置//基本配置//各相机参数设置//相机%d//相机右重合像素",i);
			hFileXml.Read(strKey,strValue);
			m_CamDetectContourSet.Items[m_CamDetectContourSet.iItemNum].iRightCoincide=_ttoi(strValue);

			strKey.Format(L"//轮廓检测系统程序配置//基本配置//各相机参数设置//相机%d//图像灰度阈值",i);
			hFileXml.Read(strKey,strValue);
			m_CamDetectContourSet.Items[m_CamDetectContourSet.iItemNum].iImgTh=_ttoi(strValue);
			m_ImgProcessParam.iCamGrayTh[i]=_ttoi(strValue);

			m_CamDetectContourSet.iItemNum++;
		}

		for(int i=0;i<m_iCameraNum;i++)
		{
			RepeatArea tagRepeatArea;
			tagRepeatArea.iLeftRepeat = m_CamDetectContourSet.Items[i].iLeftCoincide/2;
			tagRepeatArea.iRightRepeat = m_CamDetectContourSet.Items[i].iRightCoincide/2;
			m_vecRepeatArea.push_back(tagRepeatArea);
		}

		hFileXml.Read(L"//轮廓检测系统程序配置//基本配置//轮廓文件存储路径",strValue);
		m_strContourDataFilePath.Format(L"%s",strValue);

		hFileXml.Read(L"//轮廓检测系统程序配置//高级配置//图像横纵向灰度比阈值",strValue);
		m_ImgProcessParam.fGradXYTh=_wtof(strValue);

		hFileXml.Read(L"//轮廓检测系统程序配置//高级配置//拼接钢板实际坐标旋转因子",strValue);
		m_AcoodRotatingf=_wtof(strValue);
		//5100_-1231_1180_2411_1
		//24000_-1204_1207_2411_1 27 18900 0.0014286

		hFileXml.Read(L"//轮廓检测系统程序配置//特殊配置//钢板中间添加宽度",strValue);
		m_AddWidth=_wtoi(strValue);

		hFileXml.Read(L"//轮廓检测系统程序配置//特殊配置//启用外部精确长度矫正",strValue);
		if(_wtoi(strValue)>0)
		{
			m_bEnableExternalLength=true;
		}
		else
		{
			m_bEnableExternalLength=false;
		}

		return true;
	}
	else
	{
		return false;
	}
}

long H_SteelContour::GetCurrSeqNo()
{
	if(m_hDBOper_SteelRecord.m_bConnected==false)
	{
		m_hDBOper_SteelRecord.ConnectDB(m_strDBIP,L"SteelRecord",L"ARNTUSER",L"ARNTUSER",L"SqlSever");
	}
	long iCurrSteelSequenceNo;
	m_hDBOper_SteelRecord.GetNearSteelNo(iCurrSteelSequenceNo,L"select top 1 * from steel order by SequeceNo desc");
	return iCurrSteelSequenceNo;
}

long H_SteelContour::GetCurrFinishSeqNo()
{
	if(m_hDBOper_SteelRecord.m_bConnected==false)
	{
		m_hDBOper_SteelRecord.ConnectDB(m_strDBIP,L"SteelRecord",L"ARNTUSER",L"ARNTUSER",L"SqlSever");
	}
	long iCurrSteelSequenceNo=-1;
	m_hDBOper_SteelRecord.GetNearFinishSteelNo(iCurrSteelSequenceNo,L"select top 5 * from steel order by SequeceNo desc");
	return iCurrSteelSequenceNo;
}

int H_SteelContour::GetSteelImg(CString strImgPath,SteelImgInfoAndData &tSteelImgInfoAndData)
{
	FILE *fp=NULL;
	USES_CONVERSION;
	const char * imgpath=T2A(strImgPath.GetBuffer(0));
	fp=fopen(imgpath,"rb");

	if(fp==NULL)
	{
		return 0;
	}
	fseek(fp,0,SEEK_END); //定位到文件末 
	long nFileLen = ftell(fp); //文件长度
	if(nFileLen<100)
	{
		return 0;
	}
	char* Data = (char*)malloc(sizeof(char) * nFileLen);
	memset(Data, 0, sizeof(char) * nFileLen);
	fseek(fp,0,SEEK_SET); //定位到文件头
	while(!feof(fp))
	{
		fread(Data,nFileLen,1,fp);
	}
	fclose(fp);
	//
	int iPos,iOffset;
	//获取图像头大小
	iPos=0;
	iOffset=4;
	memcpy(&tSteelImgInfoAndData.iFileHead, Data + iPos, iOffset);
	//获取标识符
	iPos=4;
	iOffset=8;
	memcpy(&tSteelImgInfoAndData.strFask, Data + iPos, iOffset);
	//长度位置
	iPos=12;
	iOffset=4;
	memcpy(&tSteelImgInfoAndData.iLenghtPos, Data + iPos, iOffset);
	//检测速度
	iPos=16;
	iOffset=4;
	memcpy(&tSteelImgInfoAndData.iSpeed, Data + iPos, iOffset);
	//钢板流水号
	iPos=20;
	iOffset=4;
	memcpy(&tSteelImgInfoAndData.iSequnceNo, Data + iPos, iOffset);
	//图像宽度
	iPos=24;
	iOffset=4;
	memcpy(&tSteelImgInfoAndData.iImgWidth, Data + iPos, iOffset);
	//图像高度
	iPos=28;
	iOffset=4;
	memcpy(&tSteelImgInfoAndData.iImgHeight, Data + iPos, iOffset);
	//采集线速率
	iPos=32;
	iOffset=2;
	memcpy(&tSteelImgInfoAndData.iLineSpeed, Data + iPos, iOffset);
	//相机号
	iPos=34;
	iOffset=2;
	memcpy(&tSteelImgInfoAndData.iCameraNo, Data + iPos, iOffset);
	//图像左位置
	iPos=54+14;
	iOffset=2;
	memcpy(&tSteelImgInfoAndData.iLeftPos, Data + iPos, iOffset);
	//图像右位置
	iPos=54+16;
	iOffset=2;
	memcpy(&tSteelImgInfoAndData.iRightPos, Data + iPos, iOffset);
	//图像数据
	cv::Mat src = cv::Mat(tSteelImgInfoAndData.iImgHeight,tSteelImgInfoAndData.iImgWidth,CV_8UC1,Data+tSteelImgInfoAndData.iFileHead);	
	tSteelImgInfoAndData.srcImg=src.clone();
	//
	free(Data);
	return 1;
}

void H_SteelContour::ListImgFile(const char * dir, vector<string> &vecFileList)
{
	char dirRecord[200];
	strcpy_s(dirRecord, dir);
	strcat_s(dirRecord, "Record.dat");
	int iImgNum=0;
	if(access(dirRecord,0)>=0)
	{
		FILE *fRecord;
		fRecord = fopen(dirRecord,"rb");
		if(! fRecord)
		{
			printf("读取文件出错");
		}
		else
		{
			int pos[2];
			fread(pos,sizeof(int),2,fRecord);
			iImgNum=pos[0];
			fclose(fRecord);
		}
	}
	else
	{
		iImgNum=99999;
	}
	
	char dirNew[200];
	strcpy_s(dirNew, dir);
	strcat_s(dirNew, "\\*.img");

	intptr_t handle;
	_finddata_t findData;

	handle = _findfirst(dirNew, &findData);
	if (handle == -1)
	{
		return;
	}

	do
	{
		if (findData.attrib & _A_SUBDIR)
		{
			if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
			{
				continue;
			}		
		}
		char dirImg[200];
		strcpy_s(dirImg, dir);
		//strcat_s(dirImg, "\\");
		strcat_s(dirImg, findData.name);
		vecFileList.push_back(dirImg);

		int iname=atoi(findData.name);
		if(iname>=iImgNum-1)
		{
			break;
		}
	} while (_findnext(handle, &findData) == 0);

	_findclose(handle);
}

void H_SteelContour::SaveImageOpenCV(CString strPath,cv::Mat src) 
{
	string filename=CStringA(strPath);
	char strfilename[128];
	//sprintf(strfilename,"\\C%d__S%d__C%d__I%d__N%d_F%6f.%s",iClass,iSteel,iCamera,iImgIndex,num,fConfidence,m_ConfigInfo_Segmentation.cImgType);
	//filename+=strfilename;
	//cv::imwrite(filename,src);
	IplImage* image=&(IplImage)src;
	const char* p=filename.data();
	cvSaveImage(p,image);
}

void H_SteelContour::SingleCamMosaic(vector<vector<string>> vecCamImgList,vector<cv::Mat> &vecCamMosaicImage) 
{
	for(int i=0;i<vecCamImgList.size();i++)
	{
		vector<cv::Mat> vecImg;
		int iTotalH=0;
		int iTotalW=0;
		for(int j=0;j<vecCamImgList[i].size();j++)
		{
			USES_CONVERSION;
			CString str(vecCamImgList[i][j].c_str());
			SteelImgInfoAndData tagSteelImgInfoAndData;
			GetSteelImg(str,tagSteelImgInfoAndData);
			int iLineSpeed=tagSteelImgInfoAndData.iLineSpeed;
			int iSpeed=tagSteelImgInfoAndData.iSpeed;
			if(iSpeed<100)
			{
				continue;
			}
			float fRidio=(float)iSpeed/iLineSpeed;
			if(true==m_bIsExternalTrigger)//外触发模式下直接赋值
			{
				fRidio=m_fExternalFresY;
			}
			int iResizeH=tagSteelImgInfoAndData.iImgHeight*fRidio*2*0.5;//（tagSteelImgInfoAndData.iImgHeight*fRidio）意思是将像素拉到分辨率为1，乘以2代表将其纵向分辨率提高至0.5
			int iResizeW=tagSteelImgInfoAndData.iImgWidth*0.5;//此处乘以0.5是将图像下采样一倍
			cv::Mat src_resize;
			cv::resize(tagSteelImgInfoAndData.srcImg,src_resize,cv::Size(iResizeW,iResizeH));
			vecImg.push_back(src_resize);
			iTotalH=iTotalH+iResizeH;
			iTotalW=iResizeW;
		}

		cv::Mat cam_mosaic=cv::Mat::zeros(iTotalH,iTotalW,CV_8U);
		int iTempH=0;
		for(int j=0;j<vecImg.size();j++)
		{
			cv::Rect rect(0,iTempH,vecImg[j].cols,vecImg[j].rows);
			vecImg[j].copyTo(cam_mosaic(rect));
			iTempH=iTempH+vecImg[j].rows;
		}
		//CString strSavePath;
		//strSavePath.Format(L"%04d.bmp",i);
		//SaveImageOpenCV(strSavePath,cam_mosaic);
		vecCamMosaicImage.push_back(cam_mosaic);
	}
}


void H_SteelContour::ImgProcess(cv::Mat src,cv::Mat &dst)
{
	cv::threshold(src,dst,20,255,0);
}

/*int H_SteelContour::MulitCamMosaic(vector<cv::Mat> vecCamMosaicImage,cv::Mat& TotalMosaic) 
{
	//图像拼接前的处理
	int iRes=0;
	for(int i=0;i<vecCamMosaicImage.size()-1;i++)
	{
		cv::Mat matLeft=vecCamMosaicImage[i];
		cv::Mat matRight=vecCamMosaicImage[i+1];

		int iMinPix=32;//这里要求重合区域至少在64个像素以上
		
		cv::Rect rect_l(matLeft.cols-m_vecRepeatArea[i].iRightRepeat,0,iMinPix,matLeft.rows);
		cv::Mat roiLeft,roiLeftTh;
		matLeft(rect_l).copyTo(roiLeft);
		ImgProcess( roiLeft,roiLeftTh);
		cv::namedWindow("roiLeftTh",0);
		cv::imshow("roiLeftTh",roiLeftTh);

		cv::Rect rect_r(m_vecRepeatArea[i+1].iLeftRepeat-iMinPix,0,iMinPix,matRight.rows);
		cv::Mat roiRight,roiRightTh;
		matRight(rect_r).copyTo(roiRight);
		ImgProcess(roiRight,roiRightTh);
		cv::namedWindow("roiRightTh",0);
		cv::imshow("roiRightTh",roiRightTh);
		cv::waitKey();

		//左图拼接区域获取
		cv::Mat roiLeftTh_dil;
		int dilation_size_left = 3;
		cv::Mat element_left = getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilation_size_left + 1, 2 * dilation_size_left + 1));
		morphologyEx(roiLeftTh, roiLeftTh_dil, 3, element_left);
		vector<vector<Point> > contours_left;
		vector<Vec4i> hierarchy_left;
		findContours(roiLeftTh_dil, contours_left, hierarchy_left, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		vector<cv::Rect>boundRect_left(contours_left.size());
		int iMaxLeftArea = 0;
		int iStartLeft=-1;
		int iEndLeft=-1;
		for (int j = 0; j< contours_left.size(); j++)
		{
			boundRect_left[j] = boundingRect(cv::Mat(contours_left[j]));
			if (cv::contourArea(contours_left[j])>iMaxLeftArea)
			{
				iMaxLeftArea = cv::contourArea(contours_left[j]);
				iStartLeft=boundRect_left[j].tl().y;
				iEndLeft=boundRect_left[j].br().y;
			}
		}
		//左图重合区域修正
		//for(int j=0;j<iMinPix;j++)
		//{
		//	if(roiLeftTh_dil.at<uchar>(iStartLeft+j,0)>0)
		//	{
		//		iStartLeft=iStartLeft+j;
		//	}
		//	if(roiLeftTh_dil.at<uchar>(iEndLeft-j,0)>0)
		//	{
		//		iEndLeft=iEndLeft-j;
		//	}
		//}

		//右图拼接区域获取
		cv::Mat roiRightTh_dil;
		int dilation_size_right = 3;
		cv::Mat element_right = getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilation_size_right + 1, 2 * dilation_size_right + 1));
		morphologyEx(roiRightTh, roiRightTh_dil, 3, element_right);
		vector<vector<Point> > contours_right;
		vector<Vec4i> hierarchy_right;
		findContours(roiRightTh_dil, contours_right, hierarchy_right, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		vector<cv::Rect>boundRect_right(contours_right.size());
		int iMaxRightArea = 0;
		int iStartRight=-1;
		int iEndRight=-1;
		for (int j = 0; j< contours_right.size(); j++)
		{
			boundRect_right[j] = boundingRect(cv::Mat(contours_right[j]));
			if (cv::contourArea(contours_right[j])>iMaxRightArea)
			{
				iMaxRightArea = cv::contourArea(contours_right[j]);
				iStartRight=boundRect_right[j].tl().y;
				iEndRight=boundRect_right[j].br().y;
			}
		}
		//右图重合区域修正
		//for(int j=0;j<iMinPix;j++)
		//{
		//	if(roiRightTh_dil.at<uchar>(iStartRight+j,iMinPix-1)>0)
		//	{
		//		iStartRight=iStartRight+j;
		//	}
		//	if(roiRightTh_dil.at<uchar>(iEndRight-j,iMinPix-1)>0)
		//	{
		//		iEndRight=iEndRight-j;
		//	}
		//}

		if(iStartLeft>0 && iEndLeft>0 && iStartRight>0 && iEndRight>0)
		{
			//矫正原图使钢板区域大小一致
			int iLenLeft=iEndLeft-iStartLeft;
			int iLenRight=iEndRight-iStartRight;
			float fRidio=(float)iLenLeft/iLenRight;
			
			if(fRidio<0.8 || fRidio>1.2) //针对相机之间找不到重合点的拼接,像窄板子
			{
				iStartLeft=0;
				iStartRight=0;
				iEndLeft=matLeft.rows;
				iEndRight=matRight.rows;
				fRidio=1.0;
				iRes=-1;
			}

			cv::Mat matRight_resize;
			int iResizeH=matRight.rows*fRidio;
			cv::resize(matRight,matRight_resize,cv::Size(matRight.cols,iResizeH));
			vecCamMosaicImage[i+1]=matRight_resize;
			iStartRight=iStartRight*fRidio;
			iEndRight=iEndRight*fRidio;
			//头部坐标平齐
			if(iStartRight<iStartLeft)
			{
				int exp=abs(iStartLeft-iStartRight);
				cv::Mat zero=cv::Mat::zeros(exp+vecCamMosaicImage[i+1].rows,vecCamMosaicImage[i+1].cols,vecCamMosaicImage[i+1].type());
				cv::Rect rect(0,exp,vecCamMosaicImage[i+1].cols,vecCamMosaicImage[i+1].rows);
				vecCamMosaicImage[i+1].copyTo(zero(rect));
				vecCamMosaicImage[i+1]=zero;
			}
			else if(iStartRight>iStartLeft)
			{
				for(int k=0;k<=i;k++)
				{
					int exp=abs(iStartLeft-iStartRight);
					cv::Mat zero=cv::Mat::zeros(exp+vecCamMosaicImage[k].rows,vecCamMosaicImage[k].cols,vecCamMosaicImage[k].type());
					cv::Rect rect(0,exp,vecCamMosaicImage[k].cols,vecCamMosaicImage[k].rows);
					vecCamMosaicImage[k].copyTo(zero(rect));
					vecCamMosaicImage[k]=zero;
				}
			}
		}
	}
	//准备拼接
	int iMaxH=0;
	int iMaxW=0;
	for(int i=0;i<vecCamMosaicImage.size();i++)
	{
		if(vecCamMosaicImage[i].rows>iMaxH)
		{
			iMaxH=vecCamMosaicImage[i].rows;
		}
		iMaxW=iMaxW+vecCamMosaicImage[i].cols-m_vecRepeatArea[i].iLeftRepeat-m_vecRepeatArea[i].iRightRepeat;
	}
	TotalMosaic=cv::Mat::zeros(iMaxH,iMaxW,CV_8U);
	int x=0;
	for(int i=0;i<vecCamMosaicImage.size();i++)
	{
		int w=vecCamMosaicImage[i].cols-m_vecRepeatArea[i].iLeftRepeat-m_vecRepeatArea[i].iRightRepeat;
		cv::Rect rect_src(m_vecRepeatArea[i].iLeftRepeat,0,vecCamMosaicImage[i].cols-m_vecRepeatArea[i].iLeftRepeat-m_vecRepeatArea[i].iRightRepeat,vecCamMosaicImage[i].rows);
		cv::Rect rect_dst(x,0,w,vecCamMosaicImage[i].rows);
		vecCamMosaicImage[i](rect_src).copyTo(TotalMosaic(rect_dst));
		x=x+w;
		m_vecJoinPix.push_back(x);
	}
	m_vecJoinPix.pop_back();
	return iRes;
}*/

int H_SteelContour::MulitCamMosaic(vector<cv::Mat> vecCamMosaicImage,cv::Mat& TotalMosaic) 
{
	//图像拼接前的处理
	int iRes=0;
	for(int i=0;i<vecCamMosaicImage.size()-1;i++)
	{
		cv::Mat matLeft=vecCamMosaicImage[i];
		cv::Mat matRight=vecCamMosaicImage[i+1];

		int iMinPix=32;//这里要求重合区域至少在64个像素以上

		cv::Mat kern_top = (cv::Mat_<float>(3,3) <<  -1,-1,-1,  
													  0, 0, 0,
												      1, 1, 1); 
		cv::Mat kern_bottom = (cv::Mat_<float>(3,3) << 1, 1, 1,  
													   0, 0, 0,
												      -1,-1,-1); 
		
		//左拼接图
		cv::Rect rect_l(matLeft.cols-m_vecRepeatArea[i].iRightRepeat,0,iMinPix,matLeft.rows);
		cv::Mat roiLeft;
		matLeft(rect_l).copyTo(roiLeft);		
		cv::Mat dstF_LeftTop,dstF_LeftBottom;  
		filter2D(roiLeft,dstF_LeftTop,roiLeft.depth(),kern_top);
		filter2D(roiLeft,dstF_LeftBottom,roiLeft.depth(),kern_bottom);

		int iStartLeft=-1;
		int iEndLeft=-1;
		for(int i=64;i<dstF_LeftTop.rows;i++)
		{
			cv::Scalar sum=cv::sum(dstF_LeftTop.row(i));
			double s=sum[0];
			if(s/iMinPix>5)
			{
				cv::Rect rect(0,0,roiLeft.cols,i-32);
				cv::Mat temp;
				roiLeft(rect).copyTo(temp);
				int n=cv::countNonZero(temp);
				if(n==0)
				{
					continue;
				}
				iStartLeft=i;
				break;
			}
		}
		for(int i=dstF_LeftBottom.rows-64;i>0;i--)
		{
			cv::Scalar sum=cv::sum(dstF_LeftBottom.row(i));
			double s=sum[0];
			if(s/iMinPix>5)
			{
				cv::Rect rect(0,i+32,roiLeft.cols,dstF_LeftBottom.rows-(i+32));
				cv::Mat temp;
				roiLeft(rect).copyTo(temp);
				int n=cv::countNonZero(temp);
				if(n==0)
				{
					continue;
				}
				iEndLeft=i;
				break;
			}
		}

		//右拼接图
		cv::Rect rect_r(m_vecRepeatArea[i+1].iLeftRepeat-iMinPix,0,iMinPix,matRight.rows);
		cv::Mat roiRight;
		matRight(rect_r).copyTo(roiRight);
		cv::Mat dstF_RightTop,dstF_RightBottom;  
		filter2D(roiRight,dstF_RightTop,roiRight.depth(),kern_top);
		filter2D(roiRight,dstF_RightBottom,roiRight.depth(),kern_bottom);

		int iStartRight=-1;
		int iEndRight=-1;
		for(int i=0;i<dstF_RightTop.rows;i++)
		{
			cv::Scalar sum=cv::sum(dstF_RightTop.row(i));
			double s=sum[0];
			if(s/iMinPix>5)
			{
				iStartRight=i;
				break;
			}
		}
		for(int i=dstF_RightBottom.rows-1;i>0;i--)
		{
			cv::Scalar sum=cv::sum(dstF_RightBottom.row(i));
			double s=sum[0];
			if(s/iMinPix>5)
			{
				iEndRight=i;
				break;
			}
		}

		if(iStartLeft>0 && iEndLeft>0 && iStartRight>0 && iEndRight>0)
		{
			//矫正原图使钢板区域大小一致
			int iLenLeft=iEndLeft-iStartLeft;
			int iLenRight=iEndRight-iStartRight;
			float fRidio=(float)iLenLeft/iLenRight;
			
			if(fRidio<0.8 || fRidio>1.2) //针对相机之间找不到重合点的拼接,像窄板子
			{
				iStartLeft=0;
				iStartRight=0;
				iEndLeft=matLeft.rows;
				iEndRight=matRight.rows;
				fRidio=1.0;
				iRes=-1;
			}

			cv::Mat matRight_resize;
			int iResizeH=matRight.rows*fRidio;
			cv::resize(matRight,matRight_resize,cv::Size(matRight.cols,iResizeH));
			vecCamMosaicImage[i+1]=matRight_resize;
			iStartRight=iStartRight*fRidio;
			iEndRight=iEndRight*fRidio;
			//头部坐标平齐
			if(iStartRight<iStartLeft)
			{
				int exp=abs(iStartLeft-iStartRight);
				cv::Mat zero=cv::Mat::zeros(exp+vecCamMosaicImage[i+1].rows,vecCamMosaicImage[i+1].cols,vecCamMosaicImage[i+1].type());
				cv::Rect rect(0,exp,vecCamMosaicImage[i+1].cols,vecCamMosaicImage[i+1].rows);
				vecCamMosaicImage[i+1].copyTo(zero(rect));
				vecCamMosaicImage[i+1]=zero;
			}
			else if(iStartRight>iStartLeft)
			{
				for(int k=0;k<=i;k++)
				{
					int exp=abs(iStartLeft-iStartRight);
					cv::Mat zero=cv::Mat::zeros(exp+vecCamMosaicImage[k].rows,vecCamMosaicImage[k].cols,vecCamMosaicImage[k].type());
					cv::Rect rect(0,exp,vecCamMosaicImage[k].cols,vecCamMosaicImage[k].rows);
					vecCamMosaicImage[k].copyTo(zero(rect));
					vecCamMosaicImage[k]=zero;
				}
			}
		}
	}
	//准备拼接
	int iMaxH=0;
	int iMaxW=0;
	for(int i=0;i<vecCamMosaicImage.size();i++)
	{
		if(vecCamMosaicImage[i].rows>iMaxH)
		{
			iMaxH=vecCamMosaicImage[i].rows;
		}
		iMaxW=iMaxW+vecCamMosaicImage[i].cols-m_vecRepeatArea[i].iLeftRepeat-m_vecRepeatArea[i].iRightRepeat;
	}
	TotalMosaic=cv::Mat::zeros(iMaxH,iMaxW,CV_8U);
	int x=0;
	for(int i=0;i<vecCamMosaicImage.size();i++)
	{
		int w=vecCamMosaicImage[i].cols-m_vecRepeatArea[i].iLeftRepeat-m_vecRepeatArea[i].iRightRepeat;
		cv::Rect rect_src(m_vecRepeatArea[i].iLeftRepeat,0,vecCamMosaicImage[i].cols-m_vecRepeatArea[i].iLeftRepeat-m_vecRepeatArea[i].iRightRepeat,vecCamMosaicImage[i].rows);
		cv::Rect rect_dst(x,0,w,vecCamMosaicImage[i].rows);
		vecCamMosaicImage[i](rect_src).copyTo(TotalMosaic(rect_dst));
		x=x+w;
		m_vecJoinPix.push_back(x);
	}
	m_vecJoinPix.pop_back();
	return iRes;
}

int H_SteelContour::otsu(Mat image, double e)
{
	int width = image.cols;
	int height = image.rows;
	int x = 0, y = 0;
	int pixelCount[256];
	float pixelPro[256];
	int i, j, pixelSum = width * height, threshold = 0;
 
	uchar* data = (uchar*)image.data;
	for (i = 0; i < 256; i++)
	{
		pixelCount[i] = 0;
		pixelPro[i] = 0;
	}
 
	for (i = y; i < height; i++)
	{
		for (j = x; j<width;j++)
		{
			pixelCount[data[i * image.step+ j]]++;
		}
	}
	for (i = 0; i < 256; i++)
	{
		pixelPro[i] = (float)(pixelCount[i]) / (float)(pixelSum);
	}

	double w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
	for (i = 0; i < 256; i++)
	{
		w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
		for (j = 0; j < 256; j++)
		{
			if (j <= i) //背景部分  
			{  
				w0 += pixelPro[j];
				u0tmp += j * pixelPro[j];
			}
			else       //前景部分  
			{
				w1 += pixelPro[j];
				u1tmp += j * pixelPro[j];
			}
		}
 
		u0 = u0tmp / w0;        //第一类的平均灰度  
		u1 = u1tmp / w1;        //第二类的平均灰度  
		u = (u0tmp + u1tmp)/(w0+w1);      //整幅图像的平均灰度  	
		//deltaTmp = w0 * (u0 - u)*(u0 - u) + w1 * (u1 - u)*(u1 - u);//计算类间方差  
		deltaTmp = pow(w0,e) * (u0 - u)*(u0 - u) + pow(w1,e) * (u1 - u)*(u1 - u);
		if (deltaTmp > deltaMax)
		{
			deltaMax = deltaTmp;
			threshold = i;
		}
	} 
	return threshold;
}


void H_SteelContour::ThPreProcess(cv::Mat src,cv::Mat &dst)
{
	//获取分割阈值
	//Histogram1D h;
	//Mat hist = h.getHistogram(src_t);
	//Mat img = h.getHistogramImage(src_t);
	int th=255;
	int inv=4;
	for(int i=1;i<inv;i++) //针对每个区域计算阈值
	{
		//cv::Rect rect_t(0,src.rows*i/inv-20,src.cols,40);
		//cv::Mat src_t;
		//src(rect_t).copyTo(src_t);
		//for(float j=0.5;j<0.85;j+=0.1)
		//{
		//	int t=otsu(src_t,j);
		//	if(t<th)
		//	{
		//		th=t;
		//	}
		//}
		cv::Rect rect_l(0,src.rows*i/inv-20,src.cols/2,40);
		cv::Mat src_l;
		src(rect_l).copyTo(src_l);
		for(float j=0.5;j<1.0;j+=0.05)
		{
			int t=otsu(src_l,j);
			if(t<th)
			{
				th=t;
			}
		}
		
		cv::Rect rect_r(src.cols/2,src.rows*i/inv-20,src.cols/2,40);
		cv::Mat src_r;
		src(rect_r).copyTo(src_r);
		for(float j=0.5;j<1.0;j+=0.05)
		{
			int t=otsu(src_r,j);
			if(t<th)
			{
				th=t;
			}
		}
	}
 	cv::threshold(src,dst,min(40,max(10,th)),255,0);//40

	//
	int x_inv=256;
	int x_stride=x_inv/2;
	int y_inv=8;
	int line_num=min(10000,dst.rows*0.5);
	for(long i=0;i<line_num/y_inv;i++)
	{
		long iHead=0;
		for(int j=0;j<(dst.cols/x_stride)-2;j++)
		{
			cv::Rect rect(j*x_stride,iHead+i*y_inv,x_inv,y_inv);
			cv::Mat temp;
			dst(rect).copyTo(temp);
			int iLeft=0;
			int iRight=0;
			for(int k=0;k<temp.rows;k++)
			{
				for(int l=0;l<10;l++)
				{
					if(temp.at<uchar>(k,l)>0)
					{
						iLeft++;
					}
				}
				for(int l=temp.cols-1;l>temp.cols-10;l--)
				{
					if(temp.at<uchar>(k,l)>0)
					{
						iRight++;
					}
				}
			}
			
			if(iLeft==0 && iRight==0)
			{
				cv::Mat zero=cv::Mat::zeros(temp.rows,temp.cols,temp.type());
				zero.copyTo(dst(rect));
			}
		}
		iHead=dst.rows-1-line_num;
		for(int j=0;j<(dst.cols/x_stride)-2;j++)
		{
			cv::Rect rect(j*x_stride,iHead+i*y_inv,x_inv,y_inv);
			cv::Mat temp;
			dst(rect).copyTo(temp);
			int iLeft=0;
			int iRight=0;
			for(int k=0;k<temp.rows;k++)
			{
				for(int l=0;l<10;l++)
				{
					if(temp.at<uchar>(k,l)>0)
					{
						iLeft++;
					}
				}
				for(int l=temp.cols-1;l>temp.cols-10;l--)
				{
					if(temp.at<uchar>(k,l)>0)
					{
						iRight++;
					}
				}
			}
			
			if(iLeft==0 && iRight==0)
			{
				cv::Mat zero=cv::Mat::zeros(temp.rows,temp.cols,temp.type());
				zero.copyTo(dst(rect));
			}
		}
	}
}

void H_SteelContour::ExtractContour(long iSeqNo,cv::Mat TotalMosaic,cv::Mat& EffectSteel,cv::Mat& ContourImage,vector<cv::Point>& Contour_Steel_Precision)
{
	cv::Mat TotalMosaicTh;
	ThPreProcess( TotalMosaic,TotalMosaicTh);
	
	cv::Mat TotalMosaicTh_dil;
	int dilation_size = 5;
	cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1));
	morphologyEx(TotalMosaicTh, TotalMosaicTh_dil, 3, element);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(TotalMosaicTh_dil, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<cv::Rect>boundRect(contours.size());
	int iMaxArea = 0;
	int iMaxAreaIndex=-1;
	long iLeft,iRight,iTop,iBottom;
	for (int i = 0; i< contours.size(); i++)
	{
		boundRect[i] = boundingRect(cv::Mat(contours[i]));

		if (cv::contourArea(contours[i])>iMaxArea)
		{
			iMaxArea = cv::contourArea(contours[i]);
			iMaxAreaIndex=i;
			iLeft=boundRect[i].tl().x;
			iRight=boundRect[i].br().x;
			iTop=boundRect[i].tl().y;
			iBottom=boundRect[i].br().y;
		}
	}
	cv::Mat EffectSteelTh;
	if(iMaxAreaIndex>=0)
	{
		for(int i=0;i<m_vecJoinPix.size();i++)
		{
			m_vecJoinPix[i]=m_vecJoinPix[i]-iLeft;
		}
		cv::Rect rect(iLeft,iTop,iRight-iLeft,iBottom-iTop);
		TotalMosaic(rect).copyTo(EffectSteel);
		TotalMosaicTh_dil(rect).copyTo(EffectSteelTh);

		//得到中间部分的头尾距离与编码器距离对应，将图象纵向分辨率拉到1
		if(m_bEnableExternalLength)
		{
			float e=GetImgChangeR(iSeqNo,EffectSteel);
			if(e>0)
			{
				int iChangeLen=EffectSteel.rows*e;
				cv::resize(EffectSteel,EffectSteel,cv::Size(EffectSteel.cols,iChangeLen));
				cv::resize(EffectSteelTh,EffectSteelTh,cv::Size(EffectSteelTh.cols,iChangeLen));
			}
		}

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		findContours(EffectSteelTh, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		int iMaxArea = 0;
		int iMaxAreaIndex=-1;
		for (int i = 0; i< contours.size(); i++)
		{
			if (cv::contourArea(contours[i])>iMaxArea)
			{
				iMaxArea = cv::contourArea(contours[i]);
				iMaxAreaIndex=i;
			}
		}

		//cv::cvtColor(EffectSteel,ContourImage,cv::COLOR_GRAY2BGR);
		//cv::drawContours(ContourImage,contours,iMaxAreaIndex,cv::Scalar(255,0,0),5);
		cv::resize(EffectSteel,ContourImage,cv::Size(4096,EffectSteel.rows));

		//进行轮廓的点扩充
		for (int i = 0; i < contours[iMaxAreaIndex].size()-1; i++)
		{
			long x0=contours[iMaxAreaIndex][i].x;
			long y0=contours[iMaxAreaIndex][i].y;
			long x1=contours[iMaxAreaIndex][i+1].x;
			long y1=contours[iMaxAreaIndex][i+1].y;

			double k=(double)(x1-x0)/(double)(y1-y0);
			if(y0<y1)
			{
				for(long y=y0;y<y1;y++)
				{
					long x=k*(y-y0)+x0;
					cv::Point p;
					p.x=x;
					p.y=y;
					Contour_Steel_Precision.push_back(p);
				}
			}
			else
			{
				for(long y=y0;y>y1;y--)
				{
					long x=k*(y-y0)+x0;
					cv::Point p;
					p.x=x;
					p.y=y;
					Contour_Steel_Precision.push_back(p);
				}
			}		
		}	
	}
}

void H_SteelContour::GetEffectHeadAndTail(vector<cv::Point> Contour_Steel_Precision,int steel_width_pix,int steel_height_pix)
{
	m_iEfeectHeadPix=0;
	m_iEffectTailPix=steel_height_pix;
	for(int i=0;i<Contour_Steel_Precision.size();i++)
	{
		int x=Contour_Steel_Precision[i].x;
		int y=Contour_Steel_Precision[i].y;
		if(x>steel_width_pix*0.25 && x<steel_width_pix*0.75)
		{
			if(y<steel_height_pix*0.5)
			{
				if(y>m_iEfeectHeadPix)
				{
					m_iEfeectHeadPix=y;
				}
			}
			else
			{
				if(y<m_iEffectTailPix)
				{
					m_iEffectTailPix=y;
				}
			}
		}
	}
}

float H_SteelContour::GetImgChangeR(int iSteelNo,cv::Mat src)
{
	if(m_hDBOper_SteelRecord.m_bConnected==false)
	{
		m_hDBOper_SteelRecord.ConnectDB(m_strDBIP,L"SteelRecord",L"ARNTUSER",L"ARNTUSER",L"SqlSever");
	}
	long iLength=0;
	m_hDBOper_SteelRecord.GetSteelLength(iSteelNo,iLength,L"select top 1 * from SteelLength order by SeqNo desc");
	if(iLength>0)
	{
		float fResolution_Y=(double)iLength/src.rows;
		return fResolution_Y;
	}
	else
	{
		return -1;
	}
}


void H_SteelContour::Coord_Rectify(vector<cv::Point> & Contour_Steel_Precision)
{
	double sum_x2 = 0.0;
    double sum_y  = 0.0;
    double sum_x  = 0.0;
    double sum_xy = 0.0;
	int num=0;
	for (int i = 0; i < m_vecSteelActualEdge.size(); i++)
	{		
		long x=m_vecSteelActualEdge[i].iLength;
		long y=(m_vecSteelActualEdge[i].iLeft+m_vecSteelActualEdge[i].iRight)/2;
		if(x>m_vecSteelActualEdge.size()*0.1 && x<m_vecSteelActualEdge.size()*0.9)
		{		
			sum_x2 += x*x;
            sum_y  += y;
            sum_x  += x;
            sum_xy += x*y;
			num=num+1;
		}
	}

	float k=(num*sum_xy - sum_x*sum_y)/(num*sum_x2 - sum_x*sum_x);
	//float k=m_AcoodRotatingf;//(float)(y1-y0)/(x1-x0);
	double a=tanh(k);
	for (int i = 0; i < Contour_Steel_Precision.size(); i++)
	{
		int iSet=Contour_Steel_Precision[i].y*tan(a);
		Contour_Steel_Precision[i].x=Contour_Steel_Precision[i].x-iSet;
	}		
}

void H_SteelContour::SaveContourActualCoord(char * curr_path,char* filename,vector<cv::Point> Contour_Steel_Precision,int steel_width)
{
	char cstrPath_Contour_Steel_Precision[256];
	strcpy(cstrPath_Contour_Steel_Precision,curr_path);
	//strcat(cstrPath_Contour_Steel_Precision,"ContourActualCoord.txt");
	strcat(cstrPath_Contour_Steel_Precision,filename);
	ofstream Contour_Steel_Precision_Out(cstrPath_Contour_Steel_Precision);
	if (Contour_Steel_Precision_Out.is_open())
	{
		for (int i = 0; i < Contour_Steel_Precision.size(); i++)
		{
			long x=(Contour_Steel_Precision[i].x - steel_width*0.5) * m_fResolution_X;
			long y=Contour_Steel_Precision[i].y * m_fResolution_Y;
			Contour_Steel_Precision_Out << x << "_" << y << "\n";
		}		
		Contour_Steel_Precision_Out.close();
	}
}

void H_SteelContour::SaveContourActualEdge(char * curr_path,char* filename,vector<cv::Point> Contour_Steel_Precision,int steel_width)
{
	m_vecSteelActualEdge.clear();
	m_vecSteelActualEdge.swap(vector<SteelActualEdge>(m_vecSteelActualEdge));

	char cstrPath_Contour_Steel_Precision[256];
	strcpy(cstrPath_Contour_Steel_Precision,curr_path);
	//strcat(cstrPath_Contour_Steel_Precision,"ContourActualEdge.txt");
	strcat(cstrPath_Contour_Steel_Precision,filename);
	ofstream Contour_Steel_Precision_Out(cstrPath_Contour_Steel_Precision);
	
	long x_min=9999999;
	long x_max=0;
	long y_old=0;
	if (Contour_Steel_Precision_Out.is_open())
	{
		for (long i = 0; i < Contour_Steel_Precision.size(); i++)
		{
			//Contour_Steel_Precision_Out << Contour_Steel_Precision[i].y << "_" << Contour_Steel_Precision[i].x << "\n";
			long y=Contour_Steel_Precision[i].y;
			if(y!=y_old)
			{
				if(x_min<=x_max)
				{
					long a_x_min=(x_min -(int)(steel_width*0.5)) * m_fResolution_X;
					long a_x_max=(x_max -(int)(steel_width*0.5)) * m_fResolution_X;
					long a_y=y_old*m_fResolution_Y;
					int mask=1;
					if(y<m_iEfeectHeadPix || y>m_iEffectTailPix)
					{
						mask=0;
					}
					SteelActualEdge tagSteelActualEdge;
					tagSteelActualEdge.iLength=a_y;
					tagSteelActualEdge.iLeft=a_x_min;
					tagSteelActualEdge.iRight=a_x_max;
					tagSteelActualEdge.iWidth=a_x_max-a_x_min;
					tagSteelActualEdge.iMark=mask;
					m_vecSteelActualEdge.push_back(tagSteelActualEdge);
					Contour_Steel_Precision_Out << a_y << "_" << a_x_min << "_" << a_x_max << "_" << (a_x_max-a_x_min) << "_" << mask << "\n";
				}
				y_old=y;
				x_min=9999999;
				x_max=0;
			}

			long x=Contour_Steel_Precision[i].x;
			if (x>x_max)
			{
				x_max=x;
			}
			if(x<x_min)
			{
				x_min=x;
			}

			if(i==Contour_Steel_Precision.size()-1)
			{
				if(x_min<=x_max)
				{
					long a_x_min=(x_min -(int)(steel_width*0.5)) * m_fResolution_X;
					long a_x_max=(x_max -(int)(steel_width*0.5)) * m_fResolution_X;
					long a_y=y*m_fResolution_Y;
					int mask=1;
					if(y<m_iEfeectHeadPix || y>m_iEffectTailPix)
					{
						mask=0;
					}
					SteelActualEdge tagSteelActualEdge;
					tagSteelActualEdge.iLength=a_y;
					tagSteelActualEdge.iLeft=a_x_min;
					tagSteelActualEdge.iRight=a_x_max;
					tagSteelActualEdge.iWidth=a_x_max-a_x_min;
					tagSteelActualEdge.iMark=mask;
					m_vecSteelActualEdge.push_back(tagSteelActualEdge);
					Contour_Steel_Precision_Out << a_y << "_" << a_x_min << "_" << a_x_max << "_" << (a_x_max-a_x_min) << "_" << mask << "\n";
				}				
			}			
		}		
		Contour_Steel_Precision_Out.close();
	}
}

void H_SteelContour::SaveContourActualCutLine(char * curr_path,char* filename)
{
	m_vecSteelActualCutLine.clear();
	m_vecSteelActualCutLine.swap(vector<SteelActualCutLine>(m_vecSteelActualCutLine));
	
	char cstrPath_Contour_Steel_Precision[256];
	strcpy(cstrPath_Contour_Steel_Precision,curr_path);
	//strcat(cstrPath_Contour_Steel_Precision,"ContourActualCutLine.txt");
	strcat(cstrPath_Contour_Steel_Precision,filename);
	ofstream Contour_Steel_Precision_Out(cstrPath_Contour_Steel_Precision);
	
	if (Contour_Steel_Precision_Out.is_open())
	{
		int iMiddleBaseWidth=0;
		int iMiddleMaxLeft=-99999;
		int iMiddleMinRight=99999;
		int iSingleEdgeOffSet=100;
		long steel_length=m_vecSteelActualEdge[ m_vecSteelActualEdge.size()-1].iLength;
		for (int i = 0; i < m_vecSteelActualEdge.size(); i++)
		{		
			long y=m_vecSteelActualEdge[i].iLength;
			if(y>steel_length*0.25 && y<steel_length*0.75)
			{
				if(m_vecSteelActualEdge[i].iLeft>iMiddleMaxLeft)
				{
					iMiddleMaxLeft=m_vecSteelActualEdge[i].iLeft;
				}
				if(m_vecSteelActualEdge[i].iRight<iMiddleMinRight)
				{
					iMiddleMinRight=m_vecSteelActualEdge[i].iRight;
				}
			}
		}
		iMiddleBaseWidth=iMiddleMinRight-iMiddleMaxLeft;

		for(int i=0;i<=iSingleEdgeOffSet;i++)
		{		
			int iEffectMiddleLeft=iMiddleMaxLeft+i;
			int iEffectMiddleRight=iMiddleMinRight-i;

			int iEffectMinTop=0;
			int iEffectMaxBottom=999999;
			for (int j = m_vecSteelActualEdge.size()*0.5; j >= 0; j--)
			{
				iEffectMinTop=j;
				if(m_vecSteelActualEdge[j].iLeft>iEffectMiddleLeft || m_vecSteelActualEdge[j].iRight<iEffectMiddleRight || m_vecSteelActualEdge[j].iMark==0)
				{
					break;
				}
			}
			for (int j = m_vecSteelActualEdge.size()*0.5; j < m_vecSteelActualEdge.size(); j++)
			{
				iEffectMaxBottom=j;
				if(m_vecSteelActualEdge[j].iLeft>iEffectMiddleLeft || m_vecSteelActualEdge[j].iRight<iEffectMiddleRight || m_vecSteelActualEdge[j].iMark==0)
				{
					break;
				}
			}
			SteelActualCutLine tagSteelActualCutLine;
			tagSteelActualCutLine.iHead=iEffectMinTop;
			tagSteelActualCutLine.iTail=iEffectMaxBottom;
			tagSteelActualCutLine.iLeft=iEffectMiddleLeft;
			tagSteelActualCutLine.iRight=iEffectMiddleRight;
			m_vecSteelActualCutLine.push_back(tagSteelActualCutLine);
			Contour_Steel_Precision_Out << iEffectMinTop << "_" << iEffectMaxBottom<< "_" << iEffectMiddleLeft << "_" << iEffectMiddleRight << "\n";		
		}		
		Contour_Steel_Precision_Out.close();
	}
}


void H_SteelContour::SaveContourPixelCoord(char * curr_path,vector<cv::Point> Contour_Steel_Precision,int steel_width)
{
	char cstrPath_Contour_Steel_Precision[256];
	strcpy(cstrPath_Contour_Steel_Precision,curr_path);
	strcat(cstrPath_Contour_Steel_Precision,"ContourPixelCoord.txt");
	ofstream Contour_Steel_Precision_Out(cstrPath_Contour_Steel_Precision);
	long x_min=9999999;
	long x_max=0;
	long y_old=0;
	if (Contour_Steel_Precision_Out.is_open())
	{
		for (long i = 0; i < Contour_Steel_Precision.size(); i++)
		{
			//Contour_Steel_Precision_Out << Contour_Steel_Precision[i].y << "_" << Contour_Steel_Precision[i].x << "\n";
			long y=Contour_Steel_Precision[i].y;
			if(y!=y_old)
			{
				if(x_min<=x_max)
				{
					Contour_Steel_Precision_Out << y_old << "_" << x_min-(int)(steel_width*0.5) << "_" << x_max-(int)(steel_width*0.5) << "_" << (x_max-x_min) << "\n";
				}
				y_old=y;
				x_min=9999999;
				x_max=0;
			}

			long x=Contour_Steel_Precision[i].x;
			if (x>x_max)
			{
				x_max=x;
			}
			if(x<x_min)
			{
				x_min=x;
			}

			if(i==Contour_Steel_Precision.size()-1)
			{
				if(x_min<=x_max)
				{
					Contour_Steel_Precision_Out << y << "_" << x_min -(int)(steel_width*0.5)<< "_" << x_max -(int)(steel_width*0.5) << "_" << (x_max-x_min) << "\n";
				}				
			}			
		}		
		Contour_Steel_Precision_Out.close();
	}
}

void H_SteelContour::SaveJoinPix(char * curr_path)
{
	char cstrPath[256];
	strcpy(cstrPath,curr_path);
	strcat(cstrPath,"ContourJoinPix.txt");
	ofstream f(cstrPath);
	if (f.is_open())
	{
		for (int i = 0; i < m_vecJoinPix.size(); i++)
		{
			int x=m_vecJoinPix[i];
			f << x << "\n";
		}		
		f.close();
	}
}
// SteelContourDetectionSystem.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "HFile.h"
#include "HDBOper.h"
#include "CommonFunction.h"
#include "H_SteelContour.h"
//

///*******************主函数********************///
int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);  
    DWORD mode;  
    GetConsoleMode(hStdin, &mode);  
    mode &= ~ENABLE_QUICK_EDIT_MODE;  //移除快速编辑模式
    mode &= ~ENABLE_INSERT_MODE;      //移除插入模式
    mode &= ~ENABLE_MOUSE_INPUT;
    SetConsoleMode(hStdin, mode); 
	
	HWND hwnd=GetForegroundWindow();
	SetConsoleTitle(L"BKVision轮廓检测程序-Version2.0");
	//SetWindowTextA(hwnd,"BKVision轮廓检测程序-Version1.0");
	int width=GetSystemMetrics(SM_CXSCREEN);
	int height=GetSystemMetrics(SM_CYSCREEN);
	char setting[30];
	sprintf(setting,"mode con:cols=%d lines=%d",width,height);
	//system(setting);
	//MoveWindow(hwnd,-200,-200,width,height,true);
	SetWindowPos(hwnd,HWND_TOP,0,0,width,height*0.95,0);
	
	system("color 3D");
	HMENU hMenu;
	hMenu=GetSystemMenu(hwnd,false);
	EnableMenuItem(hMenu,SC_CLOSE,MF_GRAYED|MF_BYCOMMAND);

	//HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);
	//SMALL_RECT rc;
	//rc.Left=-200;
	//rc.Right=width;
	//rc.Top=0;
	//rc.Bottom=height;
	//SetConsoleWindowInfo(hOut,TRUE,&rc);
	//CloseHandle(hOut);

	TCHAR szAppName[MAX_PATH];
	::GetModuleFileName(NULL, szAppName, MAX_PATH);
	CString strAppName = szAppName;
	int nIndex = strAppName.ReverseFind('\\');
	CString strAppPath = strAppName.Left(nIndex);
	HFile_log					m_LogOper;
	m_LogOper.SetLogFilePath(strAppPath);

	///************************************************加载配置文件***************************************///
	m_LogOper.AddLogInfo(L"Normal>> 轮廓检测程序开始运行！");
	H_SteelContour h_SteelContour;
	m_LogOper.AddLogInfo(L"Normal>> 开始加载程序配置文件！");	
	bool bIsSuccess=h_SteelContour.LoadConfigFile(L"SteelContourDetectionSystem.xml");
	if(true==bIsSuccess)
	{
		m_LogOper.AddLogInfo(L"Normal>> 完成配置文件加载成功！");
	}
	else
	{
		m_LogOper.AddLogInfo(L"Error>> 配置文件SteelContourDetectionSystem.xml加载失败！");
		getchar();
	}
	///************************************************进入循环检测轮廓***************************************///
	m_LogOper.AddLogInfo(L"Normal>> 进入检测轮廓程序主循环！");
	long iCurrProcessSteelSequenceNo=0;
	long iLastProcessSteelSequenceNo=0;
	while(true)
	{	
		long iCurrSteelSequenceNo=0;
		char strCurrSteelSequenceNo[16];
		vector<char*> vecCamPath;
		char CamPath[16][128];
		///1、得到待处理图像的基本路径
		if(true==h_SteelContour.m_bIsOnline)
		{
			iLastProcessSteelSequenceNo=iCurrProcessSteelSequenceNo;
			do
			{
				iCurrSteelSequenceNo=h_SteelContour.GetCurrFinishSeqNo();//得到最近一块已完成的钢板
				if(iCurrSteelSequenceNo>0 && iCurrSteelSequenceNo!=iLastProcessSteelSequenceNo)
				{
					break;
				}
				else
				{
					m_LogOper.AddLogInfo(L"Normal>> 在线模式等待获取最新完成的钢板流水号！");
				}
				Sleep(5000);
			}
			while(true);
			//while(0==iCurrSteelSequenceNo);

			//if(iCurrProcessSteelSequenceNo == iCurrSteelSequenceNo-h_SteelContour.m_iDelayNum)
			//{
			//	m_LogOper.AddLogInfo(L"Normal>> 在线模式等待获取最新需要处理的钢板流水号！");
			//	Sleep(5000);
			//	continue;
			//}

			//iCurrProcessSteelSequenceNo=iCurrSteelSequenceNo-h_SteelContour.m_iDelayNum;
			
			iCurrProcessSteelSequenceNo=iCurrSteelSequenceNo;
			sprintf(strCurrSteelSequenceNo,"%d\\",iCurrProcessSteelSequenceNo);

			CString strInfo=L"";
			strInfo.Format(L"Normal>> 开始流水号<<<<--------------------%d-------------------->>>>的钢板轮廓检测！",iCurrProcessSteelSequenceNo);
			m_LogOper.AddLogInfo(strInfo);	
			

			char strSequenceNo[16];
			sprintf(strSequenceNo,"%06d\\",iCurrProcessSteelSequenceNo % h_SteelContour.m_iMaxSteelNum);

			for(int i=0;i<h_SteelContour.m_iCameraNum;i++)
			{
				int n = h_SteelContour.m_CamDetectContourSet.Items[i].strImgPath.GetLength();
				int len = WideCharToMultiByte(CP_ACP,0,h_SteelContour.m_CamDetectContourSet.Items[i].strImgPath,n,NULL,0,NULL,NULL);
				char * CamPath_S = new char[len+1];   //以字节为单位
				WideCharToMultiByte(CP_ACP,0,h_SteelContour.m_CamDetectContourSet.Items[i].strImgPath,n,CamPath_S,len,NULL,NULL);
				CamPath_S[len] = '\0';   //多字节字符以'/0'结束
				
				strcpy(CamPath[i],CamPath_S);
				strcat(CamPath[i],strSequenceNo);
				delete[] CamPath_S;	
				vecCamPath.push_back(CamPath[i]);
			}
		}
		else
		{
			m_LogOper.AddLogInfo(L"Normal>> 离线模式！");
			//char* CamPath1="C:\\Users\\GDDD\\Desktop\\aaaa\\DetectImgSrc1\\000001\\";
			//char* CamPath2="C:\\Users\\GDDD\\Desktop\\aaaa\\DetectImgSrc2\\000001\\";
			//char* CamPath3="C:\\Users\\GDDD\\Desktop\\aaaa\\DetectImgSrc3\\000001\\";
			//char* CamPath4="C:\\Users\\GDDD\\Desktop\\aaaa\\DetectImgSrc4\\000001\\";
			char* CamPath1="E:\\SampleImg\\宽厚板轮廓样本\\SrcImg2\\1\\";
			char* CamPath2="E:\\SampleImg\\宽厚板轮廓样本\\SrcImg2\\2\\";
			char* CamPath3="E:\\SampleImg\\宽厚板轮廓样本\\SrcImg2\\3\\";
			char* CamPath4="E:\\SampleImg\\宽厚板轮廓样本\\SrcImg2\\4\\";
			vecCamPath.push_back(CamPath1);
			vecCamPath.push_back(CamPath2);
			vecCamPath.push_back(CamPath3);
			vecCamPath.push_back(CamPath4);
			sprintf(strCurrSteelSequenceNo,"%d\\",0);
		}
		///2、创建轮廓数据存储目录
		m_LogOper.AddLogInfo(L"Normal>> 开始创建当前流水号轮廓存储目录！");	
		int n = h_SteelContour.m_strContourDataFilePath.GetLength();
		int len = WideCharToMultiByte(CP_ACP,0,h_SteelContour.m_strContourDataFilePath,n,NULL,0,NULL,NULL);
		char * cstrContourDataFilePath = new char[len+1];   
		WideCharToMultiByte(CP_ACP,0,h_SteelContour.m_strContourDataFilePath,n,cstrContourDataFilePath,len,NULL,NULL);
		cstrContourDataFilePath[len] = '\0';   
		CString strCurrSteelContourDataFilePath=L"";
		char cstrCurrSteelContourDataFilePath[256];//轮廓存储的目录
		if(access(cstrContourDataFilePath,0)>=0)
		{
			strCurrSteelContourDataFilePath.Format(L"%s\\%d",h_SteelContour.m_strContourDataFilePath,iCurrProcessSteelSequenceNo);
			bool flag = CreateDirectory(strCurrSteelContourDataFilePath,NULL);

			strcpy(cstrCurrSteelContourDataFilePath,cstrContourDataFilePath);
			strcat(cstrCurrSteelContourDataFilePath,strCurrSteelSequenceNo);
		}
		else
		{
			m_LogOper.AddLogInfo(L"Error>> 轮廓存储的主目录不存在！");
			delete[] cstrContourDataFilePath;
			continue;
			//getchar();
		}
		delete[] cstrContourDataFilePath;
		m_LogOper.AddLogInfo(L"Normal>> 完成创建当前流水号轮廓存储目录！");	
		///向量全部置位
		h_SteelContour.m_vecJoinPix.clear();
		h_SteelContour.m_vecJoinPix.swap(vector<int>(h_SteelContour.m_vecJoinPix));
		h_SteelContour.m_vecSteelActualEdge.clear();
		h_SteelContour.m_vecSteelActualEdge.swap(vector<SteelActualEdge>(h_SteelContour.m_vecSteelActualEdge));
		h_SteelContour.m_vecSteelActualCutLine.clear();
		h_SteelContour.m_vecSteelActualCutLine.swap(vector<SteelActualCutLine>(h_SteelContour.m_vecSteelActualCutLine));
		///3、单相机图像拼接
		m_LogOper.AddLogInfo(L"Normal>> 开始进行单相机图像拼接！");	
		bool bIsSatisfy=true;
		vector<vector<string>> vecCamImgList;
		for(int i=0;i<h_SteelContour.m_iCameraNum;i++)
		{
			vector<string> CamImgList;
			h_SteelContour.ListImgFile(vecCamPath[i],CamImgList);
			if(CamImgList.size()<=10 ||CamImgList.size()>200)
			{
				m_LogOper.AddLogInfo(L"Warning>> 相机图像数量不符合要求！");
				bIsSatisfy=false;
			}
			vecCamImgList.push_back(CamImgList);
		}
		if(false==bIsSatisfy)
		{
			continue;
		}
		vector<cv::Mat> vecCamMosaicImage;
		h_SteelContour.SingleCamMosaic(vecCamImgList,vecCamMosaicImage);
		m_LogOper.AddLogInfo(L"Normal>> 完成进行单相机图像拼接！");	
		///4、多相机图像拼接
		m_LogOper.AddLogInfo(L"Normal>> 开始进行多相机图像拼接！");	
		cv::Mat TotalSteelMosaic;
		int iRes=h_SteelContour.MulitCamMosaic(vecCamMosaicImage,TotalSteelMosaic);
		if(-1 == iRes)
		{
			m_LogOper.AddLogInfo(L"Warning>> 受图像灰度影响，未有效进行头部对齐！");	
		}
		//CString strSavePath;
		//strSavePath.Format(L"%s\\test.bmp",strCurrSteelContourDataFilePath);
		//h_SteelContour.SaveImageOpenCV(strSavePath,TotalSteelMosaic);
		m_LogOper.AddLogInfo(L"Normal>> 完成进行多相机图像拼接！");	
		
		///5、钢板轮廓提取
		m_LogOper.AddLogInfo(L"Normal>> 开始进行图像轮廓提取！");
		//h_SteelContour.SaveImageOpenCV(L"1.bmp",TotalSteelMosaic);
		cv::Mat EffectSteel,ContourImage;
		vector<cv::Point> Contour_Steel_Precision;
		h_SteelContour.ExtractContour(iCurrProcessSteelSequenceNo,TotalSteelMosaic,EffectSteel,ContourImage,Contour_Steel_Precision);

		if(Contour_Steel_Precision.size()>0)
		{	
			//轮廓点排序
			vector<cv::Point> Contour_Steel_Precision_Sort_Y;
			vector<cv::Point> Contour_Steel_Precision_Sort_Y_Rectify;
			vector<cv::Point> Contour_Steel_Precision_Sort_X;
			for(int i=0;i<Contour_Steel_Precision.size();i++)
			{
				cv::Point p;
				p.x=Contour_Steel_Precision[i].x;
				p.y=Contour_Steel_Precision[i].y;
				Contour_Steel_Precision_Sort_Y.push_back(p);
				Contour_Steel_Precision_Sort_Y_Rectify.push_back(p);
				Contour_Steel_Precision_Sort_X.push_back(p);
			}
			sort(Contour_Steel_Precision_Sort_Y.begin(),Contour_Steel_Precision_Sort_Y.end(),comp_y);
			sort(Contour_Steel_Precision_Sort_X.begin(),Contour_Steel_Precision_Sort_X.end(),comp_x);
			//保存图像
			CString strSavePath;
			strSavePath.Format(L"%s\\Steel.bmp",strCurrSteelContourDataFilePath);
			h_SteelContour.SaveImageOpenCV(strSavePath,EffectSteel);
			strSavePath.Format(L"%s\\Total.jpg",strCurrSteelContourDataFilePath);
			//h_SteelContour.SaveImageOpenCV(strSavePath,ContourImage);
			cv::Mat ContourImage_Rectify;
			Rectify_Image(ContourImage,ContourImage_Rectify);
			h_SteelContour.SaveImageOpenCV(strSavePath,ContourImage_Rectify);
			//计算头尾有效钢板轮廓点
			h_SteelContour.GetEffectHeadAndTail(Contour_Steel_Precision_Sort_X,EffectSteel.cols,EffectSteel.rows);
			//保存钢板轮廓数据（实际）
			h_SteelContour.SaveContourActualCoord(cstrCurrSteelContourDataFilePath,"ContourActualCoord.txt",Contour_Steel_Precision,EffectSteel.cols);
			//保存钢板边界数据（实际）
			h_SteelContour.SaveContourActualEdge(cstrCurrSteelContourDataFilePath,"ContourActualEdge.txt",Contour_Steel_Precision_Sort_Y,EffectSteel.cols);
			//保存优化剪切数据（实际）
			h_SteelContour.SaveContourActualCutLine(cstrCurrSteelContourDataFilePath,"ContourActualCutLine.txt");
			//保存钢板边界信息（像素）
			h_SteelContour.SaveContourPixelCoord(cstrCurrSteelContourDataFilePath,Contour_Steel_Precision_Sort_Y,EffectSteel.cols);
			//保存拼接点坐标（像素）
			h_SteelContour.SaveJoinPix(cstrCurrSteelContourDataFilePath);
			//矫正轮廓坐标
			h_SteelContour.Coord_Rectify(Contour_Steel_Precision_Sort_Y_Rectify);
			//保存钢板轮廓数据（矫正实际）
			h_SteelContour.SaveContourActualCoord(cstrCurrSteelContourDataFilePath,"ContourActualCoord_Rectify.txt",Contour_Steel_Precision_Sort_Y_Rectify,EffectSteel.cols);
			//保存钢板边界数据（矫正实际）
			sort(Contour_Steel_Precision_Sort_Y_Rectify.begin(),Contour_Steel_Precision_Sort_Y_Rectify.end(),comp_y);
			h_SteelContour.SaveContourActualEdge(cstrCurrSteelContourDataFilePath,"ContourActualEdge_Rectify.txt",Contour_Steel_Precision_Sort_Y_Rectify,EffectSteel.cols);
			//保存优化剪切数据（矫正实际）
			h_SteelContour.SaveContourActualCutLine(cstrCurrSteelContourDataFilePath,"ContourActualCutLine_Rectify.txt");
			
		}
		else
		{
			m_LogOper.AddLogInfo(L"Warning>> 未成功提取到钢板轮廓数据！");
		}
		m_LogOper.AddLogInfo(L"Normal>> 完成进行图像轮廓提取！");
		///
		if(false==h_SteelContour.m_bIsOnline)
		{
			getchar();
		}
		CString strInfo=L"";
		strInfo.Format(L"Normal>> 完成流水号<<<<--------------------%d-------------------->>>>的钢板轮廓检测！",iCurrProcessSteelSequenceNo);
		m_LogOper.AddLogInfo(strInfo);	
	}
	///********************************************************************************************************************///
	cv::waitKey(0);
	printf("Waiting.....");
	getchar();
	return 0;
}

#include "stdafx.h"
#include "CommonFunction.h"

///*******************对拼接图像的预处理********************///
void PinjiePreProcess(cv::Mat src,ImgProcessParam tagImgProcessParam,cv::Mat &dst)
{
	cv::Mat src_grad_x,src_grad_y;
	cv::Mat src_grad_x_abs,src_grad_y_abs;
	cv::Mat src_x,src_y;
	cv::Sobel(src,src_grad_x,CV_16S, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT );
	cv::Sobel(src,src_grad_y,CV_16S, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT );
	cv::convertScaleAbs(src_grad_x,src_grad_x_abs);
	cv::convertScaleAbs(src_grad_y,src_grad_y_abs);
	cv::addWeighted(src_grad_x_abs,0.5,src_grad_x_abs,0.5,0,src_x);
	cv::addWeighted(src_grad_y_abs,0.5,src_grad_y_abs,0.5,0,src_y);

	cv::Mat src_th;
	cv::threshold(src,src_th,tagImgProcessParam.iCamGrayTh[tagImgProcessParam.iCamGrayThIndex],255,0);

	int ih=16;
	int iw=64;
	for(int i=0;i<src.rows/ih;i++)
	{
		for(int j=0;j<src.cols/iw;j++)
		{
			cv::Rect rect(j*iw, i*ih, iw, ih);

			cv::Mat temp_th;
			src_th(rect).copyTo(temp_th);
			cv::Mat temp_grad_x,temp_grad_y;
			src_x(rect).copyTo(temp_grad_x);
			src_y(rect).copyTo(temp_grad_y);
			cv::Mat Mean_grad_x,Dev_grad_x,Mean_grad_y,Dev_grad_y;
			cv::meanStdDev(temp_grad_x,Mean_grad_x,Dev_grad_x);
			cv::meanStdDev(temp_grad_y,Mean_grad_y,Dev_grad_y);
			double f_mean_grad_x,f_mean_grad_y;
			f_mean_grad_x=Mean_grad_x.at<double>(0,0);
			f_mean_grad_y=Mean_grad_y.at<double>(0,0)+0.0001;
			if(f_mean_grad_x/f_mean_grad_y < tagImgProcessParam.fGradXYTh)
			{
				cv::Mat zero=cv::Mat::zeros(ih,iw,src_th.type());
				zero.copyTo(src_th(rect));
			}
			
			//cv::Mat temp_grad,temp_th;
			//src_grad(rect).copyTo(temp_grad);
			//src_th(rect).copyTo(temp_th);
			//cv::Mat Mean_grad,Dev_grad,Mean_th,Dev_th;
			//cv::meanStdDev(temp_grad,Mean_grad,Dev_grad);
			//cv::meanStdDev(temp_th,Mean_th,Dev_th);
			//double f_mean_grad,f_mean_th;
			//f_mean_grad=Mean_grad.at<double>(0,0);
			//if(f_mean_grad<3)
			//{
			//	cv::Mat zero=cv::Mat::zeros(ih,iw,src_th.type());
			//	zero.copyTo(src_th(rect));
			//}
		}
	}
	dst=src_th.clone();
}
///*******************头部横向两张图像的拼接********************///
void PinJieFuncHeadTwoImg(cv::Mat Left, cv::Mat Right, RepeatArea tagRepeatAreaLeft, RepeatArea tagRepeatAreaRight, ImgProcessParam tagImgProcessParam,cv::Mat& PinJieImg, cv::Mat& PinJieImgSrc)
{
	///**************************************************求左图像的轮廓
	cv::Mat Left_grad_th;
	//if(tagRepeatAreaLeft.iLeftRepeat==0)
	//{
	//	cv::threshold(Left, Left_grad_th, tagImgProcessParam.iGrayTh, 255, 0);
	//}
	//else
	{
		PinjiePreProcess(Left, tagImgProcessParam,Left_grad_th);
	}

	cv::Mat Left_grad_th_dil;
	int dilation_size_Left_grad_th_dil = 7;//7
	cv::Mat element_Left_grad_th_dil = getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilation_size_Left_grad_th_dil + 1, 2 * dilation_size_Left_grad_th_dil + 1));
	morphologyEx(Left_grad_th, Left_grad_th_dil, 3, element_Left_grad_th_dil);
	//namedWindow("Left_grad_th_dil", 0);
	//imshow("Left_grad_th_dil", Left_grad_th_dil);

	//cv::Mat Left_grad_th_dil_cut,Left_grad_th_dil_cut_temp;
	//cv::Rect rect_left_cut_temp(Left_grad_th_dil.cols- tagRepeatAreaLeft.iRightRepeat, 0, tagRepeatAreaLeft.iRightRepeat, Left_grad_th_dil.rows);
	//Left_grad_th_dil(rect_left_cut_temp).copyTo(Left_grad_th_dil_cut_temp);
	//cv::Rect rect_left_cut(Left_grad_th_dil_cut_temp.cols*0.9, 0, Left_grad_th_dil_cut_temp.cols*0.1, Left_grad_th_dil_cut_temp.rows);
	//Left_grad_th_dil_cut_temp(rect_left_cut).copyTo(Left_grad_th_dil_cut);
	cv::Mat Left_grad_th_dil_cut;
	cv::Rect rect_left_cut(Left_grad_th_dil.cols- tagRepeatAreaLeft.iRightRepeat, 0, tagRepeatAreaLeft.iRightRepeat, Left_grad_th_dil.rows);
	Left_grad_th_dil(rect_left_cut).copyTo(Left_grad_th_dil_cut);

	RNG rng(12345);
	vector<vector<Point> > Left_contours;
	vector<Vec4i> Left_hierarchy;
	findContours(Left_grad_th_dil_cut, Left_contours, Left_hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<cv::Rect>boundRect_left(Left_contours.size());
	Mat Left_drawing = Mat::zeros(Left_grad_th_dil_cut.size(), CV_8UC3);
	int iMaxLeftArea = 0;
	int iMaxLeftAreaIndex = 0;
	int iLeftBegin = Left_grad_th_dil_cut.rows;
	//for (int i = 0; i< Left_contours.size(); i++)
	//{
	//	approxPolyDP(cv::Mat(Left_contours[i]), Left_contours[i], 3, true);
	//	boundRect_left[i] = boundingRect(cv::Mat(Left_contours[i]));
	//	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	//	if (cv::contourArea(Left_contours[i])>iMaxLeftArea)
	//	{
	//		iMaxLeftArea = cv::contourArea(Left_contours[i]);
	//		for (int j = 0; j < Left_contours[i].size(); j++)
	//		{
	//			if (Left_contours[i][j].x < Left_grad_th_dil_cut.cols * 0.1)//0.1
	//			{
	//				if (iLeftBegin > Left_contours[i][j].y)
	//				{
	//					iLeftBegin = Left_contours[i][j].y;
	//				}
	//			}
	//		}
	//		if (iLeftBegin == 0)
	//		{
	//			iLeftBegin = boundRect_left[i].tl().y;
	//		}
	//		
	//		drawContours(Left_drawing, Left_contours, i, color, 2, 8, Left_hierarchy, 0, Point());
	//	}
	//}
	int iMaxLeftArea_Index=0;
	for (int i = 0; i< Left_contours.size(); i++)
	{
		approxPolyDP(cv::Mat(Left_contours[i]), Left_contours[i], 3, true);
		boundRect_left[i] = boundingRect(cv::Mat(Left_contours[i]));
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		if (cv::contourArea(Left_contours[i])>iMaxLeftArea)
		{
			iMaxLeftArea = cv::contourArea(Left_contours[i]);
			iMaxLeftArea_Index=i;
		}
		for (int j = 0; j < Left_contours[i].size(); j++)
		{
			if (Left_contours[i][j].x < Left_grad_th_dil_cut.cols * 0.1 && Left_contours[i][j].y > Left_grad_th_dil_cut.rows * 0.1)//0.1
			{
				if (iLeftBegin > Left_contours[i][j].y)
				{
					iLeftBegin = Left_contours[i][j].y;
				}
			}
		}		
		drawContours(Left_drawing, Left_contours, i, color, 5, 8, Left_hierarchy, 0, Point());
	}
	if (iLeftBegin == 0 && Left_contours.size()>0)
	{
		iLeftBegin = boundRect_left[iMaxLeftArea_Index].tl().y;
	}
	//namedWindow("Left_Contours", 0);
	//imshow("Left_Contours", Left_drawing);

	///**************************************************求右图像的轮廓
	cv::Mat Right_grad_th;
	//if(tagRepeatAreaRight.iRightRepeat==0)
	//{
	//	cv::threshold(Right, Right_grad_th, tagImgProcessParam.iGrayTh, 255, 0);
	//}
	//else
	{
		tagImgProcessParam.iCamGrayThIndex=tagImgProcessParam.iCamGrayThIndex+1;
		PinjiePreProcess(Right, tagImgProcessParam,Right_grad_th);
	}

	cv::Mat Right_grad_th_dil;
	int dilation_size_Right_grad_th_dil = 7;
	cv::Mat element_Right_grad_th_dil = getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilation_size_Right_grad_th_dil + 1, 2 * dilation_size_Right_grad_th_dil + 1));
	morphologyEx(Right_grad_th, Right_grad_th_dil, 3, element_Right_grad_th_dil);
	//namedWindow("Right_grad_th_dil", 0);
	//imshow("Right_grad_th_dil", Right_grad_th_dil);

	//cv::Mat Right_grad_th_dil_cut,Right_grad_th_dil_cut_temp;
	//cv::Rect rect_right_cut_temp(0, 0, tagRepeatAreaRight.iLeftRepeat, Right_grad_th_dil.rows);
	//Right_grad_th_dil(rect_right_cut_temp).copyTo(Right_grad_th_dil_cut_temp);
	//cv::Rect rect_right_cut(0, 0, Right_grad_th_dil_cut_temp.cols*0.9, Right_grad_th_dil_cut_temp.rows);
	//Right_grad_th_dil_cut_temp(rect_right_cut).copyTo(Right_grad_th_dil_cut);
	cv::Mat Right_grad_th_dil_cut;
	cv::Rect rect_right_cut(0, 0, tagRepeatAreaRight.iLeftRepeat, Right_grad_th_dil.rows);
	Right_grad_th_dil(rect_right_cut).copyTo(Right_grad_th_dil_cut);

	vector<vector<Point> > Right_contours;
	vector<Vec4i> Right_hierarchy;
	findContours(Right_grad_th_dil_cut, Right_contours, Right_hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<cv::Rect>boundRect_right(Right_contours.size());
	Mat Right_drawing = Mat::zeros(Right_grad_th_dil_cut.size(), CV_8UC3);
	int iMaxRightArea = 0;
	int iMaxRightAreaIndex = 0;
	int iRightBegin = Right_grad_th_dil_cut.rows;
	//for (int i = 0; i< Right_contours.size(); i++)
	//{
	//	approxPolyDP(cv::Mat(Right_contours[i]), Right_contours[i], 3, true);
	//	boundRect_right[i] = boundingRect(cv::Mat(Right_contours[i]));
	//	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	//	if (cv::contourArea(Right_contours[i])>iMaxRightArea)
	//	{
	//		iMaxRightArea = cv::contourArea(Right_contours[i]);
	//		for (int j = 0; j < Right_contours[i].size(); j++)
	//		{
	//			if (Right_contours[i][j].x > Right_grad_th_dil_cut.cols * 0.9)//0.9
	//			{
	//				if (iRightBegin > Right_contours[i][j].y)
	//				{
	//					iRightBegin = Right_contours[i][j].y;
	//				}
	//			}
	//		}
	//		if (iRightBegin == 0)
	//		{
	//			iRightBegin = boundRect_right[i].tl().y;
	//		}
	//		
	//		drawContours(Right_drawing, Right_contours, i, color, 2, 8, Right_hierarchy, 0, Point());
	//	}
	//}
	int iMaxRightArea_Index=0;
	for (int i = 0; i< Right_contours.size(); i++)
	{
		approxPolyDP(cv::Mat(Right_contours[i]), Right_contours[i], 3, true);
		boundRect_right[i] = boundingRect(cv::Mat(Right_contours[i]));
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		if (cv::contourArea(Right_contours[i])>iMaxRightArea)
		{
			iMaxRightArea = cv::contourArea(Right_contours[i]);
			iMaxRightArea_Index=i;
		}
		for (int j = 0; j < Right_contours[i].size(); j++)
		{
			if (Right_contours[i][j].x > Right_grad_th_dil_cut.cols * 0.9 && Right_contours[i][j].y>Right_grad_th_dil_cut.rows*0.1)//0.9
			{
				if (iRightBegin > Right_contours[i][j].y)
				{
					iRightBegin = Right_contours[i][j].y;
				}
			}
		}		
		drawContours(Right_drawing, Right_contours, i, color, 5, 8, Right_hierarchy, 0, Point());
	}
	if (iRightBegin == 0 && Right_contours.size()>0)
	{
		iRightBegin = boundRect_right[iMaxRightArea_Index].tl().y;
	}
	//namedWindow("Right_Contours", 0);
	//imshow("Right_Contours", Right_drawing);

	///******************************************************************开始进行图像拼接
	int SS = iRightBegin - iLeftBegin;
	int TT = Right_grad_th_dil.rows - Left_grad_th_dil.rows;
	int ST = 0;
	if (iRightBegin == 0 || iLeftBegin == 0)
	{
		SS = 0;
	}
	if (TT * SS > 0)
	{
		if (abs(TT) > abs(SS))
		{
			ST = 0;
		}
		else
		{
			ST = abs(SS)-abs(TT);
		}		
	}
	else
	{
		ST = abs(SS);
	}
	cv::Mat Left_Cut, Right_Cut;
	cv::Mat Left_grad_th_dil_Cut, Right_grad_th_dil_Cut;
	cv::Rect rect_left(0, 0, Left.cols - tagRepeatAreaLeft.iRightRepeat,Left.rows);
	cv::Rect rect_right(tagRepeatAreaRight.iLeftRepeat,0,Right.cols- tagRepeatAreaRight.iLeftRepeat,Right.rows);
	Left(rect_left).copyTo(Left_Cut);
	Right(rect_right).copyTo(Right_Cut);
	Left_grad_th_dil(rect_left).copyTo(Left_grad_th_dil_Cut);
	Right_grad_th_dil(rect_right).copyTo(Right_grad_th_dil_Cut);

	PinJieImg = cv::Mat::zeros(Left_Cut.rows + abs(ST), Left_Cut.cols + Right_Cut.cols, Left_Cut.type());
	PinJieImgSrc = Mat::zeros(PinJieImg.size(), CV_8UC1);
	if (SS<0)
	{
		cv::Rect rect_pinjie_left(0, 0, Left_grad_th_dil_Cut.cols, Left_grad_th_dil_Cut.rows);
		Left_grad_th_dil_Cut.copyTo(PinJieImg(rect_pinjie_left));
		Left_Cut.copyTo(PinJieImgSrc(rect_pinjie_left));
		cv::Rect rect_pinjie_right(Left_grad_th_dil_Cut.cols, abs(SS), Right_grad_th_dil_Cut.cols, Right_grad_th_dil_Cut.rows);
		Right_grad_th_dil_Cut.copyTo(PinJieImg(rect_pinjie_right));
		Right_Cut.copyTo(PinJieImgSrc(rect_pinjie_right));
	}
	else
	{
		cv::Rect rect_pinjie_left(0, abs(SS), Left_grad_th_dil_Cut.cols, Left_grad_th_dil_Cut.rows);
		Left_grad_th_dil_Cut.copyTo(PinJieImg(rect_pinjie_left));
		Left_Cut.copyTo(PinJieImgSrc(rect_pinjie_left));
		cv::Rect rect_pinjie_right(Left_grad_th_dil_Cut.cols, 0, Right_grad_th_dil_Cut.cols, Right_grad_th_dil_Cut.rows);
		Right_grad_th_dil_Cut.copyTo(PinJieImg(rect_pinjie_right));
		Right_Cut.copyTo(PinJieImgSrc(rect_pinjie_right));
	}
	//cv::namedWindow("Left", 0);
	//cv::imshow("Left", Left);
	//cv::namedWindow("Right", 0);
	//cv::imshow("Right", Right);
	//cv::namedWindow("PinJieImg",0);
	//cv::imshow("PinJieImg",PinJieImg);
	//cv::waitKey(0);
	//int a=0;
}
///*******************尾部横向两张图像的拼接********************///
void PinJieFuncTailTwoImg(cv::Mat Left, cv::Mat Right, RepeatArea tagRepeatAreaLeft, RepeatArea tagRepeatAreaRight, ImgProcessParam tagImgProcessParam,cv::Mat& PinJieImg, cv::Mat& PinJieImgSrc)
{
	///**************************************************求左图像的轮廓
	cv::Mat Left_grad_th;
	//cv::threshold(Left, Left_grad_th, 30, 255, 0);
	PinjiePreProcess(Left, tagImgProcessParam,Left_grad_th);

	cv::Mat Left_grad_th_dil;
	int dilation_size_Left_grad_th_dil = 7;
	cv::Mat element_Left_grad_th_dil = getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilation_size_Left_grad_th_dil + 1, 2 * dilation_size_Left_grad_th_dil + 1));
	morphologyEx(Left_grad_th, Left_grad_th_dil, 3, element_Left_grad_th_dil);
	//namedWindow("Left_grad_th_dil", 0);
	//imshow("Left_grad_th_dil", Left_grad_th_dil);

	cv::Mat Left_grad_th_dil_cut;
	cv::Rect rect_left_cut(Left_grad_th_dil.cols - tagRepeatAreaLeft.iRightRepeat, 0, tagRepeatAreaLeft.iRightRepeat, Left_grad_th_dil.rows);
	Left_grad_th_dil(rect_left_cut).copyTo(Left_grad_th_dil_cut);

	RNG rng(12345);
	vector<vector<Point> > Left_contours;
	vector<Vec4i> Left_hierarchy;
	findContours(Left_grad_th_dil_cut, Left_contours, Left_hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<cv::Rect>boundRect_left(Left_contours.size());
	Mat Left_drawing = Mat::zeros(Left_grad_th_dil_cut.size(), CV_8UC3);
	int iMaxLeftArea = 0;
	int iMaxLeftAreaIndex = 0;
	int iLeftEnd = 0;
	//for (int i = 0; i< Left_contours.size(); i++)
	//{
	//	approxPolyDP(cv::Mat(Left_contours[i]), Left_contours[i], 3, true);
	//	boundRect_left[i] = boundingRect(cv::Mat(Left_contours[i]));
	//	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	//	if (cv::contourArea(Left_contours[i])>iMaxLeftArea)
	//	{
	//		iMaxLeftArea = cv::contourArea(Left_contours[i]);
	//		for (int j = 0; j < Left_contours[i].size(); j++)
	//		{
	//			if (Left_contours[i][j].x < Left_grad_th_dil_cut.cols * 0.1 && Left_contours[i][j].y<Left_grad_th_dil_cut.rows*0.8)
	//			{
	//				if (iLeftEnd < Left_contours[i][j].y)
	//				{
	//					iLeftEnd = Left_contours[i][j].y;
	//				}
	//			}
	//		}
	//		if (iLeftEnd == 0)
	//		{
	//			iLeftEnd = boundRect_left[i].br().y;
	//		}
	//		//iLeftEnd = boundRect_left[i].br().y;
	//		drawContours(Left_drawing, Left_contours, i, color, 5, 8, Left_hierarchy, 0, Point());
	//	}
	//}
	int iMaxLeftArea_Index=0;
	for (int i = 0; i< Left_contours.size(); i++)
	{
		approxPolyDP(cv::Mat(Left_contours[i]), Left_contours[i], 3, true);
		boundRect_left[i] = boundingRect(cv::Mat(Left_contours[i]));
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		if (cv::contourArea(Left_contours[i])>iMaxLeftArea)
		{
			iMaxLeftArea = cv::contourArea(Left_contours[i]);
			iMaxLeftArea_Index=i;
		}
		for (int j = 0; j < Left_contours[i].size(); j++)
		{
			if (Left_contours[i][j].x < Left_grad_th_dil_cut.cols * 0.1 && Left_contours[i][j].y<Left_grad_th_dil_cut.rows*0.9)
			{
				if (iLeftEnd < Left_contours[i][j].y)
				{
					iLeftEnd = Left_contours[i][j].y;
				}
			}
		}
		//iLeftEnd = boundRect_left[i].br().y;
		drawContours(Left_drawing, Left_contours, i, color, 5, 8, Left_hierarchy, 0, Point());
	}
	if (iLeftEnd == 0 && Left_contours.size()>0)
	{
		iLeftEnd = boundRect_left[iMaxLeftArea_Index].br().y;
	}
	//namedWindow("Left_Contours", 0);
	//imshow("Left_Contours", Left_drawing);
	///**************************************************求右图像的轮廓
	cv::Mat Right_grad_th;
	//cv::threshold(Right, Right_grad_th, 30, 255, 0);
	PinjiePreProcess(Right, tagImgProcessParam,Right_grad_th);

	cv::Mat Right_grad_th_dil;
	int dilation_size_Right_grad_th_dil = 7;
	cv::Mat element_Right_grad_th_dil = getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilation_size_Right_grad_th_dil + 1, 2 * dilation_size_Right_grad_th_dil + 1));
	morphologyEx(Right_grad_th, Right_grad_th_dil, 3, element_Right_grad_th_dil);
	//namedWindow("Right_grad_th_dil", 0);
	//imshow("Right_grad_th_dil", Right_grad_th_dil);

	cv::Mat Right_grad_th_dil_cut;
	cv::Rect rect_right_cut(0, 0, tagRepeatAreaRight.iLeftRepeat, Right_grad_th_dil.rows);
	Right_grad_th_dil(rect_right_cut).copyTo(Right_grad_th_dil_cut);

	vector<vector<Point> > Right_contours;
	vector<Vec4i> Right_hierarchy;
	findContours(Right_grad_th_dil_cut, Right_contours, Right_hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<cv::Rect>boundRect_right(Right_contours.size());
	Mat Right_drawing = Mat::zeros(Right_grad_th_dil_cut.size(), CV_8UC3);
	int iMaxRightArea = 0;
	int iMaxRightAreaIndex = 0;
	int iRightEnd = 0;
	//for (int i = 0; i< Right_contours.size(); i++)
	//{
	//	approxPolyDP(cv::Mat(Right_contours[i]), Right_contours[i], 3, true);
	//	boundRect_right[i] = boundingRect(cv::Mat(Right_contours[i]));
	//	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	//	if (cv::contourArea(Right_contours[i])>iMaxRightArea)
	//	{
	//		iMaxRightArea = cv::contourArea(Right_contours[i]);
	//		for (int j = 0; j < Right_contours[i].size(); j++)
	//		{
	//			if (Right_contours[i][j].x > Right_grad_th_dil_cut.cols * 0.9 && Right_contours[i][j].y<Right_grad_th_dil_cut.rows*0.8)
	//			{
	//				if (iRightEnd < Right_contours[i][j].y)
	//				{
	//					iRightEnd = Right_contours[i][j].y;
	//				}
	//			}
	//		}
	//		if (iRightEnd == 0)
	//		{
	//			iRightEnd = boundRect_right[i].br().y;
	//		}
	//		//iRightEnd = boundRect_right[i].br().y;
	//		drawContours(Right_drawing, Right_contours, i, color, 5, 8, Right_hierarchy, 0, Point());
	//	}
	//}
	int iMaxRightArea_Index=0;
	for (int i = 0; i< Right_contours.size(); i++)
	{
		approxPolyDP(cv::Mat(Right_contours[i]), Right_contours[i], 3, true);
		boundRect_right[i] = boundingRect(cv::Mat(Right_contours[i]));
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		if (cv::contourArea(Right_contours[i])>iMaxRightArea)
		{
			iMaxRightArea = cv::contourArea(Right_contours[i]);
			iMaxRightArea_Index=i;
		}
		for (int j = 0; j < Right_contours[i].size(); j++)
		{
			if (Right_contours[i][j].x > Right_grad_th_dil_cut.cols * 0.9 && Right_contours[i][j].y<Right_grad_th_dil_cut.rows*0.9)
			{
				if (iRightEnd < Right_contours[i][j].y)
				{
					iRightEnd = Right_contours[i][j].y;
				}
			}
		}
		drawContours(Right_drawing, Right_contours, i, color, 5, 8, Right_hierarchy, 0, Point());
	}
	if (iRightEnd == 0 && Right_contours.size()>0)
	{
		iRightEnd = boundRect_right[iMaxRightArea_Index].br().y;
	}
	//iRightEnd = boundRect_right[i].br().y;
	//namedWindow("Right_Contours", 0);
	//imshow("Right_Contours", Right_drawing);

	///******************************************************************开始进行图像拼接
	int SS = iRightEnd - iLeftEnd;
	int TT = Right_grad_th_dil.rows - Left_grad_th_dil.rows;
	int ST = 0;
	if (iRightEnd == 0 || iLeftEnd == 0)
	{
		SS = 0;
	}
	if (TT * SS > 0)
	{
		if (abs(TT) > abs(SS))
		{
			ST = 0;
		}
		else
		{
			ST = abs(SS) - abs(TT);
		}
	}
	else
	{
		ST = abs(SS);
	}
	cv::Mat Left_Cut, Right_Cut;
	cv::Mat Left_grad_th_dil_Cut, Right_grad_th_dil_Cut;
	cv::Rect rect_left(0, 0, Left.cols - tagRepeatAreaLeft.iRightRepeat, Left.rows);
	cv::Rect rect_right(tagRepeatAreaRight.iLeftRepeat, 0, Right.cols - tagRepeatAreaRight.iLeftRepeat, Right.rows);
	Left(rect_left).copyTo(Left_Cut);
	Right(rect_right).copyTo(Right_Cut);
	Left_grad_th_dil(rect_left).copyTo(Left_grad_th_dil_Cut);
	Right_grad_th_dil(rect_right).copyTo(Right_grad_th_dil_Cut);

	PinJieImg = cv::Mat::zeros(Left_Cut.rows + abs(ST), Left_Cut.cols + Right_Cut.cols, Left_Cut.type());
	PinJieImgSrc = Mat::zeros(PinJieImg.size(), CV_8UC1);
	if (SS<0)
	{
		cv::Rect rect_pinjie_left(0, 0, Left_grad_th_dil_Cut.cols, Left_grad_th_dil_Cut.rows);
		Left_grad_th_dil_Cut.copyTo(PinJieImg(rect_pinjie_left));
		Left_Cut.copyTo(PinJieImgSrc(rect_pinjie_left));
		cv::Rect rect_pinjie_right(Left_grad_th_dil_Cut.cols, abs(SS), Right_grad_th_dil_Cut.cols, Right_grad_th_dil_Cut.rows);
		Right_grad_th_dil_Cut.copyTo(PinJieImg(rect_pinjie_right));
		Right_Cut.copyTo(PinJieImgSrc(rect_pinjie_right));
	}
	else
	{
		cv::Rect rect_pinjie_left(0, abs(SS), Left_grad_th_dil_Cut.cols, Left_grad_th_dil_Cut.rows);
		Left_grad_th_dil_Cut.copyTo(PinJieImg(rect_pinjie_left));
		Left_Cut.copyTo(PinJieImgSrc(rect_pinjie_left));
		cv::Rect rect_pinjie_right(Left_grad_th_dil_Cut.cols, 0, Right_grad_th_dil_Cut.cols, Right_grad_th_dil_Cut.rows);
		Right_grad_th_dil_Cut.copyTo(PinJieImg(rect_pinjie_right));
		Right_Cut.copyTo(PinJieImgSrc(rect_pinjie_right));
	}
	//cv::namedWindow("Left", 0);
	//cv::imshow("Left", Left);
	//cv::namedWindow("Right", 0);
	//cv::imshow("Right", Right);
	//cv::namedWindow("PinJieImg",0);
	//cv::imshow("PinJieImg",PinJieImg);
	//cv::waitKey(0);
	//int a=0;
}
///*******************一个相机的多张图像拼接********************///
void PinJieOneCamMuliImg(vector<string> vecImgPath, int iStart, int iEnd, cv::Mat& PinJieImg2D)
{
	cv::Mat src = cv::imread(vecImgPath[0], 0);
	cv::Mat PinJieImg = cv::Mat::zeros(src.rows*(iEnd - iStart), src.cols, src.type());

	for (int i = iStart; i < iEnd; i++)
	{
		cv::Rect rect(0, src.rows*((i - iStart) % (iEnd - iStart)), src.cols, src.rows);
		cv::Mat img = cv::imread(vecImgPath[i], 0);
		img.copyTo(PinJieImg(rect));
	}
	cv::pyrDown(PinJieImg, PinJieImg2D, cv::Size(PinJieImg.cols / 2, PinJieImg.rows / 2));
}
///*******************多个相机的同一张图像的拼接********************///
 void PinJieMuliCamOneImg(vector<cv::Mat> vecImg, vector<RepeatArea> vecRepeatArea, ImgProcessParam tagImgProcessParam,cv::Mat& PinJieImg,cv::Mat& PinJieImgSrc)
{
	 int iCutSize = 0;
	 for (int i = 0; i < vecRepeatArea.size(); i++)
	 {
		 iCutSize = iCutSize + vecRepeatArea[i].iLeftRepeat + vecRepeatArea[i].iRightRepeat;
	 }
	 PinJieImg = cv::Mat::zeros(vecImg[0].rows, vecImg[0].cols*vecImg.size()- iCutSize, vecImg[0].type());
	 PinJieImgSrc = cv::Mat::zeros(vecImg[0].rows, vecImg[0].cols*vecImg.size()- iCutSize, vecImg[0].type());
	 int iX = 0;
	 for (int i = 0; i < vecImg.size(); i++)
	 {	 
		 int iWidth= vecImg[i].cols - vecRepeatArea[i].iLeftRepeat - vecRepeatArea[i].iRightRepeat;
		 cv::Rect rect_src(vecRepeatArea[i].iLeftRepeat, 0, iWidth, vecImg[i].rows);
		 cv::Rect rect(iX, 0, iWidth, vecImg[i].rows);
		 vecImg[i](rect_src).copyTo(PinJieImgSrc(rect));
		 cv::Mat Img_th;
		 cv::threshold(vecImg[i],Img_th,tagImgProcessParam.iCamGrayTh[i],255,0);
		 Img_th(rect_src).copyTo(PinJieImg(rect));
		 iX = iX + iWidth;
	 }
}
///*******************找头部的轮廓曲线********************///
 int PinJieFuncHeadAllCamImg(vector<cv::Mat> vecImgPinJie,vector<RepeatArea> vecRepeatArea,ImgProcessParam tagImgProcessParam,vector<cv::Point> & contours,cv::Mat &dst)
{
	//先拼接多个相机的图像
	cv::Mat PinJieImg, PinJieImgSrc;
	for (int i = 0; i<vecImgPinJie.size() - 1; i++)
	{
		tagImgProcessParam.iCamGrayThIndex=i;
		if (i == 0)
		{
			PinJieFuncHeadTwoImg(vecImgPinJie[i], vecImgPinJie[i + 1], vecRepeatArea[i], vecRepeatArea[i+1],tagImgProcessParam,PinJieImg, PinJieImgSrc);
		}
		else
		{
			PinJieFuncHeadTwoImg(PinJieImgSrc, vecImgPinJie[i + 1], vecRepeatArea[i], vecRepeatArea[i + 1], tagImgProcessParam,PinJieImg, PinJieImgSrc);
		}
	}
	//cv::namedWindow("PinJieImg_Head", 0);
	//cv::imshow("PinJieImg_Head", PinJieImg);
	//cv::waitKey(0);
	//cv::namedWindow("PinJieImgSrc_Head", 0);
	//cv::imshow("PinJieImgSrc_Head", PinJieImgSrc);
	//cv::waitKey(0);

	//针对上表边部的亮线（护板）做的处理
	cv::Mat PinJieImgSrc_Th;
	cv::Rect rect_left(0,0,PinJieImgSrc.cols*0.35,PinJieImgSrc.rows);
	cv::Rect rect_center(PinJieImgSrc.cols*0.35,0,PinJieImgSrc.cols*0.3,PinJieImgSrc.rows);
	cv::Rect rect_right(PinJieImgSrc.cols*0.65,0,PinJieImgSrc.cols*0.35,PinJieImgSrc.rows);
	cv::Mat PinJieImgSrc_Th_Left,PinJieImgSrc_Th_Center,PinJieImgSrc_Th_Right;
	cv::Mat PinJieImgSrc_Left,PinJieImgSrc_Center,PinJieImgSrc_Right;
	PinJieImgSrc(rect_left).copyTo(PinJieImgSrc_Left);
	PinJieImgSrc(rect_center).copyTo(PinJieImgSrc_Center);
	PinJieImgSrc(rect_right).copyTo(PinJieImgSrc_Right);
	cv::threshold(PinJieImgSrc_Left,PinJieImgSrc_Th_Left,tagImgProcessParam.iGrayTh,255,0);
	cv::threshold(PinJieImgSrc_Right,PinJieImgSrc_Th_Right,tagImgProcessParam.iGrayTh,255,0);
	PinJieImgSrc_Th_Left.copyTo(PinJieImg(rect_left));
	PinJieImgSrc_Th_Right.copyTo(PinJieImg(rect_right));
	//拼接后图像处理
	RNG rng(12345);
	Mat PinJieImgSrc_Color;
	cv::cvtColor(PinJieImgSrc, PinJieImgSrc_Color, CV_GRAY2RGB);
	cv::Mat PinJieContour = cv::Mat::zeros(PinJieImgSrc.rows, PinJieImgSrc.cols, PinJieImgSrc.type());
	//针对拼接图像作处理
	cv::Mat PinJieImg_dil;
	int dilation_size_PinJieImg_dil = 10;
	cv::Mat element_PinJieImg_dil = getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilation_size_PinJieImg_dil + 1, 2 * dilation_size_PinJieImg_dil + 1));
	morphologyEx(PinJieImg, PinJieImg_dil, 3, element_PinJieImg_dil);
	//dilate( Right_grad_th, Right_grad_th_dil, element_Right_grad_th_dil );
	//cv::namedWindow("PinJieImg_dil", 0);
	//cv::imshow("PinJieImg_dil", PinJieImg_dil);

	vector<vector<Point> > PinJieImg_dil_contours;
	vector<Vec4i> PinJieImg_dil_hierarchy;
	findContours(PinJieImg_dil, PinJieImg_dil_contours, PinJieImg_dil_hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<cv::Rect>boundRect_PinJieImg_dil(PinJieImg_dil_contours.size());
	Mat PinJieImg_dil_drawing = Mat::zeros(PinJieImg_dil.size(), CV_8UC3);
	int iMaxPinJieImgArea = 0;
	int iMaxPinJieImgAreaIndex = 0;
	for (int i = 0; i< PinJieImg_dil_contours.size(); i++)
	{
		boundRect_PinJieImg_dil[i] = boundingRect(cv::Mat(PinJieImg_dil_contours[i]));
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		if (cv::contourArea(PinJieImg_dil_contours[i])>iMaxPinJieImgArea)
		{
			iMaxPinJieImgArea = cv::contourArea(PinJieImg_dil_contours[i]);
			iMaxPinJieImgAreaIndex = i;
		}
	}
	cv::Mat PinJieDrawing = cv::Mat::zeros(PinJieImgSrc_Color.rows, PinJieImgSrc_Color.cols, CV_8UC1);
	drawContours(PinJieDrawing, PinJieImg_dil_contours, iMaxPinJieImgAreaIndex, 255, 5);
	drawContours(PinJieContour, PinJieImg_dil_contours, iMaxPinJieImgAreaIndex, 255, -5, 8, PinJieImg_dil_hierarchy, 0, Point());
	drawContours(PinJieImgSrc_Color, PinJieImg_dil_contours, iMaxPinJieImgAreaIndex, cv::Scalar(0, 255, 0), 5, 8, PinJieImg_dil_hierarchy, 0, Point());
	//namedWindow("PinJieDrawing_Head", 0);
	//imshow("PinJieDrawing_Head", PinJieDrawing);
	//namedWindow("PinJieContour_Head", 0);
	//imshow("PinJieContour_Head", PinJieContour);
	//namedWindow("PinJieImgSrc_Color_Head", 0);
	//imshow("PinJieImgSrc_Color_Head", PinJieImgSrc_Color);

	int iStartIndex = -1;
	int iEndIndex = -1;
	int iMaxDistance_LB = PinJieImgSrc.rows*PinJieImgSrc.cols;
	int iMaxDistance_RB = PinJieImgSrc.rows*PinJieImgSrc.cols;
	int iLeftPoint = 0;
	int iRightPoint = 0;
	//for (int i = 0; i < PinJieImg_dil_contours[iMaxPinJieImgAreaIndex].size(); i++)
	//{
	//	int x = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][i].x;
	//	int y = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][i].y;
	//	
	//	int x_2_LB = abs(x - 0)*abs(x - 0);
	//	int y_2_LB = abs(y - PinJieImgSrc.rows)*abs(y - PinJieImgSrc.rows);
	//	if (x_2_LB + y_2_LB< iMaxDistance_LB)
	//	{
	//		iMaxDistance_LB = x_2_LB + y_2_LB;
	//		iStartIndex = i;
	//		iLeftPoint = y;
	//	}

	//	int x_2_RB = abs(x - PinJieImgSrc.cols)*abs(x - PinJieImgSrc.cols);
	//	int y_2_RB = abs(y - PinJieImgSrc.rows)*abs(y - PinJieImgSrc.rows);
	//	if (x_2_RB + y_2_RB< iMaxDistance_RB)
	//	{
	//		iMaxDistance_RB = x_2_RB + y_2_RB;
	//		iEndIndex = i;
	//		iRightPoint = y;
	//	}
	//}

	//计算各相机图像在拼接图像上的有效左右边界
	vector<CamCutLeftRight> vecCamCutLeftRight;
	for (int i = 0; i<vecImgPinJie.size() ; i++)
	{
		int iLeft=vecImgPinJie[i].cols*i;
		for(int j=0;j<i;j++)
		{
			iLeft=iLeft-vecRepeatArea[j].iLeftRepeat-vecRepeatArea[j].iRightRepeat;
		}
		int iRight=vecImgPinJie[i].cols*(i+1);
		for(int j=0;j<i;j++)
		{
			iRight=iRight-vecRepeatArea[j].iLeftRepeat-vecRepeatArea[j].iRightRepeat;
		}
		iRight=iRight-vecRepeatArea[i].iLeftRepeat-vecRepeatArea[i].iRightRepeat;

		CamCutLeftRight tagCamCutLeftRight;
		tagCamCutLeftRight.iLeft=iLeft;
		tagCamCutLeftRight.iRight=iRight;
		vecCamCutLeftRight.push_back(tagCamCutLeftRight);
	}
	//计算左边起始点
	for (int i = 0; i<vecCamCutLeftRight.size() ; i++)
	{
		int iLeft=vecCamCutLeftRight[i].iLeft;
		int iRight=vecCamCutLeftRight[i].iRight;
		for (int j = 0; j < PinJieImg_dil_contours[iMaxPinJieImgAreaIndex].size(); j++)
		{
			int x = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][j].x;
			int y = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][j].y;
			if(x>=iLeft && x<=iRight)
			{
				int x_2_LB = abs(x - 0)*abs(x - 0);
				int y_2_LB = abs(y - PinJieImgSrc.rows)*abs(y - PinJieImgSrc.rows);
				if (x_2_LB + y_2_LB< iMaxDistance_LB)
				{
					iMaxDistance_LB = x_2_LB + y_2_LB;
					iStartIndex = j;
					iLeftPoint = y;
				}
			}
		}
		if(iStartIndex != -1)
		{
			break;
		}
	}
	//计算右边起始点
	for (int i = vecCamCutLeftRight.size()-1; i>=0 ; i--)
	{
		int iLeft=vecCamCutLeftRight[i].iLeft;
		int iRight=vecCamCutLeftRight[i].iRight;
		for (int j = 0; j < PinJieImg_dil_contours[iMaxPinJieImgAreaIndex].size(); j++)
		{
			int x = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][j].x;
			int y = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][j].y;
			if(x>=iLeft && x<=iRight)
			{
				int x_2_RB = abs(x - PinJieImgSrc.cols)*abs(x - PinJieImgSrc.cols);
				int y_2_RB = abs(y - PinJieImgSrc.rows)*abs(y - PinJieImgSrc.rows);
				if (x_2_RB + y_2_RB< iMaxDistance_RB)
				{
					iMaxDistance_RB = x_2_RB + y_2_RB;
					iEndIndex = j;
					iRightPoint = y;
				}
			}
		}
		if(iEndIndex != -1)
		{
			break;
		}
	}
	//图像异常 
	if(iStartIndex==-1 || iEndIndex==-1)
	{
		return -1;
	}
	for (int i = iStartIndex; i >= 0; i--)
	{
		contours.push_back(PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][i]);
	}
	for (int i = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex].size()-1; i >= iEndIndex; i--)
	{
		contours.push_back(PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][i]);
	}
	vector<vector<cv::Point>> Veccontours;
	Veccontours.push_back(contours);
	drawContours(PinJieImgSrc_Color, Veccontours, 0, cv::Scalar(0, 255, 255), 5, 8, PinJieImg_dil_hierarchy, 0, Point());
	//namedWindow("PinJieImgSrc_Color_Head_New", 0);
	//imshow("PinJieImgSrc_Color_Head_New", PinJieImgSrc_Color);
	dst=PinJieImgSrc_Color.clone();

	//return iRightPoint- iLeftPoint;
	return 1;
}
///*******************找尾部的轮廓曲线********************///
int PinJieFuncTailAllCamImg(vector<cv::Mat> vecImgPinJie, vector<RepeatArea> vecRepeatArea, ImgProcessParam tagImgProcessParam,vector<cv::Point> & contours,cv::Mat &dst)
{
	cv::Mat PinJieImg, PinJieImgSrc;
	for (int i = 0; i<vecImgPinJie.size() - 1; i++)
	{
		tagImgProcessParam.iCamGrayThIndex=i;
		if (i == 0)
		{
			PinJieFuncTailTwoImg(vecImgPinJie[i], vecImgPinJie[i + 1], vecRepeatArea[i], vecRepeatArea[i + 1], tagImgProcessParam,PinJieImg, PinJieImgSrc);
		}
		else
		{
			PinJieFuncTailTwoImg(PinJieImgSrc, vecImgPinJie[i + 1], vecRepeatArea[i], vecRepeatArea[i + 1], tagImgProcessParam,PinJieImg, PinJieImgSrc);
		}
	}
	//cv::namedWindow("PinJieImg_Tail", 0);
	//cv::imshow("PinJieImg_Tail", PinJieImg);
	//cv::namedWindow("PinJieImgSrc_Tail", 0);
	//cv::imshow("PinJieImgSrc_Tail", PinJieImgSrc);
	//cv::waitKey(0);


	//针对上表边部的亮线（护板）做的处理
	cv::Mat PinJieImgSrc_Th;
	cv::Rect rect_left(0,0,PinJieImgSrc.cols*0.35,PinJieImgSrc.rows);
	cv::Rect rect_center(PinJieImgSrc.cols*0.35,0,PinJieImgSrc.cols*0.3,PinJieImgSrc.rows);
	cv::Rect rect_right(PinJieImgSrc.cols*0.65,0,PinJieImgSrc.cols*0.35,PinJieImgSrc.rows);
	cv::Mat PinJieImgSrc_Th_Left,PinJieImgSrc_Th_Center,PinJieImgSrc_Th_Right;
	cv::Mat PinJieImgSrc_Left,PinJieImgSrc_Center,PinJieImgSrc_Right;
	PinJieImgSrc(rect_left).copyTo(PinJieImgSrc_Left);
	PinJieImgSrc(rect_center).copyTo(PinJieImgSrc_Center);
	PinJieImgSrc(rect_right).copyTo(PinJieImgSrc_Right);
	cv::threshold(PinJieImgSrc_Left,PinJieImgSrc_Th_Left,tagImgProcessParam.iGrayTh,255,0);
	cv::threshold(PinJieImgSrc_Right,PinJieImgSrc_Th_Right,tagImgProcessParam.iGrayTh,255,0);
	PinJieImgSrc_Th_Left.copyTo(PinJieImg(rect_left));
	PinJieImgSrc_Th_Right.copyTo(PinJieImg(rect_right));
	//拼接后图像处理
	RNG rng(12345);
	Mat PinJieImgSrc_Color;
	cv::cvtColor(PinJieImgSrc, PinJieImgSrc_Color, CV_GRAY2RGB);
	cv::Mat PinJieContour = cv::Mat::zeros(PinJieImgSrc.rows, PinJieImgSrc.cols, PinJieImgSrc.type());
	//针对拼接图像作处理
	cv::Mat PinJieImg_dil;
	int dilation_size_PinJieImg_dil = 10;
	cv::Mat element_PinJieImg_dil = getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilation_size_PinJieImg_dil + 1, 2 * dilation_size_PinJieImg_dil + 1));
	morphologyEx(PinJieImg, PinJieImg_dil, 3, element_PinJieImg_dil);

	vector<vector<Point> > PinJieImg_dil_contours;
	vector<Vec4i> PinJieImg_dil_hierarchy;
	findContours(PinJieImg_dil, PinJieImg_dil_contours, PinJieImg_dil_hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<cv::Rect>boundRect_PinJieImg_dil(PinJieImg_dil_contours.size());
	Mat PinJieImg_dil_drawing = Mat::zeros(PinJieImg_dil.size(), CV_8UC3);
	int iMaxPinJieImgArea = 0;
	int iMaxPinJieImgAreaIndex = 0;
	for (int i = 0; i< PinJieImg_dil_contours.size(); i++)
	{
		boundRect_PinJieImg_dil[i] = boundingRect(cv::Mat(PinJieImg_dil_contours[i]));
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		if (cv::contourArea(PinJieImg_dil_contours[i])>iMaxPinJieImgArea)
		{
			iMaxPinJieImgArea = cv::contourArea(PinJieImg_dil_contours[i]);
			iMaxPinJieImgAreaIndex = i;
		}
	}
	cv::Mat PinJieDrawing = cv::Mat::zeros(PinJieImgSrc_Color.rows, PinJieImgSrc_Color.cols, CV_8UC1);
	drawContours(PinJieDrawing, PinJieImg_dil_contours, iMaxPinJieImgAreaIndex, 255, 5);
	drawContours(PinJieContour, PinJieImg_dil_contours, iMaxPinJieImgAreaIndex, 255, -5, 8, PinJieImg_dil_hierarchy, 0, Point());
	drawContours(PinJieImgSrc_Color, PinJieImg_dil_contours, iMaxPinJieImgAreaIndex, cv::Scalar(0, 255, 0), 5, 8, PinJieImg_dil_hierarchy, 0, Point());
	//namedWindow("PinJieDrawing_Tail", 0);
	//imshow("PinJieDrawing_Tail", PinJieDrawing);
	//namedWindow("PinJieContour_Tail", 0);
	//imshow("PinJieContour_Tail", PinJieContour);
	//namedWindow("PinJieImgSrc_Color_Tail", 0);
	//imshow("PinJieImgSrc_Color_Tail", PinJieImgSrc_Color);
	
	int iStartIndex = -1;
	int iEndIndex = -1;
	int iMaxDistance_LT = PinJieImgSrc.rows*PinJieImgSrc.cols;
	int iMaxDistance_RT = PinJieImgSrc.rows*PinJieImgSrc.cols;
	int iLeftPoint = 0;
	int iRightPoint = 0;
	//for (int i = 0; i < PinJieImg_dil_contours[iMaxPinJieImgAreaIndex].size(); i++)
	//{
	//	int x = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][i].x;
	//	int y = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][i].y;

	//	int x_2_RT = abs(x - PinJieImgSrc.cols)*abs(x - PinJieImgSrc.cols);
	//	int y_2_RT = abs(y - 0)*abs(y - 0);
	//	if (x_2_RT + y_2_RT< iMaxDistance_RT)
	//	{
	//		iMaxDistance_RT = x_2_RT + y_2_RT;
	//		iStartIndex = i;
	//		iRightPoint = y;
	//	}

	//	int x_2_LT = abs(x - 0)*abs(x - 0);
	//	int y_2_LT = abs(y - 0)*abs(y - 0);
	//	if (x_2_LT + y_2_LT< iMaxDistance_LT)
	//	{
	//		iMaxDistance_LT = x_2_LT + y_2_LT;
	//		iEndIndex = i;
	//		iLeftPoint = y;
	//	}
	//}

	//计算各相机图像在拼接图像上的有效左右边界
	vector<CamCutLeftRight> vecCamCutLeftRight;
	for (int i = 0; i<vecImgPinJie.size() ; i++)
	{
		int iLeft=vecImgPinJie[i].cols*i;
		for(int j=0;j<i;j++)
		{
			iLeft=iLeft-vecRepeatArea[j].iLeftRepeat-vecRepeatArea[j].iRightRepeat;
		}
		//iLeft=iLeft+vecRepeatArea[i].iLeftRepeat;
		int iRight=vecImgPinJie[i].cols*(i+1);
		for(int j=0;j<i;j++)
		{
			iRight=iRight-vecRepeatArea[j].iLeftRepeat-vecRepeatArea[j].iRightRepeat;
		}
		iRight=iRight-vecRepeatArea[i].iLeftRepeat-vecRepeatArea[i].iRightRepeat;

		CamCutLeftRight tagCamCutLeftRight;
		tagCamCutLeftRight.iLeft=iLeft;
		tagCamCutLeftRight.iRight=iRight;
		vecCamCutLeftRight.push_back(tagCamCutLeftRight);
	}
	//图像轮廓左起始点
	for (int i = 0; i<vecCamCutLeftRight.size() ; i++)
	{
		int iLeft=vecCamCutLeftRight[i].iLeft;
		int iRight=vecCamCutLeftRight[i].iRight;
		for (int j = 0; j < PinJieImg_dil_contours[iMaxPinJieImgAreaIndex].size(); j++)
		{
			int x = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][j].x;
			int y = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][j].y;
			if(x>=iLeft && x<=iRight)
			{
				int x_2_LT = abs(x - 0)*abs(x - 0);
				int y_2_LT = abs(y - 0)*abs(y - 0);
				if (x_2_LT + y_2_LT< iMaxDistance_LT)
				{
					iMaxDistance_LT = x_2_LT + y_2_LT;
					iEndIndex = j;
					iLeftPoint = y;
				}
			}
		}
		if(iEndIndex != -1)
		{
			break;
		}
	}
	//图像轮廓右起始点
	for (int i = vecCamCutLeftRight.size()-1; i>=0 ; i--)
	{
		int iLeft=vecCamCutLeftRight[i].iLeft;
		int iRight=vecCamCutLeftRight[i].iRight;
		for (int j = 0; j < PinJieImg_dil_contours[iMaxPinJieImgAreaIndex].size(); j++)
		{
			int x = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][j].x;
			int y = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][j].y;
			if(x>=iLeft && x<=iRight)
			{
				int x_2_RT = abs(x - PinJieImgSrc.cols)*abs(x - PinJieImgSrc.cols);
				int y_2_RT = abs(y - 0)*abs(y - 0);
				if (x_2_RT + y_2_RT< iMaxDistance_RT)
				{
					iMaxDistance_RT = x_2_RT + y_2_RT;
					iStartIndex = j;
					iRightPoint = y;
				}
			}
		}
		if(iStartIndex != -1)
		{
			break;
		}
	}
	//异常图像
	if(iStartIndex==-1 || iEndIndex==-1)
	{
		return -1;
	}
	for (int i = iStartIndex; i >= iEndIndex; i--)
	{
		contours.push_back(PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][i]);
	}
	vector<vector<cv::Point>> Veccontours;
	Veccontours.push_back(contours);
	drawContours(PinJieImgSrc_Color, Veccontours, 0, cv::Scalar(0, 255, 255), 5, 8, PinJieImg_dil_hierarchy, 0, Point());
	//namedWindow("PinJieImgSrc_Color_Tail_New", 0);
	//imshow("PinJieImgSrc_Color_Tail_New", PinJieImgSrc_Color);
	dst=PinJieImgSrc_Color.clone();

	//return iRightPoint- iLeftPoint;
	return 1;
}
///*******************找中间图像的边界轮廓********************///
void PinJieFuncMiddleAllCamImg(cv::Mat ImgPinJie, cv::Mat ImgPinJie_th,vector<RepeatArea> vecRepeatArea, ImgProcessParam tagImgProcessParam,vector<cv::Point> & contours_left, vector<cv::Point> & contours_right,cv::Mat &dst)
{
	
	//cv::threshold(ImgPinJie, ImgPinJie_th, tagImgProcessParam.iGrayTh, 255, 0);
	//namedWindow("ImgPinJie_th", 0);
	//imshow("ImgPinJie_th", ImgPinJie_th);

	cv::Mat ImgPinJie_th_dil;
	int dilation_size_ImgPinJie_th_dil = 7;
	cv::Mat element_ImgPinJie_th_dil = getStructuringElement(cv::MORPH_RECT, cv::Size(2 * dilation_size_ImgPinJie_th_dil + 1, 2 * dilation_size_ImgPinJie_th_dil + 1));
	morphologyEx(ImgPinJie_th, ImgPinJie_th_dil, 3, element_ImgPinJie_th_dil);

	vector<vector<Point> > PinJieImg_dil_contours;
	vector<Vec4i> PinJieImg_dil_hierarchy;
	findContours(ImgPinJie_th_dil, PinJieImg_dil_contours, PinJieImg_dil_hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<cv::Rect>boundRect_PinJieImg_dil(PinJieImg_dil_contours.size());
	Mat PinJieImg_dil_drawing = Mat::zeros(ImgPinJie_th_dil.size(), CV_8UC3);
	int iMaxPinJieImgArea = 0;
	int iMaxPinJieImgAreaIndex = 0;
	for (int i = 0; i< PinJieImg_dil_contours.size(); i++)
	{
		boundRect_PinJieImg_dil[i] = boundingRect(cv::Mat(PinJieImg_dil_contours[i]));
		if (cv::contourArea(PinJieImg_dil_contours[i])>iMaxPinJieImgArea)
		{
			iMaxPinJieImgArea = cv::contourArea(PinJieImg_dil_contours[i]);
			iMaxPinJieImgAreaIndex = i;
		}
	}
	Mat PinJieImgSrc_Color;
	cv::cvtColor(ImgPinJie, PinJieImgSrc_Color, CV_GRAY2RGB);
	drawContours(PinJieImgSrc_Color, PinJieImg_dil_contours, iMaxPinJieImgAreaIndex, cv::Scalar(0, 255, 0), 5, 8, PinJieImg_dil_hierarchy, 0, Point());
	//namedWindow("PinJieImgSrc_Color_Middle", 0);
	//imshow("PinJieImgSrc_Color_Middle", PinJieImgSrc_Color);
	//cv::waitKey(0);

	int iStartIndex_Left = -1;
	int iEndIndex_Left = -1;
	int iStartIndex_Right = -1;
	int iEndIndex_Right = -1;
	int iMinDistance_LT = ImgPinJie.rows*ImgPinJie.rows + ImgPinJie.cols*ImgPinJie.cols;
	int iMinDistance_RT = ImgPinJie.rows*ImgPinJie.rows + ImgPinJie.cols*ImgPinJie.cols;
	int iMinDistance_LB = ImgPinJie.rows*ImgPinJie.rows+ImgPinJie.cols*ImgPinJie.cols;
	int iMinDistance_RB = ImgPinJie.rows*ImgPinJie.rows + ImgPinJie.cols*ImgPinJie.cols;
	for (int i = 0; i < PinJieImg_dil_contours[iMaxPinJieImgAreaIndex].size(); i++)
	{
		int x = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][i].x;
		int y = PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][i].y;

		int x_2_LT = abs(x - 0)*abs(x - 0);
		int y_2_LT = abs(y - 0)*abs(y - 0);
		if (x_2_LT + y_2_LT< iMinDistance_LT)
		{
			iMinDistance_LT = x_2_LT + y_2_LT;
			iStartIndex_Left = i;
		}

		int x_2_LB = abs(x - 0)*abs(x - 0);
		int y_2_LB = abs(y - ImgPinJie.rows)*abs(y - ImgPinJie.rows);
		if (x_2_LB + y_2_LB< iMinDistance_LB)
		{
			iMinDistance_LB = x_2_LB + y_2_LB;
			iEndIndex_Left = i;
		}

		int x_2_RT = abs(x - ImgPinJie.cols)*abs(x - ImgPinJie.cols);
		int y_2_RT = abs(y - 0)*abs(y - 0);
		if (x_2_RT + y_2_RT< iMinDistance_RT)
		{
			iMinDistance_RT = x_2_RT + y_2_RT;
			iStartIndex_Right = i;
		}

		int x_2_RB = abs(x - ImgPinJie.cols)*abs(x - ImgPinJie.cols);
		int y_2_RB = abs(y - ImgPinJie.rows)*abs(y - ImgPinJie.rows);
		if (x_2_RB + y_2_RB< iMinDistance_RB)
		{
			iMinDistance_RB = x_2_RB + y_2_RB;
			iEndIndex_Right = i;
		}
	}

	for (int i = iStartIndex_Left; i <= iEndIndex_Left; i++)
	{
		contours_left.push_back(PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][i]);
	}
	for (int i = iStartIndex_Right; i >= iEndIndex_Right; i--)
	{
		contours_right.push_back(PinJieImg_dil_contours[iMaxPinJieImgAreaIndex][i]);
	}
	vector<vector<cv::Point>> Veccontours;
	Veccontours.push_back(contours_left);
	Veccontours.push_back(contours_right);
	drawContours(PinJieImgSrc_Color, Veccontours, 0, cv::Scalar(0, 255, 255), 10, 8, PinJieImg_dil_hierarchy, 0, Point());
	drawContours(PinJieImgSrc_Color, Veccontours, 1, cv::Scalar(0, 255, 255), 10, 8, PinJieImg_dil_hierarchy, 0, Point());
	//namedWindow("PinJieImgSrc_Color_Middle_New", 0);
	//imshow("PinJieImgSrc_Color_Middle_New", PinJieImgSrc_Color);
	dst=PinJieImgSrc_Color.clone();
	//cv::waitKey();
}
///*******************删除目录********************///
int  removeDir(const char*  dirPath)  
{  
    struct _finddata_t fb;   //查找相同属性文件的存储结构体  
    char  path[250];            
    long    handle;  
    int  resultone;  
    int   noFile;            //对系统隐藏文件的处理标记  
      
    noFile = 0;  
    handle = 0;   
    //制作路径  
    strcpy(path,dirPath);  
    strcat (path,"/*");  
    handle = _findfirst(path,&fb);  
    //找到第一个匹配的文件  
    if (handle != 0)  
    {  
        //当可以继续找到匹配的文件，继续执行  
        while (0 == _findnext(handle,&fb))  
        {  
            //windows下，常有个系统文件，名为“..”,对它不做处理  
            noFile = strcmp(fb.name,"..");            
            if (0 != noFile)  
            {  
                //制作完整路径  
                memset(path,0,sizeof(path));  
                strcpy(path,dirPath);  
                strcat(path,"/");  
                strcat (path,fb.name);  
                //属性值为16，则说明是文件夹，迭代  
                if (fb.attrib == 16)  
                {  
                     removeDir(path);     
                }            
                else  //非文件夹的文件，直接删除。对文件属性值的情况没做详细调查，可能还有其他情况。  
                {  
                    remove(path);  
                }  
            }     
        }  
        //关闭文件夹，只有关闭了才能删除。找这个函数找了很久，标准c中用的是closedir  
        //经验介绍：一般产生Handle的函数执行后，都要进行关闭的动作。  
        _findclose(handle);  
    }  
    //移除文件夹  
    resultone = rmdir(dirPath);  
    return  resultone;  
}  
///*******************从img图像获取信息********************///
int GetSteelImg(CString strImgPath,SteelImgInfoAndData &tSteelImgInfoAndData)
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
///*******************列表文件夹下的img图像********************///
void listImg(const char * dir, vector<string> &vecFileList)
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
///*******************两层文件夹下文件列表********************///
void listFiles(const char * dir, vector<vector<string>> &vecFileList)
{
	char dirNew[200];
	strcpy_s(dirNew, dir);
	strcat_s(dirNew, "\\*.*");

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
			//cout << findData.name << "\t<dir>\n";

			strcpy_s(dirNew, dir);
			strcat_s(dirNew, "\\");
			strcat_s(dirNew, findData.name);

			vector<string> vecStr;
			vecFileList.push_back(vecStr);
			listFiles(dirNew, vecFileList);
		}
		else
		{
			char dirImg[200];
			strcpy_s(dirImg, dir);
			strcat_s(dirImg, "\\");
			strcat_s(dirImg, findData.name);

			vecFileList[vecFileList.size()-1].push_back(dirImg);
		}
	} while (_findnext(handle, &findData) == 0);

	_findclose(handle);
}
///*******************保存图像********************///
void Save_Image_OpenCV(CString strPath,cv::Mat src) 
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

void Rectify_Image(cv::Mat src,cv::Mat &dst)
{
	int th=25;

	cv::Mat src_resize;
	cv::resize(src,src_resize,cv::Size(src.cols/8,32));

	cv::Mat src_resize_th;
	cv::threshold(src_resize,src_resize_th,th,255,0);

	dst=cv::Mat::zeros(src.size(), CV_8UC1);

	float *f_e=new float[src_resize.cols];
	for(int i=0;i<src_resize.cols;i++)
	{
		cv::Scalar s_m;
		s_m=cv::mean(src_resize.col(i),src_resize_th.col(i));
		float e=1;;
		if(s_m.val[0]>=th)
		{
			e=100/s_m.val[0];
		}
		f_e[i]=e;
	}

	int iRido=src.cols/src_resize.cols;
	for(int i=0;i<src.cols;i++)
	{
		int index=i/iRido;
		dst.col(i)=src.col(i)*f_e[index];
	}

	delete[] f_e;
}

bool comp_y(const cv::Point &a, const cv::Point &b)
{
	return a.y < b.y;
}

bool comp_x(const cv::Point &a, const cv::Point &b)
{
	return a.x < b.x;
}
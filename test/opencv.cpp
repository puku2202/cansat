#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <vector>
#include <algorithm>
#include <experimental/filesystem>
using namespace std;

#define IMAGE_NUM  (26)         /* 画像数 */
#define PAT_ROW    (7)          /* パターンの行数 */
#define PAT_COL    (10)         /* パターンの列数 */
#define PAT_SIZE   (PAT_ROW*PAT_COL)
#define ALL_POINTS (IMAGE_NUM*PAT_SIZE)
#define CHESS_SIZE (24.0)       /* パターン1マスの1辺サイズ[mm] */

int main(int argc, char *argv[])
{
	int i, j, k;
	int corner_count, found;
	int p_count[IMAGE_NUM];
	// cv::Mat src_img[IMAGE_NUM];
	vector<cv::Mat> srcImages;
	cv::Size pattern_size = cv::Size2i(PAT_COL, PAT_ROW);
	vector<cv::Point2f> corners;
	vector<vector<cv::Point2f>> img_points;

	//キャリブレーション画像の読み込み
	for (i = 0; i < IMAGE_NUM; i++)
	{
		ostringstream ostr;
		ostr << setw(2) << i+10 << ".png";
		cv::Mat src = cv::imread(ostr.str());
		if (src.empty())
		{
			cerr << "cannot load image file : " << ostr.str() << endl;
		}
		else
		{
			srcImages.push_back(src);
		}
	}

	//3次元空間座標の設定

	vector<cv::Point3f> object;
	for (j = 0; j < PAT_ROW; j++)
	{
		for (k = 0; k < PAT_COL; k++)
		{
			cv::Point3f p(
					j * CHESS_SIZE,
					k * CHESS_SIZE,
					0.0);
			object.push_back(p);
		}
	}

	vector<vector<cv::Point3f>> obj_points;
	for (i = 0; i < IMAGE_NUM; i++)
	{
		obj_points.push_back(object);
	}

	//３次元の点を ALL_POINTS * 3 の行列(32Bit浮動小数点数:１チャンネル)に変換する 


	//チェスボード（キャリブレーションパターン）のコーナー検出
	int found_num = 0;
	cv::namedWindow("Calibration", cv::WINDOW_AUTOSIZE);
	for (i = 0; i < IMAGE_NUM; i++)
	{
		auto found = cv::findChessboardCorners(srcImages[i], pattern_size, corners);
		if (found)
		{
			cout << setfill('0') << setw(2) << i << "... ok" << endl;
			found_num++;
		}
		else
		{
			cerr << setfill('0') << setw(2) << i << "... fail" << endl;
		}

		//コーナー位置をサブピクセル精度に修正，描画
		cv::Mat src_gray = cv::Mat(srcImages[i].size(), CV_8UC1);
		cv::cvtColor(srcImages[i], src_gray, cv::COLOR_BGR2GRAY);
		cv::find4QuadCornerSubpix(src_gray, corners, cv::Size(3,3));
		cv::drawChessboardCorners(srcImages[i], pattern_size, corners, found);
		img_points.push_back(corners);

		//cv::imshow("Calibration", srcImages[i]);
		//cv::waitKey(1);
	}
	cv::destroyWindow("Calibration");

	if (found_num != IMAGE_NUM)
	{
		cerr << "Calibration Images are insufficient." << endl;
		return -1;
	}

	//内部パラメータ，歪み係数の推定
	std::cout << "cv::calibrateCamera" << std::endl;
	cv::Mat persK, persD, persR, persT;
	std::string persFile = "./camera.xml";
	double persRMS = cv::calibrateCamera(obj_points, img_points, srcImages[0].size(), persK, persD, persR, persT);
	cv::FileStorage persxml("persFile", cv::FileStorage::WRITE);
	cv::write(persxml, "RMS", persRMS);
	cv::write(persxml, "intrinsic", persK);
	cv::write(persxml, "distorction", persD);
	persxml.release();


	/*/Undistort
	std::cout << "Undistort" << std::endl;
	cv::Mat distorted = cv::imread("photo.jpg");
	cv::Mat undistorted;
    
	cv::undistort(distorted, undistorted, persK, persD);
	cout <<persRMS  << endl;
	cv::imwrite("undistort.jpg", undistorted);
	
	for (i = 0; i < IMAGE_NUM; i++) {
		cv::ReleaseImage (&src_img[i]);
  	}
*/	

	return 0;

}

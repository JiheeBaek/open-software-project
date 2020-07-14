#include <iostream>
#include <opencv2/opencv.hpp>
#include <math.h>       /* exp */
#define IM_TYPE	CV_8UC3


using namespace cv;

// Image Type
// "G" for GrayScale Image, "C" for Color Image
#if (IM_TYPE == CV_8UC3)
typedef uchar G;
typedef cv::Vec3b C;
#elif (IM_TYPE == CV_16SC3)
typedef short G;
typedef Vec3s C;
#elif (IM_TYPE == CV_32SC3)
typedef int G;
typedef Vec3i C;
#elif (IM_TYPE == CV_32FC3)
typedef float G;
typedef Vec3f C;
#elif (IM_TYPE == CV_64FC3)
typedef double G;
typedef Vec3d C;
#endif

Mat gaussianfilterSep(const Mat input, int n, float sigmaT, float sigmaS, const char* opt);

int main() {
	Mat input = imread("lena.jpg", CV_LOAD_IMAGE_COLOR);
	Mat input_gray;
	Mat output;

	cvtColor(input, input_gray, CV_RGB2GRAY);

	if (!input.data)
	{
		std::cout << "Could not open" << std::endl;
		return -1;
	}

	namedWindow("Grayscale", WINDOW_AUTOSIZE);
	imshow("Grayscale", input_gray);
	output = gaussianfilterSep(input_gray, 1,1,1, "zero-paddle"); //Boundary process: zero-paddle, mirroring, adjustkernel
	//output = gaussianfilterSep(input_gray, 1, 1, 1, "mirroring");
	//output = gaussianfilterSep(input_gray, 1, 1, 1, "adjustkernel");

	namedWindow("Gaussian Filter", WINDOW_AUTOSIZE);
	imshow("Gaussian Filter", output);


	waitKey(0);

	return 0;
}


Mat gaussianfilterSep(const Mat input, int n, float sigmaT, float sigmaS, const char* opt) {

	Mat kernel_s;
	Mat kernel_t;

	int row = input.rows;
	int col = input.cols;
	int kernel_size = (2 * n + 1);
	int tempa;
	int tempb;
	float denom1;
	float denom2;
	float kernelvalue1;
	float kernelvalue2;

	// Initialiazing Kernel Matrix 
	kernel_s = Mat::zeros(kernel_size, 1, CV_32F);
	kernel_t = Mat::zeros(1, kernel_size, CV_32F);

	denom1 = 0.0;
	denom2 = 0.0;
	for (int a = -n; a <= n; a++) {  // Denominator in m(s,t)
		float value1 = exp(-(pow(a, 2) / (2 * pow(sigmaS, 2))));
		kernel_s.at<float>(a + n, 0) = value1;
		denom1 += value1;
	}

	for (int b = -n; b <= n; b++) {
		float value2 = exp(-(pow(b, 2) / (2 * pow(sigmaT, 2))));
		kernel_t.at<float>(0, b + n) = value2;
		denom2 += value2;
	}

	for (int a = -n; a <= n; a++)
		kernel_s.at<float>(a + n, 0) /= denom1;

	for (int b = -n; b <= n; b++)
		kernel_t.at<float>(0, b + n) /= denom2;

	Mat output = Mat::zeros(row, col, input.type());
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			float sum1_t = 0.0;
			float sum1_s = 0.0;
			float sum2_s = 0.0;
			float sum2_t = 0.0;
			Mat temp = input;
			if (!strcmp(opt, "zero-paddle")) {
				for (int b = -n; b <= n; b++) {
					// Gaussian filter with Zero-paddle boundary process:
					kernelvalue1 = kernel_t.at<float>(0, b + n);
					if ((i <= row - 1) && (i >= 0) && (j + b <= col - 1) && (j + b >= 0)) { //if the pixel is not a border pixel
						sum1_t += kernelvalue1 * (float)(input.at<G>(i, j + b));
					}
				}
				temp.at<G>(i, j) = (G)sum1_t;

				for (int a = -n; a <= n; a++) {
					// Gaussian filter with Zero-paddle boundary process:
					kernelvalue2 = kernel_s.at<float>(a + n, 0);
					if ((i + a <= row - 1) && (i + a >= 0) && (j <= col - 1) && (j >= 0)) { //if the pixel is not a border pixel
						sum1_s += kernelvalue2 * (float)(temp.at<G>(i + a, j));
					}
				}
				output.at<G>(i, j) = (G)sum1_s;
			}

			else if (!strcmp(opt, "mirroring")) {
				for (int b = -n; b <= n; b++) {
					// Gaussian filter with "mirroring" process:
					kernelvalue1 = kernel_t.at<float>(0, b + n);
					if (j + b > col - 1) {
						tempb = j - b;
					}
					else if (j + b < 0) {
						tempb = -(j + b);
					}
					else {
						tempb = j + b;
					}
					sum1_t += kernelvalue1 * (float)(input.at<G>(i, tempb));
				}
				temp.at<G>(i, j) = (G)sum1_t;

				for (int a = -n; a <= n; a++) {
					// Gaussian filter with "mirroring" process:
					kernelvalue2 = kernel_s.at<float>(a + n, 0);
					if (i + a > row - 1) {
						tempa = i - a;
					}
					else if (i + a < 0) {
						tempa = -(i + a);
					}
					else {
						tempa = i + a;
					}
					sum1_s += kernelvalue2 * (float)(temp.at<G>(tempa, j));
				}
				output.at<G>(i, j) = (G)sum1_s;
			}

			else if (!strcmp(opt, "adjustkernel")) {
				for (int b = -n; b <= n; b++) {
					kernelvalue1 = kernel_t.at<float>(0, b + n);
					// Gaussian filter with "adjustkernel" process:
					if ((j + b <= col - 1) && (j + b >= 0)) {
						sum1_t += kernelvalue1 * (float)(input.at<G>(i, j + b));
						sum2_t += kernelvalue1;
					}
				}
				temp.at<G>(i, j) = (G)(sum1_t / sum2_t);


				for (int a = -n; a <= n; a++) {
					kernelvalue2 = kernel_s.at<float>(a + n, 0);
					// Gaussian filter with "adjustkernel" process:
					if ((i + a <= row - 1) && (i + a >= 0)) {
						sum1_s += kernelvalue2 * (float)(input.at<G>(i + a, j));
						sum2_s += kernelvalue2;
					}
				}
				output.at<G>(i, j) = (G)(sum1_s / sum2_s);
			}
		}
	}
	return output;
}

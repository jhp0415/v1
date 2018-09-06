#include"CommonFunction.h"

using namespace cv;
using namespace std;
#define PI 3.14159265359

//이차원 배열 할당하기
int** IntAlloc2(int width, int height)
{
    int** tmp;
    tmp = (int**)calloc(height, sizeof(int*));
    for (int i = 0; i<height; i++)
        tmp[i] = (int*)calloc(width, sizeof(int));
    return(tmp);
}

//인코딩 파라미터 이차원배열 할당하기
encodingResult** ER_Alloc2(int width, int height)
{
    encodingResult** tmp;
    tmp = (encodingResult**)calloc(height, sizeof(encodingResult*));
    for (int i = 0; i<height; i++)
        tmp[i] = (encodingResult*)calloc(width, sizeof(encodingResult));
    return(tmp);
}

//이차원배열 해제
void IntFree2(int** image, int width, int height)
{
    for (int i = 0; i<height; i++)
        free(image[i]);

    free(image);
}

//인코딩 파라미터 이차원 배열 해제
void ER_Free2(encodingResult** image, int width, int height)
{
    for (int i = 0; i<height; i++)
        free(image[i]);

    free(image);
}

//이미지 로드
int** ReadImage(const char* name, int* width, int* height)
{
    Mat img = imread(name, IMREAD_GRAYSCALE);
    int** image = (int**)IntAlloc2(img.cols, img.rows);

    *width = img.cols;
    *height = img.rows;

    for (int i = 0; i<img.rows; i++)
        for (int j = 0; j<img.cols; j++)
            image[i][j] = img.at<unsigned char>(i, j);

    return(image);
}

//이미지 생성
void WriteImage(char* name, int** image, int width, int height)
{
    Mat img(height, width, CV_8UC1);
    for (int i = 0; i<height; i++)
        for (int j = 0; j<width; j++)
            img.at<unsigned char>(i, j) = (unsigned char)image[i][j];

    imwrite(name, img);
}

//이미지 출력하기
void ImageShow(char* winname, int** image, int width, int height)
{
    Mat img(height, width, CV_8UC1);
    for (int i = 0; i<height; i++)
        for (int j = 0; j<width; j++)
            img.at<unsigned char>(i, j) = (unsigned char)image[i][j];
    imshow(winname, img);
    waitKey(0);
}

//====================================================================================================================================

//이미지를 1/2 축소하기
void Contraction(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < height / 2; i++)
        for (int j = 0; j < width / 2; j++)
            img_out[i][j] = (img_in[i * 2][j * 2] + img_in[i * 2][j * 2 + 1] + img_in[i * 2 + 1][j * 2] + img_in[i * 2 + 1][j * 2 + 1]) / 4;
}

//이미지 복사하기
void IsoM_0(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < height; i++) for (int j = 0; j < width; j++) img_out[i][j] = img_in[i][j];
}

//이미지 좌우대칭 변환
void IsoM_1(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < height; i++) for (int j = 0; j < width; j++) img_out[i][j] = img_in[i][width - 1 - j];
}

//이미지 상하대칭 변환
void IsoM_2(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < height; i++) for (int j = 0; j < width; j++) img_out[i][j] = img_in[height - 1 - i][j];
}

//이미지 y=-x 대칭 변환
void IsoM_3(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < width; i++) for (int j = 0; j < height; j++) img_out[i][j] = img_in[j][i];
}

//이미지 y=x 대칭 변환
void IsoM_4(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < width; i++) for (int j = 0; j < height; j++) img_out[i][j] = img_in[height - 1 - j][width - 1 - i];
}

//오른쪽 90 회전 변환
void IsoM_5(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < width; i++) for (int j = 0; j < height; j++) img_out[i][j] = img_in[height - 1 - j][i];
}

//오른쪽 180 회전 변환
void IsoM_6(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < height; i++) for (int j = 0; j < width; j++) img_out[i][j] = img_in[height - 1 - i][width - 1 - j];
}

//왼쪽 90 회전 변환
void IsoM_7(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < width; i++) for (int j = 0; j < height; j++) img_out[i][j] = img_in[j][width - 1 - i];
}

//대칭 변환 함수
void Isometry(int no, int** img_in, int** img_out, int width, int height)
{
    if (no == 0) IsoM_0(img_in, img_out, width, height);		//복사
    if (no == 1) IsoM_1(img_in, img_out, width, height);		//좌우대칭
    if (no == 2) IsoM_2(img_in, img_out, width, height);		//상하대칭
    if (no == 3) IsoM_3(img_in, img_out, width, height);		//대각선대칭
    if (no == 4) IsoM_4(img_in, img_out, width, height);		//역대각선대칭
    if (no == 5) IsoM_5(img_in, img_out, width, height);		//오른쪽 90 선대칭
    if (no == 6) IsoM_6(img_in, img_out, width, height);		//오른쪽 180 선대칭
    if (no == 7) IsoM_7(img_in, img_out, width, height);		//왼쪽 90 선대칭
    if (!(no >= 0 && no < 8)) printf("no = %d Error\n", no);
}

//이미지에서 블록 읽어오기
void ReadBlock(int** image, int** block, int x, int y, int bsize)
{
    for (int i = y; i < y + bsize; i++)
        for (int j = x; j < x + bsize; j++)
            block[i - y][j - x] = image[i][j];
}

//이미지에 블록을 삽입하기
void WriteBlock(int** image, int** block, int x, int y, int bsize)
{
    for (int i = y; i < y + bsize; i++)
        for (int j = x; j < x + bsize; j++)
            image[i][j] = block[i - y][j - x];
}

//이미지의 평균 계산하기
int ComputeAVG(int** image, int width, int height)
{
    int avg = 0;
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            avg += image[i][j];

    avg = (int)(avg / (width * height) + 0.5);
    return avg;
}

//두 이미지 사이의 에러 계산하기
int ComputeError(int** block, int** image, int bsize, int x, int y)
{
    int error = 0;
    for (int i = 0; i < bsize; i++)
        for (int j = 0; j < bsize; j++)
            error += abs(block[i][j] - image[y + i][x + j]);

    return error;
}

//이미지에 평균 더하기, 뺴기
void AddOrSub_AVG(int** img_in, int** img_out, int bsize, int avg)
{
    for (int i = 0; i < bsize; i++)
        for (int j = 0; j < bsize; j++)
            img_out[i][j] = img_in[i][j] + avg;
}

//이미지에 알파 곱하기
void Multiply_Alpha(int** img_in, int** img_out, int bsize, double alpha)
{
    for (int i = 0; i < bsize; i++)
        for (int j = 0; j < bsize; j++)
            img_out[i][j] = (int)((img_in[i][j] * alpha) + 0.5);
}
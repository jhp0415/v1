#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string>
#include <opencv2/opencv.hpp>   
#include <opencv2/core/core.hpp>   
#include <opencv2/highgui/highgui.hpp>  

using namespace cv;
using namespace std;

#define PI 3.14159265359

//이미지 정보 저장 구조체
struct IMG
{
    int** image;
    int width;
    int height;
};

//인코딩 데이터 저장 구조체
struct encodingResult
{
    int bsize;
    int x, y;
    int geo;//geometric transform
    int avg;
    double alpha;
    struct encodingResult** enData = NULL;
};

//이차원 배열 할당하기
int** IntAlloc2(int width, int height);
//인코딩 파라미터 이차원배열 할당하기
encodingResult** ER_Alloc2(int width, int height);
//이차원배열 해제
void IntFree2(int** image, int width, int height);
//인코딩 파라미터 이차원 배열 해제
void ER_Free2(encodingResult** image, int width, int height);
//이미지 로드
int** ReadImage(const char* name, int* width, int* height);
//이미지 생성
void WriteImage(char* name, int** image, int width, int height);
//이미지 출력하기
void ImageShow(char* winname, int** image, int width, int height);



//이미지를 1/2 축소하기
void Contraction(int** img_in, int** img_out, int width, int height);
//이미지 복사하기
void IsoM_0(int** img_in, int** img_out, int width, int height);
//이미지 좌우대칭 변환
void IsoM_1(int** img_in, int** img_out, int width, int height);
//이미지 상하대칭 변환
void IsoM_2(int** img_in, int** img_out, int width, int height);
//이미지 y=-x 대칭 변환
void IsoM_3(int** img_in, int** img_out, int width, int height);
//이미지 y=x 대칭 변환
void IsoM_4(int** img_in, int** img_out, int width, int height);
//오른쪽 90 회전 변환
void IsoM_5(int** img_in, int** img_out, int width, int height);
//오른쪽 180 회전 변환
void IsoM_6(int** img_in, int** img_out, int width, int height);
//왼쪽 90 회전 변환
void IsoM_7(int** img_in, int** img_out, int width, int height);
//대칭 변환 함수
void Isometry(int no, int** img_in, int** img_out, int width, int height);
//이미지에서 블록 읽어오기
void ReadBlock(int** image, int** block, int x, int y, int bsize);
//이미지에 블록을 삽입하기
void WriteBlock(int** image, int** block, int x, int y, int bsize);
//이미지의 평균 계산하기
int ComputeAVG(int** image, int width, int height);
//두 이미지 사이의 에러 계산하기
int ComputeError(int** block, int** image, int bsize, int x, int y);
//이미지에 평균 더하기, 뺴기
void AddOrSub_AVG(int** img_in, int** img_out, int bsize, int avg);
//이미지에 알파 곱하기
void Multiply_Alpha(int** img_in, int** img_out, int bsize, double alpha);

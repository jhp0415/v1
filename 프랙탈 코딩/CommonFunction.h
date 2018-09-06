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

//�̹��� ���� ���� ����ü
struct IMG
{
    int** image;
    int width;
    int height;
};

//���ڵ� ������ ���� ����ü
struct encodingResult
{
    int bsize;
    int x, y;
    int geo;//geometric transform
    int avg;
    double alpha;
    struct encodingResult** enData = NULL;
};

//������ �迭 �Ҵ��ϱ�
int** IntAlloc2(int width, int height);
//���ڵ� �Ķ���� �������迭 �Ҵ��ϱ�
encodingResult** ER_Alloc2(int width, int height);
//�������迭 ����
void IntFree2(int** image, int width, int height);
//���ڵ� �Ķ���� ������ �迭 ����
void ER_Free2(encodingResult** image, int width, int height);
//�̹��� �ε�
int** ReadImage(const char* name, int* width, int* height);
//�̹��� ����
void WriteImage(char* name, int** image, int width, int height);
//�̹��� ����ϱ�
void ImageShow(char* winname, int** image, int width, int height);



//�̹����� 1/2 ����ϱ�
void Contraction(int** img_in, int** img_out, int width, int height);
//�̹��� �����ϱ�
void IsoM_0(int** img_in, int** img_out, int width, int height);
//�̹��� �¿��Ī ��ȯ
void IsoM_1(int** img_in, int** img_out, int width, int height);
//�̹��� ���ϴ�Ī ��ȯ
void IsoM_2(int** img_in, int** img_out, int width, int height);
//�̹��� y=-x ��Ī ��ȯ
void IsoM_3(int** img_in, int** img_out, int width, int height);
//�̹��� y=x ��Ī ��ȯ
void IsoM_4(int** img_in, int** img_out, int width, int height);
//������ 90 ȸ�� ��ȯ
void IsoM_5(int** img_in, int** img_out, int width, int height);
//������ 180 ȸ�� ��ȯ
void IsoM_6(int** img_in, int** img_out, int width, int height);
//���� 90 ȸ�� ��ȯ
void IsoM_7(int** img_in, int** img_out, int width, int height);
//��Ī ��ȯ �Լ�
void Isometry(int no, int** img_in, int** img_out, int width, int height);
//�̹������� ��� �о����
void ReadBlock(int** image, int** block, int x, int y, int bsize);
//�̹����� ����� �����ϱ�
void WriteBlock(int** image, int** block, int x, int y, int bsize);
//�̹����� ��� ����ϱ�
int ComputeAVG(int** image, int width, int height);
//�� �̹��� ������ ���� ����ϱ�
int ComputeError(int** block, int** image, int bsize, int x, int y);
//�̹����� ��� ���ϱ�, ����
void AddOrSub_AVG(int** img_in, int** img_out, int bsize, int avg);
//�̹����� ���� ���ϱ�
void Multiply_Alpha(int** img_in, int** img_out, int bsize, double alpha);

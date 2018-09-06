#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include <opencv2/opencv.hpp>   
#include <opencv2/core/core.hpp>   
#include <opencv2/highgui/highgui.hpp>  

using namespace cv;

#define PI 3.14159265359
typedef struct
{
    int tp[8][8];
}Temp;


int** IntAlloc2(int width, int height)
{
    int** tmp;
    tmp = (int**)calloc(height, sizeof(int*));
    for (int i = 0; i<height; i++)
        tmp[i] = (int*)calloc(width, sizeof(int));
    return(tmp);
}

void IntFree2(int** image, int width, int height)
{
    for (int i = 0; i<height; i++)
        free(image[i]);

    free(image);
}

int** ReadImage(char* name, int* width, int* height)
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

void WriteImage(char* name, Temp* image, int width, int height)
{
    Mat img(height, width, CV_8UC1);
    for (int i = 0; i<height; i++)
        for (int j = 0; j<width; j++)
            img.at<unsigned char>(i, j) = image->tp[i][j];

    imwrite(name, img);
}

void ImageShow(char* winname, int** image, int width, int height)
{
    Mat img(height, width, CV_8UC1);
    for (int i = 0; i<height; i++)
        for (int j = 0; j<width; j++)
            img.at<unsigned char>(i, j) = (unsigned char)image[i][j];
    imshow(winname, img);

}



void Mosaic()
{
    Mat image = imread("sample.jpg", IMREAD_GRAYSCALE); //�̹��� �о����
    int width = image.cols;        //���α���
    int height = image.rows;       //���α���

    int size = width / 8 * height / 8;
    //Temp* TTT = new Temp[size];

    //printf("�Ϸ�\n");

   /* for(int k=0;k<size;k++)
        for (int i = 0; i < height / 8; i++)
        for (int j = 0; j < width / 8; j++)
        {
            for (int p = 0; p < 8; p++)
                for (int q = 0; q < 8; q++)
                    (TTT+k)->tp[p][q] = image.at<unsigned char>(i * 8 + p, j * 8 + q);
            WriteImage("k", (TTT+k), 8, 8);
        }*/
    printf("�Ϸ�\n");

}



void main()
{
        /*�����̹����� 8x8������� �߶� ���� �̹������ ����
        �ι�° �̹������� ���� �̹����� ����� �κ��� ã�Ƴ��� ���� �̹����� �����δ�.
        ����ü typedef ����ϱ�*/
    Mosaic();
}
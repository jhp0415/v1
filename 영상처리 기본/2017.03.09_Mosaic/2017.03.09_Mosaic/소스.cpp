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

int** IntAlloc2(int width, int height);
void IntFree2(int** image, int width, int height);
int** ReadImage(char* name, int* width, int* height);
void WriteImage(char* name, Temp* image, int width, int height);
void ImageShow(char* winname, int** image, int width, int height);
void Mosaic();

//------------------------------------------------------------------------------------
void main()
{
    /*원본이미지를 8x8사이즈로 잘라내 작은 이미지들로 저장
    두번째 이미지에서 작은 이미지와 비슷한 부분을 찾아내어 작은 이미지를 덮분인다.
    구조체 typedef 사용하기*/
    //Mosaic();
    printf("완료\n");
}
//------------------------------------------------------------------------------------

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
//-----------------------------------------------------------------------------------------------------------


void Mosaic()
{
    Mat image = imread("Sample.jpg", IMREAD_GRAYSCALE); //이미지 읽어오기
    int width = image.cols;        //가로길이
    int height = image.rows;       //세로길이

    //int** img = (int**)IntAlloc2(width, height);
    int size = width / 8 * height / 8;
    //Temp* TTT = new Temp[size];

    printf("완료\n");

    /*for(int k=0;k<size;k++)
        for (int i = 0; i < height / 8; i++)
            for (int j = 0; j < width / 8; j++)
            {
                for (int p = 0; p < 8; p++)
                    for (int q = 0; q < 8; q++)
                        (TTT+k)->tp[p][q] = image.at<unsigned char>(i * 8 + p, j * 8 + q);
                WriteImage("k", (TTT+k), 8, 8);
            }
            */

}
  


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include <opencv2/opencv.hpp>   
#include <opencv2/core/core.hpp>   
#include <opencv2/highgui/highgui.hpp>  

using namespace cv;

#define PI 3.14159265359

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

void WriteImage(char* name, int** image, int width, int height)
{
    Mat img(height, width, CV_8UC1);
    for (int i = 0; i<height; i++)
        for (int j = 0; j<width; j++)
            img.at<unsigned char>(i, j) = (unsigned char)image[i][j];

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

void ConvertMat2Image(Mat& img_mat, int** image_out, int* width, int* height)
{
    Mat img_mat_gray;

    if (img_mat.channels() != 1)
    {
        cvtColor(img_mat, img_mat_gray, COLOR_BGR2GRAY);
        printf("\n BGR to gray was converted!");
    }
    else
        img_mat_gray = img_mat.clone();

    *width = img_mat.cols;
    *height = img_mat.rows;
    for (int i = 0; i < img_mat.rows; i++) for (int j = 0; j < img_mat.cols; j++)
        image_out[i][j] = img_mat_gray.at<unsigned char>(i, j);
}

void Scale_UP(float num)
{
    int size, tmp = num * 10;
    if (tmp % 10 <5) size = num / 1;
    else size = num / 1 + 1;
    
    Mat img_in = imread("sample.jpg", IMREAD_GRAYSCALE); //이미지 읽어오기
    int width = img_in.cols;        //가로길이
    int height = img_in.rows;       //세로길이

    int** img = (int**)IntAlloc2(width, height);  //배열 생성
    int** arr1 = (int**)IntAlloc2(width*size, height*size);  //변환배열 생성
    int** arr2 = (int**)IntAlloc2(width*size, height*size);
    img = ReadImage("sample.jpg", &width, &height);

    //가로먼저 확대
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            for (int k = 0; k < size; k++)
                arr1[i][j*size + k] = img[i][j] + ((double)(k / size)*(img[i][j + 1] - img[i][j]));
    //그다음 세로확대
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width*size; j++)
            for (int k = 0; k < size; k++)
                arr2[i*size + k][j] = arr1[i][j] + ((double)(k / size)*(arr1[i + 1][j] - arr1[i][j]));

    ImageShow("out window", arr2, width*size, height*size);
    WriteImage("Up.jpg", arr2, width*size, height*size);

    imshow("input window", img_in);
    waitKey(0);
}

void Scale_DOWN(float num)
{
    int size, tmp=num*10;
    if (tmp%10 <5) size = num / 1;
    else size = num / 1 + 1;
    
    Mat img_in = imread("sample.jpg", IMREAD_GRAYSCALE); //이미지 읽어오기
    int width = img_in.cols;        //가로길이
    int height = img_in.rows;       //세로길이

    int** img = (int**)IntAlloc2(width, height);  //배열 생성
    int** arr1 = (int**)IntAlloc2(width / size, height / size);  //변환배열 생성
    img = ReadImage("sample.jpg", &width, &height);       //이미지 읽어오기


    for (int i = 0; i < height / size; i++)
        for (int j = 0; j < width / size; j++)
            for (int k = 0; k < size; k++)
                arr1[i][j] = img[i*size][j*size];


    ImageShow("out window", arr1, width / size, height / size);
    WriteImage("Down.jpg", arr1, width / size, height / size);

    imshow("input window", img_in);
    waitKey(0);
}

void Rotation(double ang)
{
    Mat img_in = imread("xxxx.jpg", IMREAD_GRAYSCALE); //이미지 읽어오기
    int width = img_in.cols;        //가로길이
    int height = img_in.rows;       //세로길이

    int** img = (int**)IntAlloc2(width, height);  
    img = ReadImage("xxxx.jpg", &width, &height);

    int width2 = height*cos((90 * PI / 180) - ang) + width*cos(ang);
    int height2 = height*cos(ang) + width*cos((90 * PI / 180) - ang);
    int** arr1 = (int**)IntAlloc2(width2, height2);     //최대사이즈 배열
    
    int Cx = width2 / 2;        //영상의 중심점
    int Cy = height2 / 2;
    int x = 0, y = 0;
    for (int i = 0; i < height2; i++)
        for (int j = 0; j < width2; j++)
        {
            x = (j - Cx)*cos(ang) - (i - Cy)*sin(ang) + width/2;
            y = (j - Cx)*sin(ang) + (i - Cy)*cos(ang) + height/2;

            if (x>0 && x<width && y>0 && y<height)
                arr1[i][j] = img[y][x];
        }

    ImageShow("out window", arr1, width2, height2);
    WriteImage("Rotate.jpg", arr1, width2, height2);
    imshow("input window", img_in);
    waitKey(0);
}

void Rotation_color(double ang)
{
    Mat img_in = imread("xxxx.jpg", IMREAD_COLOR); //이미지 읽어오기
    int width = img_in.cols;        //가로길이
    int height = img_in.rows;       //세로길이
    
    int width2 = height*cos((90 * PI / 180) - ang) + width*cos(ang);
    int height2 = height*cos(ang) + width*cos((90 * PI / 180) - ang);
    Mat image_out(height2, width2, CV_8UC3);
    int** arr1 = (int**)IntAlloc2(width2, height2);     //최대사이즈 배열

    int Cx = width2 / 2;        //영상의 중심점
    int Cy = height2 / 2;
    int x = 0, y = 0;

    for (int i = 0; i < height2; i++)
        for (int j = 0; j < width2; j++)
        {
            x = (j - Cx)*cos(ang) - (i - Cy)*sin(ang) + width / 2;
            y = (j - Cx)*sin(ang) + (i - Cy)*cos(ang) + height / 2;

            if (x >= 0 && x < width && y>=0 && y < height)
            {
                image_out.at<Vec3b>(i, j)[0] = img_in.at<Vec3b>(y, x)[0]; // b
                image_out.at<Vec3b>(i, j)[1] = img_in.at<Vec3b>(y, x)[1]; // g
                image_out.at<Vec3b>(i, j)[2] = (img_in.at<Vec3b>(y, x)[2] + 50) >255 ? 255 : (img_in.at<Vec3b>(y, x)[2] + 50); // r
            }
        }

    imshow("input window", img_in);
    imshow("output window", image_out);
    imwrite("output_img.jpg", image_out);
    waitKey(0);
}

void main()
{
    Scale_UP(3.7);
    //Scale_DOWN(2.7);
    //Rotation((30 * PI / 180));
    //Rotation((45 * PI / 180));
    //Rotation_color((30 * PI / 180));
}
/*�����̹����� 8x8������� �߶� ���� �̹������ ����|
�ι�° �̹������� ���� �̹����� ����� �κ��� ã�Ƴ��� ���� �̹����� �����δ�.
����ü typedef ����ϱ�*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include <opencv2/opencv.hpp>   
#include <opencv2/core/core.hpp>   
#include <opencv2/highgui/highgui.hpp>  

#include<direct.h>
#include<string.h>
#include<Windows.h>;

using namespace cv;
using namespace std;

#define PI 3.14159265359
#define length 8
typedef struct
{
    int index = 0;
    int tp[length][length];
}Temp;

String path;

char FolderPath[_MAX_PATH] = { 0, };

int Extract_Color();
Temp* Read_Piece(int s);
void Compare_Color(int img_size);

//---------------------------------------------------------

int** IntAlloc2(int width, int height)
{
    int** tmp;
    tmp = (int**)calloc(height, sizeof(int*));
    for (int i = 0; i<height; i++)
        tmp[i] = (int*)calloc(width, sizeof(int));
    return(tmp);
}

void ReadImage(int num, Temp* image, int width, int height)
{
    path = FolderPath + to_string(num) + ".jpg";
    Mat img = imread(path, IMREAD_COLOR);

    for (int i = 0; i< height; i++)
        for (int j = 0; j < width; j++)
        {
            image->tp[i][j] = img.at<unsigned char>(i, j);
            //image_out.at<Vec3b>(i, j)[0] = img_in.at<Vec3b>(y, x)[0]; // b
            //image_out.at<Vec3b>(i, j)[2] = img_in.at<Vec3b>(y, x)[2]; // r
            //image_out.at<Vec3b>(i, j)[1] = img_in.at<Vec3b>(y, x)[1]; // g
        }
}

void ImageShow(int num, Temp* image, int width, int height)
{
    Mat img(height, width, CV_8UC3);

    for (int i = 0; i<height; i++)
        for (int j = 0; j<width; j++)
            img.at<unsigned char>(i, j) = image->tp[i][j];

    path = to_string(num) + ".jpg";
    imshow(path, img);
    waitKey(1);
}

void WriteImage(int num, Temp* image, int width, int height)
{
    Mat img(height, width, CV_8UC3);
    for (int i = 0; i<height; i++)
        for (int j = 0; j<width; j++)
            img.at<unsigned char>(i, j) = (unsigned char)image->tp[i][j];

    path = FolderPath + to_string(num) + ".jpg";
    imwrite(path, img);
}

void WriteImage_Mosaic(char* str, int size, Temp* image, int width, int height)
{
    Mat img(height, width, CV_8UC3);
    printf("�̹��� ��ġ�� ����\n");
    int k = 0;
    for (int m = 0; m < height / length; m++)
        for (int n = 0; n < width / length; n++)
        {
            int peice_number = (image + (k))->index;
            for (int p = 0; p < length; p++) for (int q = 0; q < length; q++)
                img.at<unsigned char>(m*length + p, n*length + q) = (unsigned char)(image + peice_number)->tp[p][q];

            k++;
        }
    printf("�̹��� ��ġ�� �Ϸ�\n");

    imwrite(str, img);
    imshow("Mosaic_Image.jpg", img);
    waitKey(0);
}

//------------------------------------------------------------------------------------
void main()
{
    getcwd(FolderPath, _MAX_PATH);      //���� ���
    strcat(FolderPath, "\\Template\\");
    mkdir(FolderPath);      //���ø� ������ ���� ����

    int image_size;         //������ ����
    image_size = Extract_Color();             //�̹��� �߶� �����ϱ�
    Compare_Color(image_size);

    system("Pause");
}
//------------------------------------------------------------------------------------

//image_out.at<Vec3b>(i, j)[0] = img_in.at<Vec3b>(y, x)[0]; // b
//image_out.at<Vec3b>(i, j)[1] = img_in.at<Vec3b>(y, x)[1]; // g
//image_out.at<Vec3b>(i, j)[2] = img_in.at<Vec3b>(y, x)[2]; // r

int Extract_Color()
{
    Mat image = imread("Sample03.jpg", IMREAD_COLOR); //�̹��� �о����
    int width = image.cols;        //���α���
    int height = image.rows;       //���α���


    int size = (width / length) * (height / length);
    printf("������ ����:%d\n", size);
    Temp* TTT = new Temp[size];

    int k = 0;
    for (int i = 0; i < height / length; i++)
        for (int j = 0; j < width / length; j++)
        {
            for (int p = 0; p < length; p++)
                for (int q = 0; q < length; q++)
                    (TTT + k)->tp[p][q] = image.at<unsigned char>(i * length + p, j * length + q);
            WriteImage(k, (TTT + k), length, length);
            k++;
        }
    free(TTT);
    printf("%d\n ", k);
    return size;
}


Temp* Read_Piece(int size)
{
    //���� �� �ҷ�����->���� �迭 ����                           
    //���� ���� ��ձ��س���                                          
    //�����ϱ�->���ο� �ּ������ϴ� �迭 �����Ͽ� ����               
    //���ο� �̹��� ���� �ҷ�����                               
    Temp* CCC = new Temp[size];

    printf("�б� ����\n");
    for (int k = 0; k < size; k++)
        ReadImage(k, (CCC + k), length, length);

    printf("�о���̱� �Ϸ�\n");
    return CCC;
}

void Compare_Color(int pic_size)
{
    //����� ���� �̹��� �ҷ��� ����ü�� ����
    Temp* PPP = new Temp[pic_size];
    PPP = Read_Piece(pic_size);
    //���� �̹��� �ҷ�����
    Mat image = imread("compare.jpg", IMREAD_COLOR); //�̹��� �о����
    int width = image.cols;        //���α���
    int height = image.rows;       //���α���
    int com_size = (width / length) * (height / length);
    //���� �̹����� �������� ����ü�� �����ϱ�
    Temp* AAA = new Temp[com_size];

    int k = 0;
    for (int i = 0; i < height / length; i++)
        for (int j = 0; j < width / length; j++)
        {
            for (int p = 0; p < length; p++)
                for (int q = 0; q < length; q++)
                    (AAA + k)->tp[p][q] = image.at<unsigned char>(i * length + p, j * length + q);
            k++;
        }



    for (int i = 0; i < com_size; i++)
        AAA[i].index = i;

    for (int m = 0; m < com_size; m++)
    {
        int min_err = INT_MAX;
        int opt_k;
        for (int k = 0; k < pic_size; k++)
        {
            int error = 0;
            for (int i = 0; i < length; i++)
                for (int j = 0; j < length; j++)
                    error += abs((AAA + m)->tp[i][j] - (PPP + k)->tp[i][j]);
            if (error < min_err)
            {
                min_err = error;
                opt_k = k;
                (PPP + m)->index = k;
            }
        }
        printf("%d��° ���̹����� ���� ����� �̹��� ����:%d \n", m, opt_k);
    }
    WriteImage_Mosaic("Mosaic_Image.jpg", com_size, PPP, (width / length)*length, (height / length)*length);
}

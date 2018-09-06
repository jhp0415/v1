#include"CommonFunction.h"

using namespace cv;
using namespace std;
#define PI 3.14159265359

//������ �迭 �Ҵ��ϱ�
int** IntAlloc2(int width, int height)
{
    int** tmp;
    tmp = (int**)calloc(height, sizeof(int*));
    for (int i = 0; i<height; i++)
        tmp[i] = (int*)calloc(width, sizeof(int));
    return(tmp);
}

//���ڵ� �Ķ���� �������迭 �Ҵ��ϱ�
encodingResult** ER_Alloc2(int width, int height)
{
    encodingResult** tmp;
    tmp = (encodingResult**)calloc(height, sizeof(encodingResult*));
    for (int i = 0; i<height; i++)
        tmp[i] = (encodingResult*)calloc(width, sizeof(encodingResult));
    return(tmp);
}

//�������迭 ����
void IntFree2(int** image, int width, int height)
{
    for (int i = 0; i<height; i++)
        free(image[i]);

    free(image);
}

//���ڵ� �Ķ���� ������ �迭 ����
void ER_Free2(encodingResult** image, int width, int height)
{
    for (int i = 0; i<height; i++)
        free(image[i]);

    free(image);
}

//�̹��� �ε�
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

//�̹��� ����
void WriteImage(char* name, int** image, int width, int height)
{
    Mat img(height, width, CV_8UC1);
    for (int i = 0; i<height; i++)
        for (int j = 0; j<width; j++)
            img.at<unsigned char>(i, j) = (unsigned char)image[i][j];

    imwrite(name, img);
}

//�̹��� ����ϱ�
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

//�̹����� 1/2 ����ϱ�
void Contraction(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < height / 2; i++)
        for (int j = 0; j < width / 2; j++)
            img_out[i][j] = (img_in[i * 2][j * 2] + img_in[i * 2][j * 2 + 1] + img_in[i * 2 + 1][j * 2] + img_in[i * 2 + 1][j * 2 + 1]) / 4;
}

//�̹��� �����ϱ�
void IsoM_0(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < height; i++) for (int j = 0; j < width; j++) img_out[i][j] = img_in[i][j];
}

//�̹��� �¿��Ī ��ȯ
void IsoM_1(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < height; i++) for (int j = 0; j < width; j++) img_out[i][j] = img_in[i][width - 1 - j];
}

//�̹��� ���ϴ�Ī ��ȯ
void IsoM_2(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < height; i++) for (int j = 0; j < width; j++) img_out[i][j] = img_in[height - 1 - i][j];
}

//�̹��� y=-x ��Ī ��ȯ
void IsoM_3(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < width; i++) for (int j = 0; j < height; j++) img_out[i][j] = img_in[j][i];
}

//�̹��� y=x ��Ī ��ȯ
void IsoM_4(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < width; i++) for (int j = 0; j < height; j++) img_out[i][j] = img_in[height - 1 - j][width - 1 - i];
}

//������ 90 ȸ�� ��ȯ
void IsoM_5(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < width; i++) for (int j = 0; j < height; j++) img_out[i][j] = img_in[height - 1 - j][i];
}

//������ 180 ȸ�� ��ȯ
void IsoM_6(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < height; i++) for (int j = 0; j < width; j++) img_out[i][j] = img_in[height - 1 - i][width - 1 - j];
}

//���� 90 ȸ�� ��ȯ
void IsoM_7(int** img_in, int** img_out, int width, int height)
{
    for (int i = 0; i < width; i++) for (int j = 0; j < height; j++) img_out[i][j] = img_in[j][width - 1 - i];
}

//��Ī ��ȯ �Լ�
void Isometry(int no, int** img_in, int** img_out, int width, int height)
{
    if (no == 0) IsoM_0(img_in, img_out, width, height);		//����
    if (no == 1) IsoM_1(img_in, img_out, width, height);		//�¿��Ī
    if (no == 2) IsoM_2(img_in, img_out, width, height);		//���ϴ�Ī
    if (no == 3) IsoM_3(img_in, img_out, width, height);		//�밢����Ī
    if (no == 4) IsoM_4(img_in, img_out, width, height);		//���밢����Ī
    if (no == 5) IsoM_5(img_in, img_out, width, height);		//������ 90 ����Ī
    if (no == 6) IsoM_6(img_in, img_out, width, height);		//������ 180 ����Ī
    if (no == 7) IsoM_7(img_in, img_out, width, height);		//���� 90 ����Ī
    if (!(no >= 0 && no < 8)) printf("no = %d Error\n", no);
}

//�̹������� ��� �о����
void ReadBlock(int** image, int** block, int x, int y, int bsize)
{
    for (int i = y; i < y + bsize; i++)
        for (int j = x; j < x + bsize; j++)
            block[i - y][j - x] = image[i][j];
}

//�̹����� ����� �����ϱ�
void WriteBlock(int** image, int** block, int x, int y, int bsize)
{
    for (int i = y; i < y + bsize; i++)
        for (int j = x; j < x + bsize; j++)
            image[i][j] = block[i - y][j - x];
}

//�̹����� ��� ����ϱ�
int ComputeAVG(int** image, int width, int height)
{
    int avg = 0;
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            avg += image[i][j];

    avg = (int)(avg / (width * height) + 0.5);
    return avg;
}

//�� �̹��� ������ ���� ����ϱ�
int ComputeError(int** block, int** image, int bsize, int x, int y)
{
    int error = 0;
    for (int i = 0; i < bsize; i++)
        for (int j = 0; j < bsize; j++)
            error += abs(block[i][j] - image[y + i][x + j]);

    return error;
}

//�̹����� ��� ���ϱ�, ����
void AddOrSub_AVG(int** img_in, int** img_out, int bsize, int avg)
{
    for (int i = 0; i < bsize; i++)
        for (int j = 0; j < bsize; j++)
            img_out[i][j] = img_in[i][j] + avg;
}

//�̹����� ���� ���ϱ�
void Multiply_Alpha(int** img_in, int** img_out, int bsize, double alpha)
{
    for (int i = 0; i < bsize; i++)
        for (int j = 0; j < bsize; j++)
            img_out[i][j] = (int)((img_in[i][j] * alpha) + 0.5);
}
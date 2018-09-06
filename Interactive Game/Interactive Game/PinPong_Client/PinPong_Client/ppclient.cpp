//����ó�� ���
#include <math.h>
#include <malloc.h>
#include <opencv2/opencv.hpp>   
#include <opencv2/core/core.hpp>   
#include <opencv2/highgui/highgui.hpp>  
#include<Windows.h>
//������
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")
using namespace cv;
using namespace std;

#define BUF_SIZE 100
#define NAME_SIZE 20

unsigned WINAPI HandleBall(void* arg);
unsigned WINAPI SendMsg(void* arg);//������ �����Լ�
unsigned WINAPI RecvMsg(void* arg);//������ �����Լ�
void ErrorHandling(char* msg);
void SendToMsg(void* arg);
void Circle(Mat* img);
double Update_Ball(Mat* frame, float* rho, float* theta, void* arg);
HANDLE hMutex;//���ؽ�


char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

#define PI 3.14159265359
#define radius 25

#define SQ(x) ((x)*(x))
float v;                 //�ʱ�ӵ�      
float Arg;       //�ʱⰢ��
float X;
float Y;
float opt_rho;
float opt_theta;
#define Width 960
#define Height 720

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET sock;
    SOCKADDR_IN serverAddr;
    HANDLE sendThread, recvThread, ballThread;
    if (argc != 4)
    {
        printf("Usage : %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// ������ ������ ����Ѵٰ� �ü���� �˸�
        ErrorHandling("WSAStartup() error!");

    sprintf(name, "[%s]", argv[3]); // ������̸� ����


    hMutex = CreateMutex(NULL, FALSE, NULL);//�ϳ��� ���ؽ��� �����Ѵ�.                                       /*���Ͽ� �ּ��Ҵ�*/
    sock = socket(PF_INET, SOCK_STREAM, 0);//������ �ϳ� �����Ѵ�.
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddr.sin_port = htons(atoi(argv[2]));


    /*�����û*/
    if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)//������ �����Ѵ�.
        ErrorHandling("connect() error");



    sendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&sock, 0, NULL);
    ////�޽��� ���ۿ� �����尡 ����ȴ�.
    ballThread = (HANDLE)_beginthreadex(NULL, 0, HandleBall, (void*)&sock, 0, NULL);
    recvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&sock, 0, NULL);//�޽��� ���ſ� �����尡 ����ȴ�.


    WaitForSingleObject(sendThread, INFINITE);//���ۿ� �����尡 �����ɶ����� ��ٸ���./
    WaitForSingleObject(ballThread, INFINITE);
    WaitForSingleObject(recvThread, INFINITE);//���ſ� �����尡 �����ɶ����� ��ٸ���.

    closesocket(sock);//������ �����Ѵ�.
    WSACleanup();//������ ���� ��������� �ü���� �˸���.
    return 0;
}


unsigned WINAPI SendMsg(void* arg)
{//���ۿ� �������Լ�
    SOCKET sock = *((SOCKET*)arg);//������ ������ �����Ѵ�.
                                  //char Msg[BUF_SIZE];
    try
    {
        while (1)
        {//�ݺ�
            WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
            String rhotheta = to_string(opt_rho) + " " + to_string(opt_theta);
            strcpy(msg, rhotheta.c_str());
            send(sock, msg, strlen(msg), 0);//nameMsg�� �������� �����Ѵ�.
            ReleaseMutex(hMutex);//���ؽ� ����
            Sleep(30);
        }
        //Sleep(30);
        return 0;
    }
    catch (Exception e)
    {
        ErrorHandling("SendMsg ����");
    }
}


void SendToMsg(void* arg)
{//���ۿ� �������Լ�
    SOCKET sock = *((SOCKET*)arg);//������ ������ �����Ѵ�.

    WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
    String rhotheta = to_string(opt_rho) + " " + to_string(opt_theta);
    strcpy(msg, rhotheta.c_str());
    send(sock, msg, strlen(msg), 0);//nameMsg�� �������� �����Ѵ�.
    ReleaseMutex(hMutex);//���ؽ� ����
                         //Sleep(30);

}


unsigned WINAPI RecvMsg(void* arg)
{
    SOCKET sock = *((SOCKET*)arg);//������ ������ �����Ѵ�.
    char Msg[BUF_SIZE];
    int strLen;
    char recvX[10], recvY[10];

    try
    {
        while (1)
        {


            strLen = recv(sock, Msg, BUF_SIZE - 1, 0);//�����κ��� �޽����� �����Ѵ�.
            if (strLen == -1)
            {
                ErrorHandling("�Է¹��� �޼����� �����ϴ�.");
                return -1;
            }
            Msg[strLen] = 0;//���ڿ��� ���� �˸��� ���� ����
            fputs(Msg, stdout);//�ڽ��� �ֿܼ� ���� �޽����� ����Ѵ�.
            fputs("\n", stdout);

            WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����                           
            strcpy(recvX, strtok(Msg, " "));
            strcpy(recvY, strtok(NULL, " "));

            X = atof(recvX);
            Y = atof(recvY);
            ReleaseMutex(hMutex);//���ؽ� ����
            Sleep(30);
        }
        return 0;
    }
    catch (Exception e)
    {
        ErrorHandling("RecvMsg ����");
    }
}


unsigned WINAPI HandleBall(void* arg)
{

    int LowH = 70;
    int HighH = 130;

    int LowS = 100;
    int HighS = 255;

    int LowV = 50;
    int HighV = 255;

    int count = 0;

    VideoCapture cap(0); // open the default camera
    if (!cap.isOpened())  // check if we succeeded
        return -1;
    cap.set(CV_CAP_PROP_FRAME_WIDTH, Width);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, Height);
    Mat edges;
    float rho = 0.0;
    float theta = 0.0;
    //namedWindow("edges", 1);
    do
    {
        WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����  
                                              //lock = true;
        Mat frame;
        cap >> frame; // get a new frame from camera
        flip(frame, frame, 1);


        cvtColor(frame, edges, COLOR_BGR2GRAY);

        int width = frame.cols;
        int height = frame.rows;
        Mat img_color; //�󺧸��� �̹���
                       //HSV�� ��ȯ
        cvtColor(frame, img_color, COLOR_RGB2HSV);
        Mat img_binary;

        inRange(img_color, Scalar(LowH, LowS, LowV), Scalar(HighH, HighS, HighV), img_binary);

        erode(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        dilate(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));


        //morphological closing ������ ���� �޿�� 
        dilate(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
        erode(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));

        // ĳ�� �˰��� ����
        Mat contours;
        if (count % 2 == 0) //count�� �������Ѽ� ����� �ӵ��� �����ؾ��Ѵ� ���� frame��µɶ����� ��µǰ� �������������
            Canny(img_binary, contours, 125, 350);
        if (count == 100)
            count = 0;
        count++;
        std::vector<cv::Vec2f> lines;
        lines.reserve(1000);

        HoughLines(contours, lines, 1, PI / 180, 60); //(��������,�������,���,�ػ�,����,�Ӱ谪(��ġ�°� �̺��� ���ƾ� ��������))


        opt_rho = 0.0;
        opt_theta = 0.0;

        //Point opt_pt1,opt_pt2;
        // ������ �࿡�� �ش� ���� ������

        Mat result(contours.rows, contours.cols, CV_8U, Scalar(255));
        std::vector<Vec2f>::const_iterator it = lines.begin();
        while (it != lines.end())
        {
            rho = (*it)[0];   // ù ��° ��Ҵ� rho �Ÿ�
            theta = (*it)[1]; // �� ��° ��Ҵ� ��Ÿ ����
                              //printf("%d", rho);
            if (theta < PI / 4. || theta > 3.*PI / 4.)
            { // ���� ��
                opt_rho = rho;
                opt_theta = theta;
            }
            ++it;
        }
        Point pt1(opt_rho / cos(opt_theta), 0); // ù �࿡�� �ش� ���� ������   //point��2d��ǥ�� ǥ�����ִ� Ŭ����
        Point pt2((opt_rho - result.rows*sin(opt_theta)) / cos(opt_theta), result.rows);
        line(frame, pt1, pt2, Scalar(255), 2); // �Ͼ� ������ �׸���
        fputs("1\n", stdout);

        fputs("2\n", stdout);
        X = X + v*cos(Arg);
        Y = Y + v*sin(Arg);
        Arg = Update_Ball(&frame, &opt_rho, &opt_theta, arg);
        printf("%f, %f\n", X, Y);
        fputs("4\n", stdout);

        imshow("frame", frame);
        if (waitKey(30) >= 0) break;


        ReleaseMutex(hMutex);//���ؽ� ����
        Sleep(30);

    }
    while (true);
    return 0;

}

double Update_Ball(Mat* frame, float* rho, float* theta, void* arg)
{
    //int Width = frame->cols;
    //int Height = frame->rows;

    if ((X >= radius && X <= Width - radius) && (Y >= radius && Y <= Height - radius))
    {//������ �¾�����
        if ((X*cos(*theta) + Y*sin(*theta) >= *rho - radius) && (X*cos(*theta) + Y*sin(*theta) <= *rho + radius))
        {
            fputs("3\n", stdout);
            SendToMsg(arg);
            Circle(frame);
            Arg = PI / 2 - 2 * Arg;
        }
        else Circle(frame);
    }
    else if ((X > 0 && X <= radius) && (Y > 0 && Y < Height))//���� �� ������
    {
        Circle(frame);
        Arg = PI - Arg;
    }
    else if (X >= Width - radius && X < Width && Y > 0 && Y < Height) //������ ��������
    {
        Circle(frame);
        Arg = PI - Arg;
    }
    else if (X > 0 && X < Width && Y > 0 && Y <= radius) //���ʺ�������
    {
        Circle(frame);
        Arg = -Arg;
    }
    else if (X > 0 && X < Width && Y < Height && Y >= Height - radius) //�Ʒ��� �� ������
    {
        Circle(frame);
        Arg = -Arg;
    }
    return Arg;
}

void Circle(Mat* img)
{
    circle(*img, Point(X, Y), radius, Scalar(0, 0, 0), -1);
}
void ErrorHandling(char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
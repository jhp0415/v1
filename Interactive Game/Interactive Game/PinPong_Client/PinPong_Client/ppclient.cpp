//영상처리 헤더
#include <math.h>
#include <malloc.h>
#include <opencv2/opencv.hpp>   
#include <opencv2/core/core.hpp>   
#include <opencv2/highgui/highgui.hpp>  
#include<Windows.h>
//통신헤더
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
unsigned WINAPI SendMsg(void* arg);//쓰레드 전송함수
unsigned WINAPI RecvMsg(void* arg);//쓰레드 수신함수
void ErrorHandling(char* msg);
void SendToMsg(void* arg);
void Circle(Mat* img);
double Update_Ball(Mat* frame, float* rho, float* theta, void* arg);
HANDLE hMutex;//뮤텍스


char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

#define PI 3.14159265359
#define radius 25

#define SQ(x) ((x)*(x))
float v;                 //초기속도      
float Arg;       //초기각도
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
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// 윈도우 소켓을 사용한다고 운영체제에 알림
        ErrorHandling("WSAStartup() error!");

    sprintf(name, "[%s]", argv[3]); // 사용자이름 저장


    hMutex = CreateMutex(NULL, FALSE, NULL);//하나의 뮤텍스를 생성한다.                                       /*소켓에 주소할당*/
    sock = socket(PF_INET, SOCK_STREAM, 0);//소켓을 하나 생성한다.
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddr.sin_port = htons(atoi(argv[2]));


    /*연결요청*/
    if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)//서버에 접속한다.
        ErrorHandling("connect() error");



    sendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&sock, 0, NULL);
    ////메시지 전송용 쓰레드가 실행된다.
    ballThread = (HANDLE)_beginthreadex(NULL, 0, HandleBall, (void*)&sock, 0, NULL);
    recvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&sock, 0, NULL);//메시지 수신용 쓰레드가 실행된다.


    WaitForSingleObject(sendThread, INFINITE);//전송용 쓰레드가 중지될때까지 기다린다./
    WaitForSingleObject(ballThread, INFINITE);
    WaitForSingleObject(recvThread, INFINITE);//수신용 쓰레드가 중지될때까지 기다린다.

    closesocket(sock);//소켓을 종료한다.
    WSACleanup();//윈도우 소켓 사용중지를 운영체제에 알린다.
    return 0;
}


unsigned WINAPI SendMsg(void* arg)
{//전송용 쓰레드함수
    SOCKET sock = *((SOCKET*)arg);//서버용 소켓을 전달한다.
                                  //char Msg[BUF_SIZE];
    try
    {
        while (1)
        {//반복
            WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
            String rhotheta = to_string(opt_rho) + " " + to_string(opt_theta);
            strcpy(msg, rhotheta.c_str());
            send(sock, msg, strlen(msg), 0);//nameMsg를 서버에게 전송한다.
            ReleaseMutex(hMutex);//뮤텍스 중지
            Sleep(30);
        }
        //Sleep(30);
        return 0;
    }
    catch (Exception e)
    {
        ErrorHandling("SendMsg 에러");
    }
}


void SendToMsg(void* arg)
{//전송용 쓰레드함수
    SOCKET sock = *((SOCKET*)arg);//서버용 소켓을 전달한다.

    WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
    String rhotheta = to_string(opt_rho) + " " + to_string(opt_theta);
    strcpy(msg, rhotheta.c_str());
    send(sock, msg, strlen(msg), 0);//nameMsg를 서버에게 전송한다.
    ReleaseMutex(hMutex);//뮤텍스 중지
                         //Sleep(30);

}


unsigned WINAPI RecvMsg(void* arg)
{
    SOCKET sock = *((SOCKET*)arg);//서버용 소켓을 전달한다.
    char Msg[BUF_SIZE];
    int strLen;
    char recvX[10], recvY[10];

    try
    {
        while (1)
        {


            strLen = recv(sock, Msg, BUF_SIZE - 1, 0);//서버로부터 메시지를 수신한다.
            if (strLen == -1)
            {
                ErrorHandling("입력받은 메세지가 없습니다.");
                return -1;
            }
            Msg[strLen] = 0;//문자열의 끝을 알리기 위해 설정
            fputs(Msg, stdout);//자신의 콘솔에 받은 메시지를 출력한다.
            fputs("\n", stdout);

            WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행                           
            strcpy(recvX, strtok(Msg, " "));
            strcpy(recvY, strtok(NULL, " "));

            X = atof(recvX);
            Y = atof(recvY);
            ReleaseMutex(hMutex);//뮤텍스 중지
            Sleep(30);
        }
        return 0;
    }
    catch (Exception e)
    {
        ErrorHandling("RecvMsg 에러");
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
        WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행  
                                              //lock = true;
        Mat frame;
        cap >> frame; // get a new frame from camera
        flip(frame, frame, 1);


        cvtColor(frame, edges, COLOR_BGR2GRAY);

        int width = frame.cols;
        int height = frame.rows;
        Mat img_color; //라벨링용 이미지
                       //HSV로 변환
        cvtColor(frame, img_color, COLOR_RGB2HSV);
        Mat img_binary;

        inRange(img_color, Scalar(LowH, LowS, LowV), Scalar(HighH, HighS, HighV), img_binary);

        erode(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        dilate(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));


        //morphological closing 영역의 구멍 메우기 
        dilate(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
        erode(img_binary, img_binary, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));

        // 캐니 알고리즘 적용
        Mat contours;
        if (count % 2 == 0) //count를 증가시켜서 선출력 속도를 조절해야한다 선은 frame출력될때마다 출력되고 고정값을줘야함
            Canny(img_binary, contours, 125, 350);
        if (count == 100)
            count = 0;
        count++;
        std::vector<cv::Vec2f> lines;
        lines.reserve(1000);

        HoughLines(contours, lines, 1, PI / 180, 60); //(엣지영상,저장공간,방법,해상도,각도,임계값(겹치는게 이보다 많아야 직선인정))


        opt_rho = 0.0;
        opt_theta = 0.0;

        //Point opt_pt1,opt_pt2;
        // 마지막 행에서 해당 선의 교차점

        Mat result(contours.rows, contours.cols, CV_8U, Scalar(255));
        std::vector<Vec2f>::const_iterator it = lines.begin();
        while (it != lines.end())
        {
            rho = (*it)[0];   // 첫 번째 요소는 rho 거리
            theta = (*it)[1]; // 두 번째 요소는 델타 각도
                              //printf("%d", rho);
            if (theta < PI / 4. || theta > 3.*PI / 4.)
            { // 수직 행
                opt_rho = rho;
                opt_theta = theta;
            }
            ++it;
        }
        Point pt1(opt_rho / cos(opt_theta), 0); // 첫 행에서 해당 선의 교차점   //point는2d좌표를 표시해주는 클래스
        Point pt2((opt_rho - result.rows*sin(opt_theta)) / cos(opt_theta), result.rows);
        line(frame, pt1, pt2, Scalar(255), 2); // 하얀 선으로 그리기
        fputs("1\n", stdout);

        fputs("2\n", stdout);
        X = X + v*cos(Arg);
        Y = Y + v*sin(Arg);
        Arg = Update_Ball(&frame, &opt_rho, &opt_theta, arg);
        printf("%f, %f\n", X, Y);
        fputs("4\n", stdout);

        imshow("frame", frame);
        if (waitKey(30) >= 0) break;


        ReleaseMutex(hMutex);//뮤텍스 중지
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
    {//직선에 맞았을때
        if ((X*cos(*theta) + Y*sin(*theta) >= *rho - radius) && (X*cos(*theta) + Y*sin(*theta) <= *rho + radius))
        {
            fputs("3\n", stdout);
            SendToMsg(arg);
            Circle(frame);
            Arg = PI / 2 - 2 * Arg;
        }
        else Circle(frame);
    }
    else if ((X > 0 && X <= radius) && (Y > 0 && Y < Height))//왼쪽 벽 맞을때
    {
        Circle(frame);
        Arg = PI - Arg;
    }
    else if (X >= Width - radius && X < Width && Y > 0 && Y < Height) //오른쪽 벽맞을때
    {
        Circle(frame);
        Arg = PI - Arg;
    }
    else if (X > 0 && X < Width && Y > 0 && Y <= radius) //위쪽벽맞을때
    {
        Circle(frame);
        Arg = -Arg;
    }
    else if (X > 0 && X < Width && Y < Height && Y >= Height - radius) //아래쪽 벽 맞을때
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
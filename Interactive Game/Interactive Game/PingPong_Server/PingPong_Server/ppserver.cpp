#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")//윈도우 소켓관련 함수포함
//영상처리 헤더
#include <math.h>
#include <malloc.h>
#include <opencv2/opencv.hpp>   
#include <opencv2/core/core.hpp>   
#include <opencv2/highgui/highgui.hpp>  
#include<Windows.h>

#define BUF_SIZE 100
#define MAX_CLNT 256
unsigned WINAPI RecvMsg(void* arg);//쓰레드 함수
unsigned WINAPI HandleBall(void* arg);//공출력 쓰레드 함수
unsigned WINAPI SendMsg(void* arg);//메시지 보내는 함수
void ErrorHandling(char* msg);
float Update_Ball();

int clientCount = 0;
SOCKET clientSocks[MAX_CLNT];//클라이언트 소켓 보관용 배열
HANDLE hMutex;//뮤텍스
char msg[BUF_SIZE];

#define PI 3.14159265359
#define Height 720
#define Width 960
#define radius 30
#define SQ(x) ((x)*(x))
float v = 10;                 //초기속도      
float Arg = (PI / 180) * 50;       //초기각도
float X = 350;
float Y = 150;
float theta;
float rho;

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET serverSock, clientSock;
    SOCKADDR_IN serverAddr, clientAddr;     //주소할당 구조체
    int clientAddrSize;
    HANDLE hThread, ballThread, sendThread;
    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //윈도우 소켓을 사용하겠다는 사실을 운영체제에 전달
        ErrorHandling("WSAStartup() error!");

    hMutex = CreateMutex(NULL, FALSE, NULL);//하나의 뮤텍스를 생성한다.
    serverSock = socket(PF_INET, SOCK_STREAM, 0); //하나의 소켓을 생성한다.

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET; //IPv4 사용
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //서버에서 모든 NIC를 할당하고 싶을때 사용.
    serverAddr.sin_port = htons(atoi(argv[1])); //포트번호를 정수로 준다.

    if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) //생성한 소켓을 배치한다.
        ErrorHandling("bind() error");


    if (listen(serverSock, 5) == SOCKET_ERROR)//소켓을 준비상태에 둔다.
        ErrorHandling("listen() error");


    while (1)
    {
        clientAddrSize = sizeof(clientAddr);
        clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize);//서버에게 전달된 클라이언트 소켓을 clientSock에 전달

        WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행(공유데이터 보호목적)
        clientSocks[clientCount++] = clientSock;//클라이언트 소켓배열에 방금 가져온 소켓 주소를 전달
        ReleaseMutex(hMutex);//뮤텍스 중지

        sendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&clientSock, 0, NULL);
        ballThread = (HANDLE)_beginthreadex(NULL, 0, HandleBall, (void*)&clientSock, 0, NULL);
        hThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&clientSock, 0, NULL);//HandleClient 쓰레드 실행, clientSock을 매개변수로 전달

        printf("Connected Client IP : %s\n", inet_ntoa(clientAddr.sin_addr));
    }
    closesocket(serverSock);//생성한 소켓을 끈다.
    WSACleanup();//윈도우 소켓을 종료하겠다는 사실을 운영체제에 전달
    return 0;
}

unsigned WINAPI RecvMsg(void* arg)
{
    try
    {
        SOCKET clientSock = *((SOCKET*)arg); //매개변수로받은 클라이언트 소켓을 전달
        int strLen = 0, i;
        char msg[BUF_SIZE];

        char Rho[10], Theta[10];
        /*서버에서 클라이언트들에게 보낸다*/
        while (1)
        {
            WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
            strLen = recv(clientSock, msg, sizeof(msg), 0);
            //만약 클라이언트로부터 직선 정보가 들어오면 직선정보 업데이트
            //fputs(msg, stdout);
            //fputs("\n", stdout);
            strcpy(Rho, strtok(msg, " "));
            strcpy(Theta, strtok(NULL, " "));
            rho = atof(Rho);
            theta = atof(Theta);
            ReleaseMutex(hMutex);//뮤텍스 중지
            Sleep(30);
        }

        //이 줄을 실행한다는 것은 해당 클라이언트가 나갔다는 사실임. 따라서 해당 클라이언트를 배열에서 제거해줘야함
        WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
        for (i = 0; i < clientCount; i++)
        {//배열의 갯수만큼
            if (clientSock == clientSocks[i])
            {//만약 현재 clientSock값이 배열의 값과 같다면
                while (i++ < clientCount - 1)//클라이언트 개수 만큼
                    clientSocks[i] = clientSocks[i + 1];//앞으로 땡긴다.
                break;
            }
        }
        clientCount--;//클라이언트 개수 하나 감소
        ReleaseMutex(hMutex);//뮤텍스 중지
        closesocket(clientSock);//소켓을 종료한다.
        return 0;
    }
    catch (Exception e)
    {
        ErrorHandling("HandleClient 에러");
        return 0;
    }
}


unsigned WINAPI SendMsg(void* arg)
{ //메시지를 모든 클라이언트에게 보낸다.
    try
    {
        while (1)
        {
            int i;
            char Msg[BUF_SIZE];
            String XYposition = to_string(X) + " " + to_string(Y);
            strcpy(Msg, XYposition.c_str());
            //fputs(Msg, stdout);
            //fputs("\n", stdout);
            WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
            for (i = 0; i < clientCount; i++)//클라이언트 개수만큼
                send(clientSocks[i], Msg, strlen(Msg), 0);//클라이언트들에게 메시지를 전달한다.
            ReleaseMutex(hMutex);//뮤텍스 중지  
            Sleep(30);
           
        }
        return 0;
    }
    catch (Exception e)
    {
        ErrorHandling("SendMsg 에러");
        return 0;
    }

}


unsigned WINAPI HandleBall(void* arg)
{
    try
    {
        while (1)
        {
            WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
            Arg = Update_Ball();
            X = X + v*cos(Arg);
            Y = Y + v*sin(Arg);
            ReleaseMutex(hMutex);//뮤텍스 중지
            Sleep(30);
        }
        return 0;
    }
    catch (Exception e)
    {
        ErrorHandling("HandleBall 에러");
        return 0;
    }
}



float Update_Ball()
{
    Mat server(640, 480, 0);
    if (X > radius && X < Width - radius && Y > radius && Y < Height - radius)
    {//직선에 맞았을때
        if ((X*cos(theta) + Y*sin(theta) >= rho - radius) && (X*cos(theta) + Y*sin(theta) <= rho + radius))
        {
            circle(server, Point(X, Y), radius, Scalar(0, 0, 0), -1);
            Arg = PI / 2 - 2 * Arg;
        }
        else  circle(server, Point(X, Y), radius, Scalar(0, 0, 0), -1);

    }
    else if (X >= 0 && X <= radius && Y >= 0 && Y <= Height - 0)
    {
        circle(server, Point(X, Y), radius, Scalar(0, 0, 0), -1);
        Arg = PI - Arg;
    }
    else if (X >= Width - radius && X <= Width && Y >= 0 && Y <= Height - 0)
    {
        circle(server, Point(X, Y), radius, Scalar(0, 0, 0), -1);
        Arg = PI - Arg;
    }
    else if (X >= 0 && X <= Width - 0 && Y >= 0 && Y <= radius)
    {
        circle(server, Point(X, Y), radius, Scalar(0, 0, 0), -1);
        Arg = -Arg;
    }
    else if (X >= 0 && X <= Width - 0 && Y <= Height && Y >= Height - radius)
    {
        circle(server, Point(X, Y), radius, Scalar(0, 0, 0), -1);
        Arg = -Arg;
    }

    return Arg;
}



void ErrorHandling(char* msg)
{
    fputs(msg, stdout);
    fputc('\n', stdout);
    exit(1);
}
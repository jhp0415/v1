#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")//������ ���ϰ��� �Լ�����
//����ó�� ���
#include <math.h>
#include <malloc.h>
#include <opencv2/opencv.hpp>   
#include <opencv2/core/core.hpp>   
#include <opencv2/highgui/highgui.hpp>  
#include<Windows.h>

#define BUF_SIZE 100
#define MAX_CLNT 256
unsigned WINAPI RecvMsg(void* arg);//������ �Լ�
unsigned WINAPI HandleBall(void* arg);//����� ������ �Լ�
unsigned WINAPI SendMsg(void* arg);//�޽��� ������ �Լ�
void ErrorHandling(char* msg);
float Update_Ball();

int clientCount = 0;
SOCKET clientSocks[MAX_CLNT];//Ŭ���̾�Ʈ ���� ������ �迭
HANDLE hMutex;//���ؽ�
char msg[BUF_SIZE];

#define PI 3.14159265359
#define Height 640
#define Width 480
#define radius 30
#define SQ(x) ((x)*(x))
float v = 10;                 //�ʱ�ӵ�      
float Arg = (PI / 180) * 50;       //�ʱⰢ��
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
    SOCKADDR_IN serverAddr, clientAddr;     //�ּ��Ҵ� ����ü
    int clientAddrSize;
    HANDLE hThread, ballThread, sendThread;
    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //������ ������ ����ϰڴٴ� ����� �ü���� ����
        ErrorHandling("WSAStartup() error!");

    hMutex = CreateMutex(NULL, FALSE, NULL);//�ϳ��� ���ؽ��� �����Ѵ�.
    serverSock = socket(PF_INET, SOCK_STREAM, 0); //�ϳ��� ������ �����Ѵ�.

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET; //IPv4 ���
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //�������� ��� NIC�� �Ҵ��ϰ� ������ ���.
    serverAddr.sin_port = htons(atoi(argv[1])); //��Ʈ��ȣ�� ������ �ش�.

    if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) //������ ������ ��ġ�Ѵ�.
        ErrorHandling("bind() error");


    if (listen(serverSock, 5) == SOCKET_ERROR)//������ �غ���¿� �д�.
        ErrorHandling("listen() error");


    while (1)
    {
        clientAddrSize = sizeof(clientAddr);
        clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize);//�������� ���޵� Ŭ���̾�Ʈ ������ clientSock�� ����

        WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����(���������� ��ȣ����)
        clientSocks[clientCount++] = clientSock;//Ŭ���̾�Ʈ ���Ϲ迭�� ��� ������ ���� �ּҸ� ����
        ReleaseMutex(hMutex);//���ؽ� ����

        sendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&clientSock, 0, NULL);
        ballThread = (HANDLE)_beginthreadex(NULL, 0, HandleBall, (void*)&clientSock, 0, NULL);
        hThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&clientSock, 0, NULL);//HandleClient ������ ����, clientSock�� �Ű������� ����

        printf("Connected Client IP : %s\n", inet_ntoa(clientAddr.sin_addr));
    }
    closesocket(serverSock);//������ ������ ����.
    WSACleanup();//������ ������ �����ϰڴٴ� ����� �ü���� ����
    return 0;
}

unsigned WINAPI RecvMsg(void* arg)
{
    try
    {
        SOCKET clientSock = *((SOCKET*)arg); //�Ű������ι��� Ŭ���̾�Ʈ ������ ����
        int strLen = 0, i;
        char msg[BUF_SIZE];

        char Rho[10], Theta[10];
        /*�������� Ŭ���̾�Ʈ�鿡�� ������*/
        while (1)
        {
            WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
            strLen = recv(clientSock, msg, sizeof(msg), 0);
            //���� Ŭ���̾�Ʈ�κ��� ���� ������ ������ �������� ������Ʈ

            strcpy(Rho, strtok(msg, " "));
            strcpy(Theta, strtok(NULL, " "));
            rho = atof(Rho);
            theta = atof(Theta);
            ReleaseMutex(hMutex);//���ؽ� ����
            Sleep(50);
        }

        //�� ���� �����Ѵٴ� ���� �ش� Ŭ���̾�Ʈ�� �����ٴ� �����. ���� �ش� Ŭ���̾�Ʈ�� �迭���� �����������
        WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
        for (i = 0; i < clientCount; i++)
        {//�迭�� ������ŭ
            if (clientSock == clientSocks[i])
            {//���� ���� clientSock���� �迭�� ���� ���ٸ�
                while (i++ < clientCount - 1)//Ŭ���̾�Ʈ ���� ��ŭ
                    clientSocks[i] = clientSocks[i + 1];//������ �����.
                break;
            }
        }
        clientCount--;//Ŭ���̾�Ʈ ���� �ϳ� ����
        ReleaseMutex(hMutex);//���ؽ� ����
        closesocket(clientSock);//������ �����Ѵ�.
        return 0;
    }
    catch (Exception e)
    {
        ErrorHandling("HandleClient ����");
        return 0;
    }
}


unsigned WINAPI SendMsg(void* arg)
{ //�޽����� ��� Ŭ���̾�Ʈ���� ������.
    try
    {
        while (1)
        {
            int i;
            char Msg[BUF_SIZE];
            String XYposition = to_string(X) + " " + to_string(Y);
            strcpy(Msg, XYposition.c_str());

            WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
            for (i = 0; i < clientCount; i++)//Ŭ���̾�Ʈ ������ŭ
                send(clientSocks[i], Msg, strlen(Msg), 0);//Ŭ���̾�Ʈ�鿡�� �޽����� �����Ѵ�.
            ReleaseMutex(hMutex);//���ؽ� ����  
            Sleep(50);
        }
        return 0;
    }
    catch (Exception e)
    {
        ErrorHandling("SendMsg ����");
        return 0;
    }

}


unsigned WINAPI HandleBall(void* arg)
{
    try
    {
        while (1)
        {
            WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
            Arg = Update_Ball();
            X = X + v*cos(Arg);
            Y = Y + v*sin(Arg);
            ReleaseMutex(hMutex);//���ؽ� ����
            Sleep(50);
        }
        return 0;
    }
    catch (Exception e)
    {
        ErrorHandling("HandleBall ����");
        return 0;
    }
}



float Update_Ball()
{
    Mat server(640, 480, 0);
    if (X > radius && X < Width - radius && Y > radius && Y < Height - radius)
    {//������ �¾�����
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
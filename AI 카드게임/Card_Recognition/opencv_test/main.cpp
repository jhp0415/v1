//--------소켓통신
#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <Windows.h>
#include <iostream>
//---------------------
#include <iostream>
#include <stdio.h>
#include <malloc.h>
#include "opencv2/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/xfeatures2d.hpp"


using namespace cv;
using namespace cv::xfeatures2d;

const int LOOP_NUM = 10;
const int GOOD_PTS_MAX = 50;
const float GOOD_PORTION = 0.15f;

int64 work_begin = 0;
int64 work_end = 0;

char Ranking[21][100] = {
    { "1KKeut" },
    { "2KKeut" },{ "3KKeut" },
    { "4KKeut" },{ "5KKeut" },
    { "6KKeut" },{ "7KKeut" },
    { "8KKeut" },{ "9KKeut" },
    { "SeRyuk" },{ "JangSa" },
    { "JangPPing" },{ "GuPPing" },
    { "DockSa" },{ "Ali" },
    { "TTang" },{ "94 PaTo" },
    { "18 GwangTTaeng" },{ "13 GwangTTaeng" },
    { "49 PaTo" },{ "38 GwangTTaeng" }
};


typedef struct CARD
{
    UMat img;
    char name[10];
};

//---------------------소켓통신--------------------------------------

WSADATA wsaData;
SOCKET connect_sock;
SOCKADDR_IN connect_addr;

//에러 처리 하는 함수
void ErrorHandling(const char* Message)
{
    fputs(Message, stderr);
    fputc('\n', stderr);
    exit(1);	//오류가 발생했으니 그대로 프로그램 종료
}
void SetingSocket()
{
    //1. 소켓 생성 및 기본 셋팅
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) ErrorHandling("WSAStartup() error");
    connect_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (connect_sock == INVALID_SOCKET) ErrorHandling("socket() error");

    memset(&connect_addr, 0, sizeof(connect_addr));
    connect_addr.sin_family = AF_INET;
    //connect_addr.sin_addr.S_un.S_addr = inet_addr(argv[1]);//argv[1]:인수에 입력된 IP
    //connect_addr.sin_port = htons(atoi(argv[2]));	//argv[2]:인수에 입력된 port
    connect_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    connect_addr.sin_port = htons(atoi("50007"));

    //2. 입력한 주소로 접속
    if (connect(connect_sock, (SOCKADDR*)&connect_addr, sizeof(connect_addr)))
        ErrorHandling("connect() error");
}
char socketdata[3] = { 0 };
void RecvMsf()
{
    while (1)
    {
        char buf[255] = { 0 };
        //char buf;
        int recvsize2 = recv(connect_sock, buf, sizeof(buf), 0);
        printf("%s", buf);
        if (strcmp(buf, "ddd") == 0) break;
    }
}
void SendMsg(int num1, int num2)
{
    while (1)
    {
        //sprintf(&socketdata[0], "%d", num1);
        //sprintf(&socketdata[1], "%d", num2);
        //sprintf(&socketdata[2], "%d", 1);
        itoa(num1, &socketdata[0], 10);
        itoa(num2, &socketdata[1], 10);
        itoa(1, &socketdata[2], 10);


        printf("client >> %c,%c,%c\n", socketdata[0], socketdata[1], socketdata[2]);
        int retval = send(connect_sock, (char*)socketdata, sizeof(char)*3, 0);
        if (retval == SOCKET_ERROR)
        {
            ErrorHandling("send()");
            exit(1);
        }
        //---------서버가 제대로 받았는지 메시지 수신-----------
        char buf[255] = { 0 };
        int recvsize2 = recv(connect_sock, buf, sizeof(buf), 0);
  /*      if (recvsize2 <= 0)
        {
            printf("접속종료\n");
            break;
        }*/
        buf[recvsize2] = '\0';
        printf("server >> %s\n", buf);
        break;
        //-------------------------------------------
    }
}
void EndSocket()
{//소켓 종료
    closesocket(connect_sock);
    WSACleanup();
}
//-----------------------------------------------------------------


static void workBegin()
{
    work_begin = getTickCount();
}

static void workEnd()
{
    work_end = getTickCount() - work_begin;
}

static double getTime()
{
    return work_end / ((double)getTickFrequency())* 1000.;
}

struct SURFDetector
{
    Ptr<Feature2D> surf;
    SURFDetector(double hessian = 1000.0)
    {
        surf = SURF::create(hessian);
    }
    template<class T>
    void operator()(const T& in, const T& mask, std::vector<cv::KeyPoint>& pts, T& descriptors, bool useProvided = false)
    {
        surf->detectAndCompute(in, mask, pts, descriptors, useProvided);
    }
};

template<class KPMatcher>
struct SURFMatcher
{
    KPMatcher matcher;
    template<class T>
    void match(const T& in1, const T& in2, std::vector<cv::DMatch>& matches)
    {
        matcher.match(in1, in2, matches);
    }
};

int GetAngleABC(Point a, Point b, Point c)
{
    Point ab = { b.x - a.x, b.y - a.y };
    Point cb = { b.x - c.x, b.y - c.y };

    float dot = (ab.x * cb.x + ab.y * cb.y); // dot product
    float cross = (ab.x * cb.y - ab.y * cb.x); // cross product

    float alpha = atan2(cross, dot);

    return (int)floor(alpha * 180.0 / CV_PI + 0.5);
}

static Mat drawGoodMatches(
    struct CARD card,
    const Mat& img2,
    const std::vector<KeyPoint>& keypoints1,
    const std::vector<KeyPoint>& keypoints2,
    std::vector<DMatch>& matches,
    std::vector<Point2f>& scene_corners_,
    char** cardname
)
{
    std::sort(matches.begin(), matches.end());
    std::vector< DMatch > good_matches;
    double minDist = matches.front().distance;
    double maxDist = matches.back().distance;


    const int ptsPairs = std::min(GOOD_PTS_MAX, (int)(matches.size() * GOOD_PORTION));
    for (int i = 0; i < ptsPairs; i++)
    {
        good_matches.push_back(matches[i]);
    }

    std::cout << "\nMax distance: " << maxDist << std::endl;
    std::cout << "Min distance: " << minDist << std::endl;

    // drawing the results
    Mat img_matches;

    drawMatches(card.img, keypoints1, img2, keypoints2,
                good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    //-- Localize the object
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;


    for (size_t i = 0; i < good_matches.size(); i++)
    {
        //-- Get the keypoints from the good matches
        obj.push_back(keypoints1[good_matches[i].queryIdx].pt);
        scene.push_back(keypoints2[good_matches[i].trainIdx].pt);
    }
    //-- Get the corners from the image_1 ( the object to be "detected" )
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = Point(0, 0);
    obj_corners[1] = Point(card.img.cols, 0);
    obj_corners[2] = Point(card.img.cols, card.img.rows);
    obj_corners[3] = Point(0, card.img.rows);
    std::vector<Point2f> scene_corners(4);

    Mat H = findHomography(obj, scene, RANSAC);
    perspectiveTransform(obj_corners, scene_corners, H);

    scene_corners_ = scene_corners;

    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
    // 맞는 이미지 사각형 

    line(img_matches,
         scene_corners[0] + Point2f((float)card.img.cols, 0), scene_corners[1] + Point2f((float)card.img.cols, 0),
         Scalar(0, 255, 0), 2, LINE_AA);
    line(img_matches,
         scene_corners[1] + Point2f((float)card.img.cols, 0), scene_corners[2] + Point2f((float)card.img.cols, 0),
         Scalar(0, 255, 0), 2, LINE_AA);
    line(img_matches,
         scene_corners[2] + Point2f((float)card.img.cols, 0), scene_corners[3] + Point2f((float)card.img.cols, 0),
         Scalar(0, 255, 0), 2, LINE_AA);
    line(img_matches,
         scene_corners[3] + Point2f((float)card.img.cols, 0), scene_corners[0] + Point2f((float)card.img.cols, 0),
         Scalar(0, 255, 0), 2, LINE_AA);



    // 사각형 내부에 숫자? 글씨 적음
    cv::Point2f mypoint;
    mypoint.x = (scene_corners[0].x + (float)card.img.cols + scene_corners[1].x + (float)card.img.cols) / 2;
    mypoint.y = (scene_corners[0].y + scene_corners[3].y) / 2;


    float angle = GetAngleABC(scene_corners[0] + Point2f((float)card.img.cols, 0), scene_corners[1] + Point2f((float)card.img.cols, 0), scene_corners[2] + Point2f((float)card.img.cols, 0));
    float angle2 = GetAngleABC(scene_corners[0] + Point2f((float)card.img.cols, 0), scene_corners[3] + Point2f((float)card.img.cols, 0), scene_corners[2] + Point2f((float)card.img.cols, 0));
    if ((abs(angle) <= 110 && abs(angle) >= 70) && (abs(angle2) <= 110 && abs(angle2) >= 70))
    {

        //cv::putText(img_matches, card.name, mypoint, 2, 1.2, Scalar(0, 255, 0));

        if (mypoint.y < 264.799011)
        {
            if (cardname[0][0] == NULL)
                strcpy(cardname[0], card.name);
            else if (strcmp(cardname[0], card.name) != 0)
                strcpy(cardname[1], card.name);
        }
        else
        {
            if (cardname[2][0] == NULL)
                strcpy(cardname[2], card.name);
            else if (strcmp(cardname[2], card.name) != 0)
                strcpy(cardname[3], card.name);
        }
    }

    return img_matches;
}
// This program demonstrates the usage of SURF_OCL.
// use cpu findHomography interface to calculate the transformation matrix

int Game(Mat img_matches, char* name, char* name2, int x, int y)
{
    // 한글 지원 안돼서 영어로 쳐야돼
    // c + 안돼

    // 그리고 각 상태마다 점수내서 누가 이겻는지도 해야함

    int a = name[0] - '0', a2 = name[2] - '0';
    int b = name2[0] - '0', b2 = name2[2] - '0';

    cv::Point mypoint;
    mypoint.x = x; mypoint.y = y;
    int index = 20;

    if (((a == 3 && a2 == 1) && (b == 8 && b2 == 1)) || ((b == 3 && b2 == 1) && (a == 8 && a2 == 1)))//1
        index = 20;
    //cv::putText(img_matches, Ranking[20], mypoint, 2, 1.2, Scalar(0, 255, 0));
    else if (((a == 4 && a2 == 1) && (b == 9 && b2 == 1)) || ((b == 4 && b2 == 1) && (a == 9 && a2 == 1)))//2
        index = 19;
    //cv::putText(img_matches, Ranking[19], mypoint, 2, 1.2, Scalar(0, 255, 0));
    else if (((a == 1 && a2 == 1) && (b == 3 && b2 == 1)) || ((b == 1 && b2 == 1) && (a == 3 && a2 == 1)))//3
        index = 18;
    //cv::putText(img_matches, Ranking[18], mypoint, 2, 1.2, Scalar(0, 255, 0));
    else if (((a == 1 && a2 == 1) && (b == 8 && b2 == 1)) || ((b == 1 && b2 == 1) && (a == 8 && a2 == 1)))//4
        index = 17;
    //cv::putText(img_matches, Ranking[17], mypoint, 2, 1.2, Scalar(0, 255, 0));

    else if (((a == 4 && a2 == 2) && (b == 9 && b2 == 1)) || ((b == 4 && b2 == 2) && (a == 9 && a2 == 1)))//5
        index = 16;
    //cv::putText(img_matches, Ranking[16], mypoint, 2, 1.2, Scalar(0, 255, 0));
    else if (((a == 4 && a2 == 1) && (b == 9 && b2 == 2)) || ((b == 4 && b2 == 1) && (a == 9 && a2 == 2)))//6
        index = 16;
    //cv::putText(img_matches, Ranking[16], mypoint, 2, 1.2, Scalar(0, 255, 0));
    else if (((a == 4 && a2 == 2) && (b == 9 && b2 == 2)) || ((b == 4 && b2 == 2) && (a == 9 && a2 == 2)))//7
        index = 16;
    //cv::putText(img_matches, Ranking[16], mypoint, 2, 1.2, Scalar(0, 255, 0));

    else if (a == b)//8
    {
        char c[10] = { ((a + b) % 10-1 + '/') };
        index = 15;
        //strcat(c, Ranking[15]);
        //cv::putText(img_matches, c, mypoint, 2, 1.2, Scalar(0, 255, 0));

    }
    else if (((a == 1) && (b == 2)) || ((b == 1) && (a == 2)))//9
        index = 14;
    //cv::putText(img_matches, Ranking[14], mypoint, 2, 1.2, Scalar(0, 255, 0));
    else if (((a == 4) && (b == 1)) || ((b == 4) && (a == 1)))//10
        index = 13;
    //cv::putText(img_matches, Ranking[13], mypoint, 2, 1.2, Scalar(0, 255, 0));
    else if (((a == 1) && (b == 9)) || ((b == 1) && (a == 9)))//11
        index = 12;
    //cv::putText(img_matches, Ranking[12], mypoint, 2, 1.2, Scalar(0, 255, 0));
    else if (((a == 1) && (b == 10)) || ((b == 1) && (a == 10)))//12
        index = 11;
    //cv::putText(img_matches, Ranking[11], mypoint, 2, 1.2, Scalar(0, 255, 0));
    else if (((a == 10) && (b == 4)) || ((b == 10) && (a == 4)))//13
        index = 10;
    //cv::putText(img_matches, Ranking[10], mypoint, 2, 1.2, Scalar(0, 255, 0));
    else if (((a == 6) && (b == 4)) || ((b == 6) && (a == 4)))//14
        index = 9;
    //cv::putText(img_matches, Ranking[9], mypoint, 2, 1.2, Scalar(0, 255, 0));

    else//15
    {
        if (a + b < 10)
        {
            char c[10] = { ((a + b) % 10 + '0') };
            index = (a + b) % 10-1;
            //strcat(c, Ranking[(a + b) % 10 - 1]);
            //cv::putText(img_matches, c, mypoint, 2, 1.2, Scalar(0, 255, 0));
        }
        else
        {
            char c[10] = { ((a + b) % 10 + '0') };
            index = (a + b) % 10-1;
            //strcat(c, Ranking[(a + b) % 10 - 1]);
            //cv::putText(img_matches, c, mypoint, 2, 1.2, Scalar(0, 255, 0));
        }
    }
    cv::putText(img_matches, Ranking[index], mypoint, 2, 1.2, Scalar(0, 255, 0));
    return index;

}

void main(int argc, char* argv[])
{

    SetingSocket();
  
    char buf[255] = { 0 };
    printf("\n>> ");

    //---------서버로부터 메시지 수신-----------
    int recvsize = recv(connect_sock, buf, sizeof(buf), 0);
    if (recvsize <= 0)
    {
        printf("접속종료\n");
    }
    buf[recvsize] = '\0';
    printf("server >> %s\n", buf);
    CARD card[20];

    VideoCapture vc(0);
    if (!vc.isOpened()) return;

    char leftName[20];
    char** Mycard;
    Mycard = (char**)calloc(4, sizeof(char*));

    for (int i = 0; i<4; i++)
        Mycard[i] = (char*)calloc(10, sizeof(char));

    for (int i = 0; i < 10; i++)
    {
        sprintf(leftName, "%d_1.jpg", i + 1);
        imread(leftName, IMREAD_GRAYSCALE).copyTo(card[i].img);
        sprintf(card[i].name, "%d_1", i + 1);

        sprintf(leftName, "%d_2.jpg", i + 1);
        imread(leftName, IMREAD_GRAYSCALE).copyTo(card[i + 10].img);
        sprintf(card[i + 10].name, "%d_2", i + 1);
    }

    int count = 0, m = 0;
    Mat img_matches;
    Mat img;

    while (1)
    {
        
        vc >> img;

        if (1)
        {
            for (m = 0; m < 20; m++)
            {
                vc >> img;
                //declare input/output
                std::vector<KeyPoint> keypoints1, keypoints2;
                std::vector<DMatch> matches;

                UMat _descriptors1, _descriptors2;
                Mat descriptors1 = _descriptors1.getMat(ACCESS_RW),
                    descriptors2 = _descriptors2.getMat(ACCESS_RW);

                //instantiate detectors/matchers
                SURFDetector surf;

                SURFMatcher<BFMatcher> matcher;

                surf(card[m].img.getMat(ACCESS_READ), Mat(), keypoints1, descriptors1);
                surf(img, Mat(), keypoints2, descriptors2);
                matcher.match(descriptors1, descriptors2, matches);

                std::vector<Point2f> corner;

                img_matches = drawGoodMatches(card[m], img, keypoints1, keypoints2, matches, corner, Mycard);

                std::cout << Mycard[0] << std::endl;
                std::cout << Mycard[1] << std::endl;
                std::cout << Mycard[2] << std::endl;
                std::cout << Mycard[3] << std::endl;

                //-- Show detected matches
                if (Mycard[1][0] != NULL && Mycard[3][0] != NULL)
                {
                    int index1 = Game(img_matches, Mycard[0], Mycard[1], 100, 200);
                    int index2 = Game(img_matches, Mycard[2], Mycard[3], 100, 350);
                    SendMsg(index1, index2);
                }
                cv::Point2f Mypoint1, Mypoint2, Mypoint3, Mypoint4;

                // 이 부분도 수정하면 좋을 듯?
                Mypoint1.x = 300;
                Mypoint1.y = 200;

                Mypoint2.x = 450;
                Mypoint2.y = 200;

                Mypoint3.x = 300;
                Mypoint3.y = 400;

                Mypoint4.x = 450;
                Mypoint4.y = 400;

                if (Mycard[1] != NULL && Mycard[3] != NULL && Mycard[2] != NULL && Mycard[4] != NULL)
                {
                    cv::putText(img_matches, Mycard[1], Mypoint1, 2, 1.2, Scalar(0, 255, 0));
                    cv::putText(img_matches, Mycard[0], Mypoint2, 2, 1.2, Scalar(0, 255, 0));
                    cv::putText(img_matches, Mycard[2], Mypoint3, 2, 1.2, Scalar(0, 255, 0));
                    cv::putText(img_matches, Mycard[3], Mypoint4, 2, 1.2, Scalar(0, 255, 0));
                }
                // 여기까지

                imshow("surf matches", img_matches);

                if (waitKey(10) == 27) break;



                if (Mycard[1][0] != NULL && Mycard[3][0] != NULL) break;
            }
        }
        count++;
        //if (Mycard[1][0] != NULL && Mycard[3][0] != NULL) break;
       
        //imshow("surf matches", img_matches);

        Mycard[0][0] = 0; Mycard[1][0] = 0; Mycard[2][0] = 0; Mycard[3][0] = 0;
        //waitKey(1);
        RecvMsf();
    }

    /*while (1)
    {
        imshow("surf matches", img_matches);

    }
*/
    //destroyAllWindows();
    EndSocket();
}

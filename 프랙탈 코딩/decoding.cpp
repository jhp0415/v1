#include"CommonFunction.h"

using namespace cv;
using namespace std;

#define PI 3.14159265359

int error_threshold = 3;

//디코딩 파라미터 계산하기
void ComputDecodingParameter(encodingResult data, int** domain, int bsize)
{
    int** temp_iso = (int**)IntAlloc2(bsize, bsize);
    int** temp_alp = (int**)IntAlloc2(bsize, bsize);

    int domain_avg = ComputeAVG(domain, bsize, bsize);		//domain 평균구하기
    AddOrSub_AVG(domain, domain, bsize, -domain_avg);
    //geo 변환
    Isometry(data.geo, domain, temp_iso, bsize, bsize);
    //alpha 곱하기
    Multiply_Alpha(temp_iso, temp_alp, bsize, data.alpha);
    //encoding avg 더하기
    AddOrSub_AVG(temp_alp, domain, bsize, data.avg);

    IntFree2(temp_iso, bsize, bsize);
    IntFree2(temp_alp, bsize, bsize);
}

//디코딩 호출
void Decoding(encodingResult** enData, IMG dec, IMG tmp, int height, int width, int cy, int cx, int bsize)
{
    //재귀 종료 조건
    if (bsize < 4) return;

    int** temp_read = (int**)IntAlloc2(bsize * 2, bsize * 2);
    int** domain = (int**)IntAlloc2(bsize, bsize);
   
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            //1/2축소
            ReadBlock(tmp.image, temp_read, enData[i][j].x, enData[i][j].y, bsize * 2);	//2n블럭 읽어오기
            Contraction(temp_read, domain, bsize * 2, bsize * 2);

            //decoding parameter 적용하기
            ComputDecodingParameter(enData[i][j], domain, bsize);
            //dec 이미지에 블록을 삽입
            WriteBlock(dec.image, domain, cx + j*bsize, cy + i*bsize, bsize);

            //읽은 데이터가 쪼개진 데이터면 --> 재귀하기
            if (enData[i][j].enData != NULL)
            {
                //저장될 이미지 공간read, 원본 이미지tmp를 파라미터로 가져가기
                Decoding(enData[i][j].enData, dec, tmp, 2, 2, cy + i*bsize, cx + j*bsize, bsize / 2);
            }
        }
    }
    IntFree2(temp_read, bsize * 2, bsize * 2);
    IntFree2(domain, bsize, bsize);
}

//파일에서 인코딩 파라미터 읽어오기
void ReadParameter_recursive(FILE* fp, encodingResult** A, int height, int width, int bsize)
{
    if (bsize < 4) return; 

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            fscanf(fp, "%d%d%d%d%d%lf%p", &A[i][j].bsize, &A[i][j].x, &A[i][j].y, &A[i][j].geo, &A[i][j].avg, &A[i][j].alpha, &A[i][j].enData);

            if (A[i][j].enData != NULL)
            {
                //할당
                A[i][j].enData = ER_Alloc2(2, 2);
                ReadParameter_recursive(fp, A[i][j].enData, 2, 2, A[i][j].bsize / 2);
            }
        }
    }
}

//파일 읽기 호출
bool ReadParameter(const char* name, encodingResult** A, int height, int width, int bsize)
{
    FILE* fp = fopen(name, "r");
    if (fp == NULL)
    {
        printf("Failure in fopen!!");
        return false;
    }
    
    ReadParameter_recursive(fp, A, height, width, bsize);

    fclose(fp);
}

//Mat를 data로 초기화 함수
void Init_Mat(IMG input, int data)
{
    for (int i = 0; i < input.height; i++)
        for (int j = 0; j < input.width; j++)
            input.image[i][j] = data;
}

//이미지를 복사하기
void CopyMat(IMG img_in, IMG img_out)
{
    for (int i = 0; i < img_in.height; i++)
        for (int j = 0; j < img_in.width; j++)
            img_out.image[i][j] = img_in.image[i][j];
}

//라인 그리기
void DrawLine(int** block, int y, int x, int bsize)
{
    for (int i = y; i < y + bsize; i++)
        for (int j = x; j < x + bsize; j++)
        {
            if (i == y) block[i][j] = 255;
            if (j == x) block[i][j] = 255;
        }
}

//디코딩 격자무늬 그리기
void DrawLattice(encodingResult** enData, IMG copy, int height, int width, int cy, int cx, int bsize)
{
    //재귀 조건
    if (bsize < 4) return;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            DrawLine(copy.image, cy + y*bsize, cx + x*bsize, bsize);

            if (enData[y][x].enData != NULL)
            {
                DrawLattice(enData[y][x].enData, copy, 2, 2, cy + y*bsize, cx + x*bsize, bsize / 2);
            }
        }
    }
}

//psnr 계산하기
double computePSNR(int** A, int** B, int width, int height)
{
    double error = 0.0;
    for (int i = 0; i < height; i++) for (int j = 0; j < width; j++)
    {
        error += (double)(A[i][j] - B[i][j]) * (A[i][j] - B[i][j]);
    }
    error = error / (width*height);
    double psnr = 10.0 * log10(255.*255. / error);
    return(psnr);
}

//---------------------------------------------------
void main()
{
    //cout 입출력 빠르게
    ios_base::sync_with_stdio(false);

    IMG copy;
    IMG input, dec, tmp;
    input.image = ReadImage("lena256x512.bmp", &input.width, &input.height);
    dec.width = input.width;    dec.height = input.height;
    tmp.width = input.width;    tmp.height = input.height;
    copy.width = input.width;    copy.height = input.height;
    dec.image = (int**)IntAlloc2(dec.width, dec.height);
    tmp.image = (int**)IntAlloc2(tmp.width, tmp.height);
    copy.image = (int**)IntAlloc2(copy.width, copy.height);


    int bsize = 16;
    string filename = "encoding" + to_string(error_threshold) + ".txt";
    encodingResult** enData = ER_Alloc2(input.width / bsize, input.height / bsize);

    //파일에서 읽기
    ReadParameter(filename.c_str(), enData, input.height / bsize, input.width / bsize, bsize);

    //image_dec를 128로 초기화
    Init_Mat(dec, 128);

    for (int n = 0; n < 5; n++)
    {
        //이미지 출력
        ImageShow("result", dec.image, dec.width, dec.height);
        //dec를 tmp로 복사하기
        CopyMat(dec, tmp);
        //디코딩
        Decoding(enData, dec, tmp, input.height / bsize, input.width / bsize, 0, 0, bsize);
    }

    //격자 그리기
    CopyMat(input, copy);
    DrawLattice(enData, copy, copy.height / bsize, copy.width / bsize, 0, 0, bsize);
    ImageShow("DrawLine", copy.image, copy.width, copy.height);

    //psnr값 출력하기
    cout << "psnr: " << computePSNR(input.image, tmp.image, input.width, input.height) << endl;


    IntFree2(input.image, input.width, input.height);
    IntFree2(dec.image, dec.width, dec.height);
    IntFree2(tmp.image, tmp.width, tmp.height);
    ER_Free2(enData, input.width / bsize, input.height / bsize);
}

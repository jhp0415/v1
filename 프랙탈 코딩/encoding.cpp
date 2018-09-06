#include "CommonFunction.h"

using namespace cv;
using namespace std;

#define PI 3.14159265359

int error_threshold = 3;

//최적의 변환 geo, alpha 값 찾기
int FindAlphaAndBestISO(int** img_in, int** block, int bsize, int* geo_num, double* alpha)
{
    int error, error_min = INT_MAX;
    int** temp_iso = (int**)IntAlloc2(bsize, bsize);
    int** temp_alp = (int**)IntAlloc2(bsize, bsize);
    //geo 변환
    for (int geo = 0; geo < 8; geo++)
    {
        Isometry(geo, block, temp_iso, bsize, bsize);

        //alpha 곱하기
        for (double alp = 0.3; alp <= 1.0; alp += 0.1)
        {
            Multiply_Alpha(temp_iso, temp_alp, bsize, alp);
            error = ComputeError(img_in, temp_alp, bsize, 0, 0);

            if (error < error_min)
            {
                error_min = error;
                *geo_num = geo;
                *alpha = alp;
            }
        }
    }
    IntFree2(temp_iso, bsize, bsize);
    IntFree2(temp_alp, bsize, bsize);
    return error_min;
}

//Domain AC 만들기
void MakeDomain(IMG input, int** domain, int y, int x, int bsize)
{
    int** temp_read = (int**)IntAlloc2(bsize * 2, bsize * 2);
    //domain AC 생성
    ReadBlock(input.image, temp_read, x, y, bsize * 2);
    Contraction(temp_read, domain, bsize * 2, bsize * 2);
    int domain_mean = ComputeAVG(domain, bsize, bsize);
    AddOrSub_AVG(domain, domain, bsize, -domain_mean);

    IntFree2(temp_read, bsize * 2, bsize * 2);
}

//최적의 인코딩 x,y, geo, avg, alpha 계산하기
int TemplateMatchingWithDownSamplingPlusShuffle(encodingResult* data, IMG input, int** block, int bsize, int stride)
{
    int geo, block_avg, error, error_min = INT_MAX;
    double alpha;
    int** domain = (int**)IntAlloc2(bsize, bsize);
    int** block_AC = (int**)IntAlloc2(bsize, bsize);

    //block AC 생성
    block_avg = ComputeAVG(block, bsize, bsize);
    AddOrSub_AVG(block, block_AC, bsize, -block_avg);

    for (int y = 0; y < input.height - bsize * 2 + 1; y += stride)
    {
        for (int x = 0; x < input.width - bsize * 2 + 1; x += stride)
        {
            //domain AC 생성
            MakeDomain(input, domain, y, x, bsize);
            //최적의 alpha, geo 값 찾기
            error = FindAlphaAndBestISO(block_AC, domain, bsize, &geo, &alpha);

            if (error < error_min)
            {
                error_min = error;
                data->bsize = bsize;    data->x = x;    data->y = y;    data->geo = geo;    data->avg = block_avg;  data->alpha = alpha;    data->enData = NULL;
            }
        }
    }

    IntFree2(domain, bsize, bsize);
    IntFree2(block_AC, bsize, bsize);
    int result = error_min / bsize / bsize;

    return result;
}

//인코딩 및 결과로그 출력
void Encoding(encodingResult** enData, IMG input, int height, int width, int cy, int cx, int bsize, int stride)
{
    //재귀 조건 : 16, 8, 4까지만 쪼개기
    if (bsize < 4) return;

    int** block = (int**)IntAlloc2(bsize, bsize);
    IMG read2;
    //read2.image = (int**)IntAlloc2(bsize, bsize);
    read2 = { block, bsize, bsize };

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            ReadBlock(input.image, block, cx + x*bsize, cy + y*bsize, bsize);
            int error = TemplateMatchingWithDownSamplingPlusShuffle(&enData[y][x], input, block, bsize, stride);

            ////결과 출력
            //cout << "bsize: " << enData[y / bsize][x / bsize].bsize << "=> ";
            //cout << "avg: " << enData[y / bsize][x / bsize].avg << ", " << "alpha: " << enData[y / bsize][x / bsize].alpha << ", "
            //    << "geo: " << enData[y / bsize][x / bsize].geo << ", " << "x: " << enData[y / bsize][x / bsize].x << ", "
            //    << "y: " << enData[y / bsize][x / bsize].y << ", "
            //    << "enData: " << enData[y / bsize][x / bsize].enData << endl;
            cout << "error: " << error << ", ";
            if (error > error_threshold)
            {
                enData[y][x].enData = ER_Alloc2(2, 2);
                Encoding(enData[y][x].enData, input, 2, 2, cy + y*bsize, cx + x*bsize, bsize / 2, stride / 2);
            }
        }
    }
    IntFree2(block, bsize, bsize);
}

//파일에 인코딩 파라미터 쓰기
void WriteParameter_recursive(FILE* fp, encodingResult** A, int height, int width)
{
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
        {
            //재귀 조건
            if (A[i][j].bsize < 4) return;

            fprintf(fp, "%d %d %d %d %d %lf %p\n", A[i][j].bsize, A[i][j].x, A[i][j].y, A[i][j].geo, A[i][j].avg, A[i][j].alpha, A[i][j].enData);
            if (A[i][j].enData != NULL)
            {
                WriteParameter_recursive(fp, A[i][j].enData, 2, 2);
            }
        }
}


//파일에 쓰기 호출
bool WriteParameter(const char* name, encodingResult** A, int width, int height)
{
    FILE* fp = fopen(name, "w");
    if (fp == NULL)
    {
        printf("Failure in fopen!!");
        return false;
    }

    WriteParameter_recursive(fp, A, width, height);

    fclose(fp);
    return true;

}

void main()
{
    ios_base::sync_with_stdio(false);

    IMG input;
    input.image = ReadImage("lena256x512.bmp", &input.width, &input.height);

    //int bsize = 8;
    int bsize = 16;
    int stride = 8;

    string filename = "encoding" + to_string(error_threshold) + ".txt";

    //인코딩
    encodingResult** enData = ER_Alloc2(input.width / bsize, input.height / bsize);
    Encoding(enData, input, input.height / bsize, input.width / bsize, 0, 0, bsize, stride);

    //파일에 쓰기
    WriteParameter(filename.c_str(), enData, input.height / bsize, input.width / bsize);

    IntFree2(input.image, input.width, input.height);
    ER_Free2(enData, input.width / bsize, input.height / bsize);
}
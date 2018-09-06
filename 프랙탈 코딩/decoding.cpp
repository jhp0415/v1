#include"CommonFunction.h"

using namespace cv;
using namespace std;

#define PI 3.14159265359

int error_threshold = 3;

//���ڵ� �Ķ���� ����ϱ�
void ComputDecodingParameter(encodingResult data, int** domain, int bsize)
{
    int** temp_iso = (int**)IntAlloc2(bsize, bsize);
    int** temp_alp = (int**)IntAlloc2(bsize, bsize);

    int domain_avg = ComputeAVG(domain, bsize, bsize);		//domain ��ձ��ϱ�
    AddOrSub_AVG(domain, domain, bsize, -domain_avg);
    //geo ��ȯ
    Isometry(data.geo, domain, temp_iso, bsize, bsize);
    //alpha ���ϱ�
    Multiply_Alpha(temp_iso, temp_alp, bsize, data.alpha);
    //encoding avg ���ϱ�
    AddOrSub_AVG(temp_alp, domain, bsize, data.avg);

    IntFree2(temp_iso, bsize, bsize);
    IntFree2(temp_alp, bsize, bsize);
}

//���ڵ� ȣ��
void Decoding(encodingResult** enData, IMG dec, IMG tmp, int height, int width, int cy, int cx, int bsize)
{
    //��� ���� ����
    if (bsize < 4) return;

    int** temp_read = (int**)IntAlloc2(bsize * 2, bsize * 2);
    int** domain = (int**)IntAlloc2(bsize, bsize);
   
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            //1/2���
            ReadBlock(tmp.image, temp_read, enData[i][j].x, enData[i][j].y, bsize * 2);	//2n�� �о����
            Contraction(temp_read, domain, bsize * 2, bsize * 2);

            //decoding parameter �����ϱ�
            ComputDecodingParameter(enData[i][j], domain, bsize);
            //dec �̹����� ����� ����
            WriteBlock(dec.image, domain, cx + j*bsize, cy + i*bsize, bsize);

            //���� �����Ͱ� �ɰ��� �����͸� --> ����ϱ�
            if (enData[i][j].enData != NULL)
            {
                //����� �̹��� ����read, ���� �̹���tmp�� �Ķ���ͷ� ��������
                Decoding(enData[i][j].enData, dec, tmp, 2, 2, cy + i*bsize, cx + j*bsize, bsize / 2);
            }
        }
    }
    IntFree2(temp_read, bsize * 2, bsize * 2);
    IntFree2(domain, bsize, bsize);
}

//���Ͽ��� ���ڵ� �Ķ���� �о����
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
                //�Ҵ�
                A[i][j].enData = ER_Alloc2(2, 2);
                ReadParameter_recursive(fp, A[i][j].enData, 2, 2, A[i][j].bsize / 2);
            }
        }
    }
}

//���� �б� ȣ��
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

//Mat�� data�� �ʱ�ȭ �Լ�
void Init_Mat(IMG input, int data)
{
    for (int i = 0; i < input.height; i++)
        for (int j = 0; j < input.width; j++)
            input.image[i][j] = data;
}

//�̹����� �����ϱ�
void CopyMat(IMG img_in, IMG img_out)
{
    for (int i = 0; i < img_in.height; i++)
        for (int j = 0; j < img_in.width; j++)
            img_out.image[i][j] = img_in.image[i][j];
}

//���� �׸���
void DrawLine(int** block, int y, int x, int bsize)
{
    for (int i = y; i < y + bsize; i++)
        for (int j = x; j < x + bsize; j++)
        {
            if (i == y) block[i][j] = 255;
            if (j == x) block[i][j] = 255;
        }
}

//���ڵ� ���ڹ��� �׸���
void DrawLattice(encodingResult** enData, IMG copy, int height, int width, int cy, int cx, int bsize)
{
    //��� ����
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

//psnr ����ϱ�
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
    //cout ����� ������
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

    //���Ͽ��� �б�
    ReadParameter(filename.c_str(), enData, input.height / bsize, input.width / bsize, bsize);

    //image_dec�� 128�� �ʱ�ȭ
    Init_Mat(dec, 128);

    for (int n = 0; n < 5; n++)
    {
        //�̹��� ���
        ImageShow("result", dec.image, dec.width, dec.height);
        //dec�� tmp�� �����ϱ�
        CopyMat(dec, tmp);
        //���ڵ�
        Decoding(enData, dec, tmp, input.height / bsize, input.width / bsize, 0, 0, bsize);
    }

    //���� �׸���
    CopyMat(input, copy);
    DrawLattice(enData, copy, copy.height / bsize, copy.width / bsize, 0, 0, bsize);
    ImageShow("DrawLine", copy.image, copy.width, copy.height);

    //psnr�� ����ϱ�
    cout << "psnr: " << computePSNR(input.image, tmp.image, input.width, input.height) << endl;


    IntFree2(input.image, input.width, input.height);
    IntFree2(dec.image, dec.width, dec.height);
    IntFree2(tmp.image, tmp.width, tmp.height);
    ER_Free2(enData, input.width / bsize, input.height / bsize);
}

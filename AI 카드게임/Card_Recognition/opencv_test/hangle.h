#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <windows.h>



HBITMAP setTextToBitmap(char* text, CvScalar color);	// Text To Bitmap	
IplImage * setBitmapToIplImage(HBITMAP hBitmap);		// Bitmap to IplImage


// OpenCV���� �ѱ� ����� ���� �߰���..

// �ѱ� ����� ���� ���� �Լ� 

void setPutText(IplImage* src_image,					// ������
                         IplImage *dst_image,		// ���� ����
                         char * text,				// �ؽ�Ʈ 
                         CvPoint textPos,			// �ؽ�Ʈ ��ġ
                         CvScalar color)			// �ؽ�Ʈ ���� 
{

    //if (src_image->nChannels == 1)									// ��ϵ� �����϶�
    //    cvCvtPlaneToPix(src_image, src_image, src_image, NULL, dst_image);

/*    if (src_image->nChannels != 1)	*/							// �÷� �����϶�
        cvCopy(src_image, dst_image);

    int width = src_image->width;
    int height = src_image->height;

    // ���� �ʰ��� ó��.
    if ((textPos.y > src_image->height) || (textPos.x > src_image->width))// �ؽ�Ʈ ũ�Ⱑ �׸� ũ�⺸�� ũ�ٸ�..
    {
        cvCopy(src_image, dst_image);
    }

    // �޸� �����ϴ� ���� ������ ��...

    CvScalar var = { 0,0,0,0 };
    HBITMAP hBitmap = setTextToBitmap(text, color);			// Text to Bimap
    IplImage * text_image = setBitmapToIplImage(hBitmap);	// Bitmap to IplImage	




                                                            // �ؽ�Ʈ ������ ���ɿ������� ����.

    for (int nRow = textPos.y; nRow < (textPos.y + text_image->height); nRow++)

    {

        for (int nCol = textPos.x; nCol < (textPos.x + text_image->width); nCol++)

        {

            if (nRow >= height || nCol >= width)	continue;

            var = cvGet2D(text_image, nRow - textPos.y, nCol - textPos.x);




            if (var.val[0] == color.val[0] &&

                var.val[1] == color.val[1] &&

                var.val[2] == color.val[2])

                cvSet2D(dst_image, nRow, nCol, var);

        }

    }



    free(text_image->imageData);			// �̹��� �����Ͱ� ���� ������ �����Ƿ� �޸� ����..(				// �̺κ��� ������ GDI�� �������� ������ �޸𸮴� �þ.

    cvReleaseImage(&text_image);

    DeleteObject(hBitmap);

}




// Text To Bitmap �ؽ�Ʈ�� ��Ʈ������ ó��..

HBITMAP setTextToBitmap(char* text, CvScalar color)

{

    int textLength = (int)strlen(text);




    if (textLength <= 0) return NULL;




    HDC hTextDC = CreateCompatibleDC(NULL);



    HFONT hOldFont = (HFONT)SelectObject(hTextDC, (HFONT)GetStockObject(SYSTEM_FONT));// �ý��� ��Ʈ�� ����

    HBITMAP hDstBMP = NULL;




    RECT textArea = { 0, 0, 0, 0 };

    DrawText(hTextDC, (LPCTSTR)text, textLength, &textArea, DT_CALCRECT);			// DC�� �ؽ�Ʈ �ӽ÷� ����..




    if ((textArea.right > textArea.left) && (textArea.bottom > textArea.top))

    {

        BITMAPINFOHEADER  bitmapInfoHeader;

        memset(&bitmapInfoHeader, 0x0, sizeof(BITMAPINFOHEADER));




        void *lpvBits = NULL;




        bitmapInfoHeader.biSize = sizeof(bitmapInfoHeader);

        bitmapInfoHeader.biWidth = textArea.right - textArea.left;

        bitmapInfoHeader.biHeight = textArea.bottom - textArea.top;

        bitmapInfoHeader.biPlanes = 1;

        bitmapInfoHeader.biBitCount = 32;

        bitmapInfoHeader.biCompression = BI_RGB;



        hDstBMP = CreateDIBSection(hTextDC,

            (LPBITMAPINFO)&bitmapInfoHeader,

                                   0,

                                   (LPVOID*)&lpvBits,

                                   NULL,

                                   0);



        HBITMAP hOldBMP = (HBITMAP)SelectObject(hTextDC, hDstBMP);	// DC���� ��Ʈ�� ������ ������




        if (hOldBMP != NULL)

        {

            int TEXT_RED = (int)color.val[2];

            int TEXT_GREEN = (int)color.val[1];

            int TEXT_BLUE = (int)color.val[0];



            SetTextColor(hTextDC, RGB(TEXT_RED, TEXT_GREEN, TEXT_BLUE));		// ���ڻ���.

            SetBkColor(hTextDC, 0x00EFFEFF);									// ��� ����

            SetBkMode(hTextDC, OPAQUE);											// �����ϰ� ó��




            DrawText(hTextDC, (LPCTSTR)text, textLength, &textArea, DT_NOCLIP);

        }

        ::SelectObject(hTextDC, hOldBMP);



    }

    ::SelectObject(hTextDC, hOldFont);

    if (hTextDC)
    {
        ::DeleteDC(hTextDC);
    }

    if (hOldFont)
    {
        ::DeleteObject(hOldFont);
    }




    return hDstBMP;

}




// Bitmap to IplImage	�ѱ� ����� �� ������ ��Ʈ������ ����

IplImage* setBitmapToIplImage(HBITMAP hBitmap)

{

    BITMAP bitmap;

    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bitmap);




    int nChannels = (bitmap.bmBitsPixel == 1) ? 1 : bitmap.bmBitsPixel * 0.125;

    int depth = (bitmap.bmBitsPixel == 1) ? IPL_DEPTH_1U : IPL_DEPTH_8U;



    IplImage *	mImage = cvCreateImageHeader(cvSize(bitmap.bmWidth, bitmap.bmHeight),

                                             depth, nChannels);



    mImage->imageData = (char*)malloc(bitmap.bmHeight * bitmap.bmWidth * nChannels * sizeof(char));

    memcpy(mImage->imageData, (char*)(bitmap.bmBits), bitmap.bmHeight * bitmap.bmWidth * nChannels);




    cvFlip(mImage, mImage, 0);											// ���� ������

    return mImage;

}
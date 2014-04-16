#include "stdio.h"

#include "cvcam.h"
#include "highgui.h"
#include "cxcore.h"
#include "time.h"
#include "cvaux.h"
#include "cv.h"

#include <vector>

using namespace std;

struct ppt{
	int x1;
	int y1;
	int x2;
	int y2;
	int x3;
	int y3;
	int x4;
	int y4;	
};

IplImage* iplTemp,* iplTemp2;

int capture=0;
void camcallback(IplImage* image1);
void detect_prepare(IplImage* image1);

IplImage *frame_copy = 0, *frame_copy2 = 0;
CvHaarClassifierCascade* cascade = NULL;
CvHaarClassifierCascade* cascade2 = NULL;
vector< CvRect > Frontal_Face1;	
vector< ppt > Frontal_Face45;
vector< ppt > l_Face90;
vector< ppt > r_Face90;

vector< ppt > Frontal_Face45r;
vector< ppt > l_Face90r;
vector< ppt > r_Face90r;

vector< CvRect > r_90_Face;
vector< CvRect > l_90_Face;
int i,j;
BOOL b;
static CvMemStorage* storage = 0;
static CvMemStorage* storage2 = 0;
int width, height;

vector< pair<int, int> > cp;

void detect_and_draw_objects_All(IplImage* image, 
								 IplImage* image2, 
								 CvHaarClassifierCascade* cascade,
								 CvHaarClassifierCascade* cascade3,
								 vector<CvRect> &r1, vector<CvRect> &r3, vector<CvRect> &r4,
								 vector< ppt > &r5,	 vector< ppt > &r6, vector<ppt> &r7,
								 vector< ppt > &r8,	 vector< ppt > &r9, vector<ppt> &r10,
								 int do_pyramids );

IplImage* GS_rotate( IplImage *src_image, int angle );
IplImage *GS_createImage( CvSize size, int nChannels );

CvPoint pts[4];
CvPoint * rect=pts;
int npts=4;

CvPoint pts2[5];
CvPoint * rect2=pts2;
int npts2=5;

void main()
{

	//////////////////////////////////////////////////////////////////////////
	int ncams = cvcamGetCamerasCount();
	printf("현재 카메라 개수 = %d\n\n",ncams);
	printf("key d is to detect MVFD \n");
	printf("key s is stop \n");	
	printf("key b is escape \n");


	//////////////////////////////////////////////////////////////////////////
	//1개 카메라만 선택
	int *out;
	int nselected = cvcamSelectCamera(&out);
	cvNamedWindow("MultiView Face Detection ONE", 1);
	HWND hwnd1 = (HWND)cvGetWindowHandle("MultiView Face Detection ONE");
	cvcamSetProperty(out[0], CVCAM_PROP_ENABLE, CVCAMTRUE);
	cvcamSetProperty(out[0], CVCAM_PROP_RENDER, CVCAMTRUE );
	cvcamSetProperty(out[0], CVCAM_PROP_WINDOW, &hwnd1);
	cvcamSetProperty(out[0], CVCAM_PROP_CALLBACK,camcallback);

	//////////////////////////////////////////////////////////////////////////
	//캐스케이트 파일 로딩
	cascade =
		(CvHaarClassifierCascade*)cvLoad(".\\haarcascade_frontalface_default.xml");
	cascade2 =(CvHaarClassifierCascade*)cvLoad(".\\haarcascade_profileface.xml");
	storage = cvCreateMemStorage(0);

	cvcamInit(); 
	cvcamStart();
 

	int c=0;
	int A=1;
	while(A)
	{
		c = cvWaitKey(10);
		
		switch( (char) c )
		{
		case 'd':
			printf("Start Detection!\n");
			capture=1;
			break;		
		case 's':
			printf("Stop Detection!\n");
			capture=0;
			break;
		case 'b':
			printf("break!\n");
			A=0;
			break;       
		default:
			;
		}
	}
	

	cvcamStop( );
	cvcamExit( );


	cvReleaseImage( &frame_copy );
	cvReleaseImage( &frame_copy2 );
}

void camcallback(IplImage* image1)
{

	// 가로 세로 크기
	width = image1->width;
	height = image1->height;

	int flip;
	//이미지 할당
	if( frame_copy == 0)
	{
		//복사할 이미지 할당 
		frame_copy = cvCreateImage( cvSize(image1->width,image1->height),
			IPL_DEPTH_8U, image1->nChannels );		
		frame_copy2 = cvCreateImage( cvSize(image1->width,image1->height),
			IPL_DEPTH_8U, image1->nChannels );
		printf("allocation\n");
	}

	
	// 뒤집혀 있으면 바로 잡는다. 
	if( image1->origin == IPL_ORIGIN_TL )
	{
		cvCopy( image1, frame_copy, 0 );
		flip=0;
	}
	else
	{		
		cvFlip( image1, frame_copy, 0 );
		flip=1;		
	}
	cvFlip( frame_copy, frame_copy2, 1 );
	
	Frontal_Face1.clear();		
	r_90_Face.clear();
	l_90_Face.clear();

	Frontal_Face45.clear();
	l_Face90.clear();
	r_Face90.clear();

	Frontal_Face45r.clear();
	l_Face90r.clear();
	r_Face90r.clear();


	detect_and_draw_objects_All(frame_copy, frame_copy2, cascade, cascade2,
		Frontal_Face1, r_90_Face, l_90_Face,
		Frontal_Face45,r_Face90, l_Face90,
		Frontal_Face45r,r_Face90r, l_Face90r,
		1);


	//////////////////////////////////////////////////////////////////////////
	//정면 0도 
	for(i=0 ; i<Frontal_Face1.size(); ++i)
	{
		if(flip)
				Frontal_Face1[i].y = height - Frontal_Face1[i].height - Frontal_Face1[i].y;

		cvRectangle( image1, 
			cvPoint(Frontal_Face1[i].x,Frontal_Face1[i].y),
			cvPoint((Frontal_Face1[i].x+Frontal_Face1[i].width),(Frontal_Face1[i].y+Frontal_Face1[i].height)),
			CV_RGB(0,0,255), 2 );
	}


	for(i=0 ; i<r_90_Face.size(); ++i)
	{	
		if(flip)
				r_90_Face[i].y = height - r_90_Face[i].height - r_90_Face[i].y;

		r_90_Face[i].x += 20;
		r_90_Face[i].width-=20;
		
		pts2[0].x = r_90_Face[i].x;
		pts2[0].y = r_90_Face[i].y;
		
		pts2[1].x = r_90_Face[i].x+r_90_Face[i].width;
		pts2[1].y = r_90_Face[i].y;			
		
		pts2[2].x = r_90_Face[i].x+r_90_Face[i].width;
		pts2[2].y = r_90_Face[i].y+r_90_Face[i].height;
		
		pts2[3].x = r_90_Face[i].x;
		pts2[3].y = r_90_Face[i].y+r_90_Face[i].height;
		
		pts2[4].x = r_90_Face[i].x-20;
		pts2[4].y = r_90_Face[i].y+r_90_Face[i].height/2;
		
		cvPolyLine( image1, &rect2, &npts2,1,1, CV_RGB(0,255,0),2);
	}

	for(i=0 ; i<l_90_Face.size(); ++i)
	{
		if(flip)
				l_90_Face[i].y = height - l_90_Face[i].height - l_90_Face[i].y;

		l_90_Face[i].x = width - l_90_Face[i].x - l_90_Face[i].width;
		l_90_Face[i].width -= 20;
		
		pts2[0].x = l_90_Face[i].x;
		pts2[0].y = l_90_Face[i].y;
		
		pts2[1].x = l_90_Face[i].x+l_90_Face[i].width;
		pts2[1].y = l_90_Face[i].y;
		
		pts2[2].x = l_90_Face[i].x+l_90_Face[i].width+20;
		pts2[2].y = l_90_Face[i].y+l_90_Face[i].height/2;
		
		pts2[3].x = l_90_Face[i].x+l_90_Face[i].width;
		pts2[3].y = l_90_Face[i].y+l_90_Face[i].height;
		
		pts2[4].x = l_90_Face[i].x;
		pts2[4].y = l_90_Face[i].y+l_90_Face[i].height;
		
		cvPolyLine( image1, &rect2, &npts2,1,1, CV_RGB(255,255,0),2);
			}


	//////////////////////////////////////////////////////////////////////////
	//정면 45도 	
	for(i=0 ; i<Frontal_Face45.size(); ++i)
	{		
		if(flip){
			Frontal_Face45[i].y1 = height - Frontal_Face45[i].y1;
			Frontal_Face45[i].y2 = height - Frontal_Face45[i].y2;
			Frontal_Face45[i].y3 = height - Frontal_Face45[i].y3;
			Frontal_Face45[i].y4 = height - Frontal_Face45[i].y4;
		}
		
		pts[0].x  = Frontal_Face45[i].x1;
		pts[0].y  = Frontal_Face45[i].y1;
		
		pts[1].x  = Frontal_Face45[i].x2;
		pts[1].y  = Frontal_Face45[i].y2;
		
		pts[2].x  = Frontal_Face45[i].x3;
		pts[2].y  = Frontal_Face45[i].y3;
		
		pts[3].x  = Frontal_Face45[i].x4;
		pts[3].y  = Frontal_Face45[i].y4;

		cvPolyLine( image1, &rect, &npts,1,1, CV_RGB(0,0,255),2);
	}


	//////////////////////////////////////////////////////////////////////////
	//정면 45도 	-
	for(i=0 ; i<Frontal_Face45r.size(); ++i)
	{		
		if(flip){
			Frontal_Face45r[i].y1 = height - Frontal_Face45r[i].y1;
			Frontal_Face45r[i].y2 = height - Frontal_Face45r[i].y2;
			Frontal_Face45r[i].y3 = height - Frontal_Face45r[i].y3;
			Frontal_Face45r[i].y4 = height - Frontal_Face45r[i].y4;
		}
		
		pts[0].x  = Frontal_Face45r[i].x1;
		pts[0].y  = Frontal_Face45r[i].y1;
		
		pts[1].x  = Frontal_Face45r[i].x2;
		pts[1].y  = Frontal_Face45r[i].y2;
		
		pts[2].x  = Frontal_Face45r[i].x3;
		pts[2].y  = Frontal_Face45r[i].y3;
		
		pts[3].x  = Frontal_Face45r[i].x4;
		pts[3].y  = Frontal_Face45r[i].y4;
		
		cvPolyLine( image1, &rect, &npts,1,1, CV_RGB(0,0,255),2);
	}
	
	/*
	for(i=0 ; i<r_Face90.size(); ++i)
	{		
		if(flip){
			r_Face90[i].y1 = height - r_Face90[i].y1;
			r_Face90[i].y2 = height - r_Face90[i].y2;
			r_Face90[i].y3 = height - r_Face90[i].y3;
			r_Face90[i].y4 = height - r_Face90[i].y4;
		}
		
		pts2[0].x  = r_Face90[i].x4;
		pts2[0].y  = r_Face90[i].y1+(r_Face90[i].y4-r_Face90[i].y1)/4;

		pts2[1].x  = r_Face90[i].x1;
		pts2[1].y  = r_Face90[i].y1;
		
		pts2[2].x  = r_Face90[i].x2;
		pts2[2].y  = r_Face90[i].y2;
		
		pts2[3].x  = r_Face90[i].x3;
		pts2[3].y  = r_Face90[i].y3;
		
		pts2[4].x  = r_Face90[i].x4;
		pts2[4].y  = r_Face90[i].y4;
		
		cvPolyLine( image1, &rect2, &npts2,1,1, CV_RGB(0,255,0),2);
	}

	for(i=0 ; i<l_Face90.size(); ++i)
	{		
		if(flip){
			l_Face90[i].y1 = height - l_Face90[i].y1;
			l_Face90[i].y2 = height - l_Face90[i].y2;
			l_Face90[i].y3 = height - l_Face90[i].y3;
			l_Face90[i].y4 = height - l_Face90[i].y4;
		}
		
		pts2[0].x  = l_Face90[i].x2;
		pts2[0].y  = l_Face90[i].y1+(l_Face90[i].y3-l_Face90[i].y2)/4;
		
		pts2[1].x  = l_Face90[i].x1;
		pts2[1].y  = l_Face90[i].y1;
		
		pts2[2].x  = l_Face90[i].x2;
		pts2[2].y  = l_Face90[i].y2;
		
		pts2[3].x  = l_Face90[i].x3;
		pts2[3].y  = l_Face90[i].y3;
		
		pts2[4].x  = l_Face90[i].x4;
		pts2[4].y  = l_Face90[i].y4;
		
		cvPolyLine( image1, &rect2, &npts2,1,1, CV_RGB(255,255,0),2);
		
	}
	*/

	
	//detect_prepare(image1);
	//90도 회전 얼굴 검출
	//180도 회전 얼굴 검출 
	//270도 회전 얼굴 검출

	
}


void detect_and_draw_objects_All(IplImage* image, IplImage* image2,
							   CvHaarClassifierCascade* cascade ,CvHaarClassifierCascade* cascade3,
							   vector<CvRect> &r1, vector<CvRect> &r3, vector<CvRect> &r4,
							   vector< ppt > &r5, vector< ppt > &r6, vector< ppt > &r7,
							   vector< ppt > &r8, vector< ppt > &r9, vector< ppt > &r10,
							   int do_pyramids )
{
	vector<CvRect> returnRect;
	double scale = 1;
	IplImage* gray = cvCreateImage( cvSize(image->width,image->height), 8, 1 );
    IplImage* small_img_R = cvCreateImage( cvSize( cvRound (image->width/scale),
		cvRound (image->height/scale)),8, 1 );
	IplImage* small_img_L = cvCreateImage( cvSize( cvRound (image->width/scale),
		cvRound (image->height/scale)),8, 1 );

    int i;	
    cvCvtColor( image, gray, CV_BGR2GRAY );
    cvResize( gray, small_img_R, CV_INTER_LINEAR );
	cvCvtColor( image2, gray, CV_BGR2GRAY );
    cvResize( gray, small_img_L, CV_INTER_LINEAR );

	//cvClearMemStorage( storage2 );
	
	CvSeq* faces=NULL;
	CvSeq* faces2=NULL;
	CvSeq* faces3=NULL;
	
	//use the fastest variant
	faces = cvHaarDetectObjects(small_img_R,cascade,storage,1.2,2,CV_HAAR_DO_CANNY_PRUNING);	
	faces2 = cvHaarDetectObjects(small_img_R,cascade3,storage,1.2,2,CV_HAAR_DO_CANNY_PRUNING);
	faces3 = cvHaarDetectObjects(small_img_L,cascade3,storage,1.2,2,CV_HAAR_DO_CANNY_PRUNING);

	cp.clear();
	//draw all the rectangles 
	for( i = 0; i < faces->total; i++ )
	{
		// extract the rectanlges only 
		CvRect face_rect = *(CvRect*)cvGetSeqElem( faces, i);

		cp.push_back(make_pair(face_rect.x+face_rect.width/2, face_rect.y+face_rect.height/2) );

		face_rect.x *= scale;
		face_rect.y *= scale;
		face_rect.width *= scale;
		face_rect.height *= scale;		
		r1.push_back(face_rect);		
	}

	for( i = 0; i < faces2->total; i++ )
	{
		// extract the rectanlges only 
		CvRect face_rect3 = *(CvRect*)cvGetSeqElem( faces2, i);
		b=TRUE;
		for(j=0; j<cp.size(); ++j)
		{
			if( (face_rect3.x < cp[j].first && (face_rect3.x+face_rect3.width) > cp[j].first ) &&
				(face_rect3.y < cp[j].second && (face_rect3.y+face_rect3.height) > cp[j].second) )
			{
				b=FALSE;
				break;
			}
		}		
		if(!b)
			continue;
		else
			cp.push_back(make_pair(face_rect3.x+face_rect3.width/2, face_rect3.y+face_rect3.height/2) );

		face_rect3.x *= scale;
		face_rect3.y *= scale;
		face_rect3.width *= scale;
		face_rect3.height *= scale;
		
		r3.push_back(face_rect3);
	}

	for( i = 0; i < faces3->total; i++ )
	{		
		// extract the rectanlges only 
		CvRect face_rect3 = *(CvRect*)cvGetSeqElem( faces3, i);
		
		b=TRUE;		
		for(j=0; j<cp.size(); ++j)
		{
			if( (face_rect3.x < cp[j].first && (face_rect3.x+face_rect3.width) > cp[j].first ) &&
				(face_rect3.y < cp[j].second && (face_rect3.y+face_rect3.height) > cp[j].second) )
			{
				b=FALSE;
				break;
			}
		}		
		if(!b)
			continue;
		else
			cp.push_back(make_pair(face_rect3.x+face_rect3.width/2, face_rect3.y+face_rect3.height/2) );
			

		face_rect3.x *= scale;
		face_rect3.y *= scale;
		face_rect3.width *= scale;
		face_rect3.height *= scale;		
		r4.push_back(face_rect3);
	}	

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	iplTemp=GS_rotate(small_img_R, 45);
	//iplTemp2=GS_rotate(small_img_L, 45);
	faces=NULL;
	faces2=NULL;
	faces3=NULL;
	
	//use the fastest variant
	faces = cvHaarDetectObjects(iplTemp,cascade,storage,1.2,2,CV_HAAR_DO_CANNY_PRUNING);	
	//faces2 = cvHaarDetectObjects(iplTemp,cascade3,storage,1.2,2,CV_HAAR_DO_CANNY_PRUNING);
	//faces3 = cvHaarDetectObjects(iplTemp2,cascade3,storage,1.2,2,CV_HAAR_DO_CANNY_PRUNING);
	cvReleaseImage( &iplTemp );
	//cvReleaseImage( &iplTemp2 );

	double M_PI = 3.141592654;
	double radius = -45*(M_PI/180.0);
	double sin_value = sin(radius);
	double cos_value = cos(radius);
	
	// 중심 화소를 얻는다.
	int centerX = small_img_R->height/2;
	int centerY = small_img_R->width/2;
	
	ppt temp;
	//frontal
	//cp.clear();
	for( i = 0; i < faces->total; i++ )
	{
		// extract the rectanlges only 
		CvRect face_rect = *(CvRect*)cvGetSeqElem( faces, i);

		b=TRUE;		
		for(j=0; j<cp.size(); ++j)
		{
			if( (face_rect.x < cp[j].first && (face_rect.x+face_rect.width) > cp[j].first ) &&
				(face_rect.y < cp[j].second && (face_rect.y+face_rect.height) > cp[j].second) )
			{
				b=FALSE;
				break;
			}
		}		
		if(!b)
			continue;
		else
			cp.push_back(make_pair(face_rect.x+face_rect.width/2, face_rect.y+face_rect.height/2) );

		

		//////////////////////////////////////////////////////////////////////////		
		temp.y1 = ((face_rect.y-centerX)*cos_value - (face_rect.x-centerY)*sin_value + centerX)*scale;
		temp.x1 = ((face_rect.y-centerX)*sin_value + (face_rect.x-centerY)*cos_value + centerY)*scale;

		temp.y2 = ((face_rect.y-centerX)*cos_value - (face_rect.x+face_rect.width-centerY)*sin_value + centerX)*scale;
		temp.x2 = ((face_rect.y-centerX)*sin_value + (face_rect.x+face_rect.width-centerY)*cos_value + centerY)*scale;

		temp.y3 = ((face_rect.y+face_rect.height-centerX)*cos_value - (face_rect.x+face_rect.width-centerY)*sin_value + centerX)*scale;
		temp.x3 = ((face_rect.y+face_rect.height-centerX)*sin_value + (face_rect.x+face_rect.width-centerY)*cos_value + centerY)*scale;

		temp.y4 = ((face_rect.y+face_rect.height-centerX)*cos_value - (face_rect.x-centerY)*sin_value + centerX)*scale;
		temp.x4 = ((face_rect.y+face_rect.height-centerX)*sin_value + (face_rect.x-centerY)*cos_value + centerY)*scale;	
		
		r5.push_back(temp);
	}

	/*
	//right
	for( i = 0; i < faces2->total; i++ )
	{
		// extract the rectanlges only 
		CvRect face_rect = *(CvRect*)cvGetSeqElem( faces2, i);

		b=TRUE;		
		for(j=0; j<cp.size(); ++j)
		{
			if( (face_rect.x < cp[j].first && (face_rect.x+face_rect.width) > cp[j].first ) &&
				(face_rect.y < cp[j].second && (face_rect.y+face_rect.height) > cp[j].second) )
			{
				b=FALSE;
				break;
			}
		}		
		if(!b)
			continue;
		else
			cp.push_back(make_pair(face_rect.x+face_rect.width/2, face_rect.y+face_rect.height/2) );
		

		//////////////////////////////////////////////////////////////////////////		
		temp.y1 = ((face_rect.y-centerX)*cos_value - (face_rect.x-centerY)*sin_value + centerX)*scale;
		temp.x1 = ((face_rect.y-centerX)*sin_value + (face_rect.x-centerY)*cos_value + centerY)*scale;
		
		temp.y2 = ((face_rect.y-centerX)*cos_value - (face_rect.x+face_rect.width-centerY)*sin_value + centerX)*scale;
		temp.x2 = ((face_rect.y-centerX)*sin_value + (face_rect.x+face_rect.width-centerY)*cos_value + centerY)*scale;
		
		temp.y3 = ((face_rect.y+face_rect.height-centerX)*cos_value - (face_rect.x+face_rect.width-centerY)*sin_value + centerX)*scale;
		temp.x3 = ((face_rect.y+face_rect.height-centerX)*sin_value + (face_rect.x+face_rect.width-centerY)*cos_value + centerY)*scale;
		
		temp.y4 = ((face_rect.y+face_rect.height-centerX)*cos_value - (face_rect.x-centerY)*sin_value + centerX)*scale;
		temp.x4 = ((face_rect.y+face_rect.height-centerX)*sin_value + (face_rect.x-centerY)*cos_value + centerY)*scale;	
		
		r6.push_back(temp);
	}
	

	//right
	for( i = 0; i < faces3->total; i++ )
	{
		// extract the rectanlges only 
		CvRect face_rect = *(CvRect*)cvGetSeqElem( faces3, i);

		//printf("%d \n", faces3->total);

		
		b=TRUE;		
		for(j=0; j<cp.size(); ++j)
		{
			if( (face_rect.x < cp[j].first && (face_rect.x+face_rect.width) > cp[j].first ) &&
				(face_rect.y < cp[j].second && (face_rect.y+face_rect.height) > cp[j].second) )
			{
				b=FALSE;
				break;
			}
		}		
		if(!b)
			continue;
		else
			cp.push_back(make_pair(face_rect.x+face_rect.width/2, face_rect.y+face_rect.height/2) );
		
		
		//////////////////////////////////////////////////////////////////////////		
		temp.y1 = ((face_rect.y-centerX)*cos_value - (face_rect.x-centerY)*sin_value + centerX)*scale;
		temp.x1 = ((face_rect.y-centerX)*sin_value + (face_rect.x-centerY)*cos_value + centerY)*scale;
		
		temp.y2 = ((face_rect.y-centerX)*cos_value - (face_rect.x+face_rect.width-centerY)*sin_value + centerX)*scale;
		temp.x2 = ((face_rect.y-centerX)*sin_value + (face_rect.x+face_rect.width-centerY)*cos_value + centerY)*scale;
		
		temp.y3 = ((face_rect.y+face_rect.height-centerX)*cos_value - (face_rect.x+face_rect.width-centerY)*sin_value + centerX)*scale;
		temp.x3 = ((face_rect.y+face_rect.height-centerX)*sin_value + (face_rect.x+face_rect.width-centerY)*cos_value + centerY)*scale;
		
		temp.y4 = ((face_rect.y+face_rect.height-centerX)*cos_value - (face_rect.x-centerY)*sin_value + centerX)*scale;
		temp.x4 = ((face_rect.y+face_rect.height-centerX)*sin_value + (face_rect.x-centerY)*cos_value + centerY)*scale;	
		
		r7.push_back(temp);
	}
	*/


	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	iplTemp=GS_rotate(small_img_R, -45);
	//iplTemp2=GS_rotate(small_img_L, -45);
	faces=NULL;
	faces2=NULL;
	faces3=NULL;
	
	//use the fastest variant
	faces = cvHaarDetectObjects(iplTemp,cascade,storage,1.2,2,CV_HAAR_DO_CANNY_PRUNING);	
	//faces2 = cvHaarDetectObjects(iplTemp,cascade3,storage,1.2,2,CV_HAAR_DO_CANNY_PRUNING);
	//faces3 = cvHaarDetectObjects(iplTemp2,cascade3,storage,1.2,2,CV_HAAR_DO_CANNY_PRUNING);
	cvReleaseImage( &iplTemp );
	//cvReleaseImage( &iplTemp2 );

	//double M_PI = 3.141592654;
	radius = 45*(M_PI/180.0);
	sin_value = sin(radius);
	cos_value = cos(radius);
	
	// 중심 화소를 얻는다.
	//int centerX = small_img_R->height/2;
	//int centerY = small_img_R->width/2;
	
	//ppt temp;
	//frontal
	//cp.clear();
	for( i = 0; i < faces->total; i++ )
	{
		// extract the rectanlges only 
		CvRect face_rect = *(CvRect*)cvGetSeqElem( faces, i);

		b=TRUE;		
		for(j=0; j<cp.size(); ++j)
		{
			if( (face_rect.x < cp[j].first && (face_rect.x+face_rect.width) > cp[j].first ) &&
				(face_rect.y < cp[j].second && (face_rect.y+face_rect.height) > cp[j].second) )
			{
				b=FALSE;
				break;
			}
		}		
		if(!b)
			continue;
		else
			cp.push_back(make_pair(face_rect.x+face_rect.width/2, face_rect.y+face_rect.height/2) );

		

		//////////////////////////////////////////////////////////////////////////		
		temp.y1 = ((face_rect.y-centerX)*cos_value - (face_rect.x-centerY)*sin_value + centerX)*scale;
		temp.x1 = ((face_rect.y-centerX)*sin_value + (face_rect.x-centerY)*cos_value + centerY)*scale;

		temp.y2 = ((face_rect.y-centerX)*cos_value - (face_rect.x+face_rect.width-centerY)*sin_value + centerX)*scale;
		temp.x2 = ((face_rect.y-centerX)*sin_value + (face_rect.x+face_rect.width-centerY)*cos_value + centerY)*scale;

		temp.y3 = ((face_rect.y+face_rect.height-centerX)*cos_value - (face_rect.x+face_rect.width-centerY)*sin_value + centerX)*scale;
		temp.x3 = ((face_rect.y+face_rect.height-centerX)*sin_value + (face_rect.x+face_rect.width-centerY)*cos_value + centerY)*scale;

		temp.y4 = ((face_rect.y+face_rect.height-centerX)*cos_value - (face_rect.x-centerY)*sin_value + centerX)*scale;
		temp.x4 = ((face_rect.y+face_rect.height-centerX)*sin_value + (face_rect.x-centerY)*cos_value + centerY)*scale;	
		
		r8.push_back(temp);
	}

	/*
	//right
	for( i = 0; i < faces2->total; i++ )
	{
		// extract the rectanlges only 
		CvRect face_rect = *(CvRect*)cvGetSeqElem( faces2, i);

		b=TRUE;		
		for(j=0; j<cp.size(); ++j)
		{
			if( (face_rect.x < cp[j].first && (face_rect.x+face_rect.width) > cp[j].first ) &&
				(face_rect.y < cp[j].second && (face_rect.y+face_rect.height) > cp[j].second) )
			{
				b=FALSE;
				break;
			}
		}		
		if(!b)
			continue;
		else
			cp.push_back(make_pair(face_rect.x+face_rect.width/2, face_rect.y+face_rect.height/2) );
		

		//////////////////////////////////////////////////////////////////////////		
		temp.y1 = ((face_rect.y-centerX)*cos_value - (face_rect.x-centerY)*sin_value + centerX)*scale;
		temp.x1 = ((face_rect.y-centerX)*sin_value + (face_rect.x-centerY)*cos_value + centerY)*scale;
		
		temp.y2 = ((face_rect.y-centerX)*cos_value - (face_rect.x+face_rect.width-centerY)*sin_value + centerX)*scale;
		temp.x2 = ((face_rect.y-centerX)*sin_value + (face_rect.x+face_rect.width-centerY)*cos_value + centerY)*scale;
		
		temp.y3 = ((face_rect.y+face_rect.height-centerX)*cos_value - (face_rect.x+face_rect.width-centerY)*sin_value + centerX)*scale;
		temp.x3 = ((face_rect.y+face_rect.height-centerX)*sin_value + (face_rect.x+face_rect.width-centerY)*cos_value + centerY)*scale;
		
		temp.y4 = ((face_rect.y+face_rect.height-centerX)*cos_value - (face_rect.x-centerY)*sin_value + centerX)*scale;
		temp.x4 = ((face_rect.y+face_rect.height-centerX)*sin_value + (face_rect.x-centerY)*cos_value + centerY)*scale;	
		
		r9.push_back(temp);
	}
	

	//right
	for( i = 0; i < faces3->total; i++ )
	{
		// extract the rectanlges only 
		CvRect face_rect = *(CvRect*)cvGetSeqElem( faces3, i);

		//printf("%d \n", faces3->total);

		
		b=TRUE;		
		for(j=0; j<cp.size(); ++j)
		{
			if( (face_rect.x < cp[j].first && (face_rect.x+face_rect.width) > cp[j].first ) &&
				(face_rect.y < cp[j].second && (face_rect.y+face_rect.height) > cp[j].second) )
			{
				b=FALSE;
				break;
			}
		}		
		if(!b)
			continue;
		else
			cp.push_back(make_pair(face_rect.x+face_rect.width/2, face_rect.y+face_rect.height/2) );
		
		
		//////////////////////////////////////////////////////////////////////////		
		temp.y1 = ((face_rect.y-centerX)*cos_value - (face_rect.x-centerY)*sin_value + centerX)*scale;
		temp.x1 = ((face_rect.y-centerX)*sin_value + (face_rect.x-centerY)*cos_value + centerY)*scale;
		
		temp.y2 = ((face_rect.y-centerX)*cos_value - (face_rect.x+face_rect.width-centerY)*sin_value + centerX)*scale;
		temp.x2 = ((face_rect.y-centerX)*sin_value + (face_rect.x+face_rect.width-centerY)*cos_value + centerY)*scale;
		
		temp.y3 = ((face_rect.y+face_rect.height-centerX)*cos_value - (face_rect.x+face_rect.width-centerY)*sin_value + centerX)*scale;
		temp.x3 = ((face_rect.y+face_rect.height-centerX)*sin_value + (face_rect.x+face_rect.width-centerY)*cos_value + centerY)*scale;
		
		temp.y4 = ((face_rect.y+face_rect.height-centerX)*cos_value - (face_rect.x-centerY)*sin_value + centerX)*scale;
		temp.x4 = ((face_rect.y+face_rect.height-centerX)*sin_value + (face_rect.x-centerY)*cos_value + centerY)*scale;	
		
		r10.push_back(temp);
	}
	*/


	//////////////////////////////////////////////////////////////////////////


	cvReleaseImage( &small_img_R );
	cvReleaseImage( &small_img_L );
	cvReleaseImage( &gray );
}

IplImage *GS_createImage( CvSize size, int nChannels )
{
	IplImage *dst_image = cvCreateImage( size, IPL_DEPTH_8U, nChannels );
	cvSetZero( dst_image );
	
	return dst_image;
}

IplImage* GS_rotate( IplImage *src_image, int angle )
{
	
	int i, j;
	double new_x, new_y, var;
	double M_PI = 3.141592654;
	
	// 초기화 
	int height = src_image->height;
	int width = src_image->width;
	IplImage *dst_image = GS_createImage( cvGetSize(src_image), src_image->nChannels );
	if( dst_image->nChannels != 1 ) return dst_image;
	
	// angle을 radius로 변환한다.
	// Windows OS에서는 반대이므로 270도에서 빼준다.
	double radius = -angle*(M_PI/180.0);
	double sin_value = sin(radius);
	double cos_value = cos(radius);
	
	// 중심 화소를 얻는다.
	int centerX = height/2;
	int centerY = width/2;
	
    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
			new_x = (i-centerX)*cos_value - (j-centerY)*sin_value + centerX;
			new_y = (i-centerX)*sin_value + (j-centerY)*cos_value + centerY;
			
			// 범위밖을 벗어나면 0을 할당한다.
			if(new_x < 0 || new_x > height) 
			{
				var = 0.0; 
			}
			else if(new_y < 0 || new_y > width) 
			{
				var = 0.0; 
			}
			// 범위안에 들면 new_x와 new_y에 근사한 화소값을 할당한다.
			else 
			{
				var = cvGetReal2D( src_image, (int)new_x, (int)new_y );
			}
			
			cvSetReal2D( dst_image, i, j, var );
        }
    }
	
	return dst_image;
}



void detect_prepare(IplImage* image1)
{
	
	/*
	int width, height;	
	
	if( frame_copy == 0)
	{
		//복사할 이미지 할당 
		frame_copy = cvCreateImage( cvSize(image1->width,image1->height),
			IPL_DEPTH_8U, image1->nChannels );
		
		frame_copy2 = cvCreateImage( cvSize(image1->width,image1->height),
			IPL_DEPTH_8U, image1->nChannels );
	}

	// 뒤집혀 있으면 바로 잡는다. 
	if( image1->origin == IPL_ORIGIN_TL )
	{
		cvCopy( image1, frame_copy, 0 );
		flip=0;
	}
	else
	{		
		cvFlip( image1, frame_copy, 0 );
		flip=1;
	}

	cvFlip( frame_copy, frame_copy2, 1 );

	
	// 가로 세로 크기
	width = image1->width;
	height = image1->height;
	
	
	Frontal_Face1.clear();
	r_45_Face.clear();
	r_90_Face.clear();
	l_45_Face.clear();
	l_90_Face.clear();
	Frontal_Face2.clear();


	detect_and_draw_objects_2( frame_copy, cascade,cascade3,
				Frontal_Face1,r_90_Face,1 );			
	
	detect_and_draw_objects_2( frame_copy2, cascade,cascade3,
		Frontal_Face2,l_90_Face,1 );
	

	//정면 1.
	for(int i=0 ; i<Frontal_Face1.size(); ++i)
	{
		if(flip)
			Frontal_Face1[i].y = height - Frontal_Face1[i].height - Frontal_Face1[i].y;

		cvRectangle( image1, 
			cvPoint(Frontal_Face1[i].x,Frontal_Face1[i].y),
			cvPoint((Frontal_Face1[i].x+Frontal_Face1[i].width),(Frontal_Face1[i].y+Frontal_Face1[i].height)),
			CV_RGB(0,0,255), 2 );
	}
	
	
	CvPoint pts[5];
	CvPoint * rect=pts;
	int npts=5;
	
	for(i=0 ; i<r_90_Face.size(); ++i)
	{	
		if(flip)
			r_90_Face[i].y = height - r_90_Face[i].height - r_90_Face[i].y;

		r_90_Face[i].x += 20;
		r_90_Face[i].width-=20;
		
		pts[0].x = r_90_Face[i].x;
		pts[0].y = r_90_Face[i].y;
		
		pts[1].x = r_90_Face[i].x+r_90_Face[i].width;
		pts[1].y = r_90_Face[i].y;			
		
		pts[2].x = r_90_Face[i].x+r_90_Face[i].width;
		pts[2].y = r_90_Face[i].y+r_90_Face[i].height;
		
		pts[3].x = r_90_Face[i].x;
		pts[3].y = r_90_Face[i].y+r_90_Face[i].height;
		
		pts[4].x = r_90_Face[i].x-20;
		pts[4].y = r_90_Face[i].y+r_90_Face[i].height/2;
		
		cvPolyLine( image1, &rect, &npts,1,1, CV_RGB(255,0,0),2);
	}
	
	for(i=0 ; i<l_90_Face.size(); ++i)
	{
		if(flip)
			l_90_Face[i].y = height - l_90_Face[i].height - l_90_Face[i].y;

		l_90_Face[i].x = width - l_90_Face[i].x - l_90_Face[i].width;		
		l_90_Face[i].width -= 20;
		
		pts[0].x = l_90_Face[i].x;
		pts[0].y = l_90_Face[i].y;
		
		pts[1].x = l_90_Face[i].x+l_90_Face[i].width;
		pts[1].y = l_90_Face[i].y;
		
		pts[2].x = l_90_Face[i].x+l_90_Face[i].width+20;
		pts[2].y = l_90_Face[i].y+l_90_Face[i].height/2;
		
		pts[3].x = l_90_Face[i].x+l_90_Face[i].width;
		pts[3].y = l_90_Face[i].y+l_90_Face[i].height;
		
		pts[4].x = l_90_Face[i].x;
		pts[4].y = l_90_Face[i].y+l_90_Face[i].height;
		
		cvPolyLine( image1, &rect, &npts,1,1, CV_RGB(255,0,0),2);
	}

  */
}


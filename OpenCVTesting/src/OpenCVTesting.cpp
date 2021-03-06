/**
 * Display video from webcam
 *
 * Author  Steven Clementson
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <sys/time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

#define PI 3.14159

// Computes direct distance between 2 points using Pythagoras
int distanceDirect(Point a, Point b)
{
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}

// Computes bearing heading between 2 points in degrees from North clockwise
int trajectory(Point from, Point to)
{
	int deltaX = to.x - from.x;
	int deltaY = from.y - to.y;

	if(deltaX > 0)
	{
		if(deltaY > 0)
		{
			cout << "Q1: ";
			return (180/PI)*atan(deltaX/deltaY);			// Quadrant 1
		}
		cout << "Q4: ";
		return 90 + (180/PI)*atan(-deltaY/deltaX);			// Quadrant 4
	}
	if(deltaY > 0)
	{
		cout << "Q2: ";
		return (int)(360 - (180/PI)*atan(-deltaX/deltaY)) % 360;	// Quadrant 2
	}
	cout << "Q3: ";
	if(deltaY == 0)
		return 270;
	return 180 +(180/PI)* atan(deltaX/deltaY);				// Quadrant 3
}

Point contourCentre(vector<Point> contour)
{
	Point centre(0, 0);
	for(unsigned i=0; i<contour.size(); i++)
		centre += contour[i];

	centre.x /= contour.size();
	centre.y /= contour.size();

	return centre;
}

vector<Point> findBlobContour(Mat image, Vec3b matchColour, int thres)
{
	Scalar lb(matchColour[0]-thres, matchColour[1]-thres,
			matchColour[2]-thres, matchColour[3]-thres);
	Scalar ub(matchColour[0]+thres, matchColour[1]+thres,
			matchColour[2]+thres, matchColour[3]+thres);
	Mat lowerBound(image.rows, image.cols, image.type(), lb);
	Mat upperBound(image.rows, image.cols, image.type(), ub);

	// Create binary image
	Mat mask;
	inRange(image, lowerBound, upperBound, mask);

	// Erode and Dilate
	Mat elem(4, 4, CV_8U, Scalar(1));
	morphologyEx(mask, mask, MORPH_CLOSE, elem);
	morphologyEx(mask, mask, MORPH_OPEN, elem);

	namedWindow("cleanMask", 1);
	imshow("cleanMask", mask);

	vector< vector<Point> > contours;
	findContours(mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	vector<Point> nothing;
	nothing.assign(1, Point(0,0));
	if(contours.size() < 1)
		return nothing;

	// Find largest contour
	vector<Point> largestContour = contours[0];
	for(vector< vector<Point> > :: iterator it = contours.begin();
			it != contours.end(); ++it)
	{
		if (contourArea(*it) > contourArea(largestContour))
			largestContour = *it;
	}

	return largestContour;
}

int main()
{
    VideoCapture	video = VideoCapture(0);

    if(!video.isOpened())
    {
        cout << "Cannot initialise webcam!" << endl;
        return 1;
    }

    Mat		  		frame;
    int       		key = 0;
    bool			objectSaved = false;
    Vec3b			objectColour;
    Mat 			converted;
    vector<Point>	blobContour;
    Point			blobLocation;
  //  int				bearing;
  //  int 			counter = 0;


    namedWindow("output", 1);

    video >> frame;
    Point centre;
    centre.x = frame.cols/2;
    centre.y = frame.rows/2;

    while(key != 'q')
    {
    	// ********* For latency testing *************
    	/*char filename[256];
    	struct timeval t;
    	gettimeofday(&t, NULL);
    	sprintf(filename, "%d.%06d.jpg", (int)t.tv_sec, (int)t.tv_usec);
        video >> frame;
        imwrite(filename, frame);*/
    	// *******************************************

        if(objectSaved)
        {
        	//counter = (counter+1)%10;
        	cvtColor(frame, converted, CV_BGR2Lab);
        	blobContour = findBlobContour(converted, objectColour, 15);

        	// Find centre point of contour
        	if(contourArea(blobContour) > 200)
        	{
        		Point newLocation = contourCentre(blobContour);

        		if((distanceDirect(newLocation, blobLocation) < 100)
        				|| (blobLocation.x == 0))
        		{
        			// only compute trajectory every 10 frames
        			//if(counter == 0)
        			//{
        				//bearing = trajectory(oldBlobLocation, newLocation);
        				//oldBlobLocation = newLocation;
        				//cout << bearing << endl;
        			//}
        			blobLocation = newLocation;
        		}

        		// Update desired colour
        		//objectColour = converted.at<Vec3b>(blobLocation);

        		// Draw the contour
        		vector< vector<Point> > contour;
        		contour.assign(1, blobContour);
        		drawContours(frame, contour, -1, CV_RGB(255, 0, 0), 2);
        		circle(frame, blobLocation, 5, CV_RGB(0, 255, 0), 7);
        	}
        	else
        		blobLocation = Point(0, 0);

        }
        else
        {
        	circle(frame, centre, 10, CV_RGB(255, 0, 0));
        	if(key == ' ')
        	{
        		cvtColor(frame, converted, CV_BGR2Lab);
        		objectColour = converted.at<Vec3b>(centre);
        		objectSaved = true;
        	}
        	blobLocation = centre;
        }

        imshow("output", frame);
        key = waitKey(1);
    }

    return 0;
}

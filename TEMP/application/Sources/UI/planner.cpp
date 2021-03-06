/******************************
 * Flight Planner Skeleton
 * Cooper Bills (csb88@cornell.edu)
 * Cornell University
 * 1/4/11
 ******************************/
#define cvAlgWindow "AlgorithmView"
#define cvBotAlgWindow "BottomAlgorithmView"

#include <ardrone_api.h>
#include <stdio.h>
#include <pthread.h>
#include "planner.hpp"
#include "Navdata/NavDataContainer.hpp"
#include "Tools/coopertools.hpp"
#include "xbee/xbee.hpp"
#include <sys/time.h>
#include <fstream>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>

//Globablly Accessable Variables:
extern IplImage* frontImgStream; //image from front camera (not thread safe, make copy before modifying)
extern IplImage* bottomImgStream; //image from bottom camera (if enabled)
extern int32_t ALTITUDE; //Altitude of drone in mm (defined in Navdata/navdata.c)
extern float32_t PSI; //Current Direction of Drone (-180deg to 180deg) (defined in Navdata/navdata.c)
extern NavDataContainer globalNavData; //Navdata (defined in Navdata/NavDatacontainer.cpp)
extern int newFrameAvailable; //if navdata indicates a new frame (defined in Navdata/navdata.c)

Xbee *xbee;


//*****************************************************************

using namespace cv;
using namespace std;

#define GAZ_GAIN 1
#define YAW_GAIN 1

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

//***************************************************************************************



//Thread to run separately:
void *Planner_Thread(void *params)
{
	Planner *self = (Planner *)params;
	xbee = new Xbee(7, 12.0);
	while(!newFrameAvailable) {usleep(100000);} //wait for initialization
  
 	//******** Add Your Initialization Below this Line *********
	IplImage*	iplFrame = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3);
	int       	key = 0;
	bool		objectSaved = false;
	Vec3b		objectColour;
	Mat 		converted;
	vector<Point>	blobContour;
	Point		blobLocation;
	//int			bearing;
    VideoCapture	video;
    video.open(0);

	namedWindow("output", 1);
	

  	//******** Add Your Initialization Above this Line *********

  	while(self->running) //Continue to loop until system is closed
  	{
    		if(self->enabled)
    		{

			if(!newFrameAvailable)
			{
				usleep(100000);
				printf("\n    Uh-Oh!  No New Frames \n\n");
				//If you would like to handle this situation, put it here.
				continue; //planner will only run when new data is available
			}

			cvCopy(frontImgStream, iplFrame);
			newFrameAvailable = 0; //reset
			xbee->updateFrontDeriv(); //Update xbee (if applicable)
			cvNamedWindow("newWindow", CV_WINDOW_AUTOSIZE);
			cvShowImage("newWindow", iplFrame);


			//******** Edit Below this Line **********

			/* When algorithms are enabled, This loop will loop indefinitely
			 (until algorithms are disabled and manual control is
			 regained).  The goal of this loop is to take the input from
			 the drone (images, sensors, etc.), process it, then output
			 control values.  There are 4 axis of control on our drones:
			 pitch (forward/backward), roll (left/right), yaw (turning),
			 and gaz (up/down).  An external thread reads dXXX_final and
			 sends the command to the drone for us. */

			// Commands are values from -25000 to 25000:
			// Pitch - Forward is Negative.
			// Roll - Right is Positive.
			// Yaw - Right is Positive.
			// Gaz - Up is Negative.

			// For example, we want the drone to remain in place, but hover ~1.5m:
			// A simple proportional controller:
			self->dpitch = 0; // No forward motion
			self->droll = 0; // No side-to-side motion
			self->dyaw = 0; // No turning motion
			self->dgaz = ALTITUDE - 1500; // Adjust altitude by difference from goal.
			Mat frame;
			 video >> frame;
			//Mat frame(iplFrame);
			//printf("Frame made\n");

		  	//Point centre;
		   	//centre.x = frame.cols/2;
		    	//centre.y = frame.rows/2;

			/*if(objectSaved)
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
						//bearing = trajectory(oldBlobLocation, newLocation);
						blobLocation = newLocation;
					}

					// Update desired colour
					objectColour = converted.at<Vec3b>(blobLocation);

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
			}*/

			imshow("output", frame);

			//int deltaX = blobLocation.x - centre.x;
			//int deltaY = blobLocation.y - centre.y;
			//******** Edit Above this Line **********

			//Apply commands all at once
			self->dgaz_final = self->dgaz; //- deltaY*GAZ_GAIN;
			self->dyaw_final = self->dyaw;// + deltaX*YAW_GAIN;
			self->droll_final = self->droll;
			self->dpitch_final = self->dpitch;

    } //end if enabled
    else
    {
      pthread_yield();
    }
    usleep(50000);
  }

  //De-initialization 
  printf("thread returned\n");
  return 0;
}


Planner::Planner()
{
  dpitch_final = 0;
  dyaw_final = 0;
  droll_final = 0;
  dgaz_final = 0;
  dpitch = 0;
  dyaw = 0;
  droll = 0;
  dgaz = 0;
  enabled = false;
  running = true;
  
  //Create the Planner Thread
  threadid = pthread_create( &plannerthread, NULL, Planner_Thread, (void*) this);
}

Planner::~Planner()
{
  //destructor
  running = false;
}





/******************************
 * Flight Planner Skeleton
 * Cooper Bills (csb88@cornell.edu)
 * Cornell University
 * 1/4/11
 ******************************
#define cvAlgWindow "AlgorithmView"
#define cvBotAlgWindow "BottomAlgorithmView"

#include <ardrone_api.h>
#include <stdio.h>
#include <pthread.h>
#include "planner.hpp"
#include "Navdata/NavDataContainer.hpp"
#include "Tools/coopertools.hpp"
#include "xbee/xbee.hpp"
#include <sys/time.h>
#include <fstream>

using namespace std;

//Globablly Accessable Variables:
//extern IplImage* frontImgStream; //image from front camera (not thread safe, make copy before modifying)
//extern IplImage* bottomImgStream; //image from bottom camera (if enabled)
extern int32_t ALTITUDE; //Altitude of drone in mm (defined in Navdata/navdata.c)
extern float32_t PSI; //Current Direction of Drone (-180deg to 180deg) (defined in Navdata/navdata.c)
extern NavDataContainer globalNavData; //Navdata (defined in Navdata/NavDatacontainer.cpp)
extern int newFrameAvailable; //if navdata indicates a new frame (defined in Navdata/navdata.c)

Xbee *xbee;

//Thread to run separately:
void *Planner_Thread(void *params)
{
  Planner *self = (Planner *)params;
  xbee = new Xbee(7, 12.0);
  while(frontImgStream == NULL) {usleep(100000);} //wait for initialization
  cvStartWindowThread();
  cvNamedWindow(cvAlgWindow, CV_WINDOW_AUTOSIZE);
  //cvNamedWindow(cvBotAlgWindow, CV_WINDOW_AUTOSIZE);
  
  //******** Add Your Initialization Below this Line *********


  //******** Add Your Initialization Above this Line *********

  while(self->running) //Continue to loop until system is closed
  {
    if(self->enabled)
    {
      if(!newFrameAvailable)
      {
        usleep(100000);
        if(!newFrameAvailable) printf("\n    Uh-Oh!  No New Frames \n\n");
	//If you would like to handle this situation, put it here.
        continue; //planner will only run when new data is available
      }
      newFrameAvailable = 0; //reset
      xbee->updateFrontDeriv(); //Update xbee (if applicable)
      
      //******** Edit Below this Line **********

      /* When algorithms are enabled, This loop will loop indefinitely
	 (until algorithms are disabled and manual control is
	 regained).  The goal of this loop is to take the input from
	 the drone (images, sensors, etc.), process it, then output
	 control values.  There are 4 axis of control on our drones:
	 pitch (forward/backward), roll (left/right), yaw (turning),
	 and gaz (up/down).  An external thread reads dXXX_final and
	 sends the command to the drone for us. 

      // Commands are values from -25000 to 25000:
      // Pitch - Forward is Negative.
      // Roll - Right is Positive.
      // Yaw - Right is Positive.
      // Gaz - Up is Negative.

      // For example, we want the drone to remain in place, but hover ~1.5m:
      // A simple proportional controller:
      self->dpitch = 0; // No forward motion
      self->droll = 0; // No side-to-side motion
      self->dyaw = 0; // No turning motion
      self->dgaz = ALTITUDE - 1500; // Adjust altitude by difference from goal.


      //******** Edit Above this Line **********

      //Apply commands all at once
      self->dgaz_final = self->dgaz;
      self->dyaw_final = self->dyaw;
      self->droll_final = self->droll;
      self->dpitch_final = self->dpitch;

    } //end if enabled
    else
    {
      pthread_yield();
    }
    usleep(50000);
  }

  //De-initialization 
  cvDestroyWindow(cvAlgWindow);
  cvDestroyWindow(cvBotAlgWindow);
  printf("thread returned\n");
  return 0;
}


Planner::Planner()
{
  dpitch_final = 0;
  dyaw_final = 0;
  droll_final = 0;
  dgaz_final = 0;
  dpitch = 0;
  dyaw = 0;
  droll = 0;
  dgaz = 0;
  enabled = false;
  running = true;
  
  //Create the Planner Thread
  threadid = pthread_create( &plannerthread, NULL, Planner_Thread, (void*) this);
}

Planner::~Planner()
{
  //destructor
  running = false;
}*/


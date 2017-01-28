#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <WPILib.h>
#include <CameraServer.h>
#include <IterativeRobot.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/types.hpp>//all of these might not be neccisary
#include <LiveWindow/LiveWindow.h>
#include <SmartDashboard/SendableChooser.h>
#include <SmartDashboard/SmartDashboard.h>

class Robot: public frc::IterativeRobot {//uncoment to enable vision

public:

	double Leftgo,Rightgo;
	bool   light,camswitcher;
	bool   SparkUno;
	bool   SparkDue;

	Joystick *rightDrive =new Joystick(0,2,9);
	Joystick *leftDrive  =new Joystick(1,2,9);
	Joystick *gamePad    =new Joystick(2,6,9);

	Talon *fLeft         =new Talon(0);
	Talon *fRight        =new Talon(1);
	Talon *bLeft         =new Talon(2);
	Talon *bRight        =new Talon(3);
	Spark *X             =new Spark(4);
	Spark *B             =new Spark (5);

	Encoder *encRight    =new Encoder(0,1);
	Encoder *encLeft     =new Encoder(2,3);

	DigitalOutput *lightpwm =new DigitalOutput(0);

	RobotDrive *robotDrive  =new RobotDrive(fLeft,fRight,bLeft,bRight);

	static void VisionThread(){// multithreading is required for the image proccessing so yah
				 cs::UsbCamera cam =  CameraServer::GetInstance()->StartAutomaticCapture(0); //starts capturing basic images into camera
				 cs::UsbCamera cam2 = CameraServer::GetInstance()->StartAutomaticCapture(1);
				 //cam.SetExposureManual(50);
				 int resolutionx, resolutiony, screenheight, screenwidth, cornerx, cornery, baselineI, iint[648], xpict, window, jj, kk, maxI[5], maxIposn[5], countI, max1, max1posn, max2, max2posn, temper;
				 // cam2.UsbCamera("cam2",2);

				 cam.SetResolution(640,480);
				 cam2.SetResolution(640,480);

				 cs::CvSink sinker = CameraServer::GetInstance()->GetVideo(cam);//Grabs video to sink into the mat image cruncher

				 cs::CvSource cheese = CameraServer::GetInstance()->PutVideo("Rectangle",640,480);//Serves up the images gathered your on camera

				 cv::Mat cruncher(640,480,CV_32F);//this is the magic image cruncher when converting make sure there both the same type try this next CV_16UC1 its 16nit the other is 8
				 // also there is CV_32FC1 i think its 32bit these also appere to not need the C1 so try CV_8U and the likes
				 cv::Mat cruncher2(640,480,CV_8U);



			 while(true){//image processing happens in here


					 if(sinker.GrabFrame(cruncher)==0){// if theres nothing there you got problems

						 cheese.NotifyError(sinker.GetError());//HEY LISTEN! you got some problems tell me about them
						 continue;//restarts the thread I think
				 }
					/* if(camswitcher){
						 cs::UsbCamera cam2 = CameraServer::GetInstance()->StartAutomaticCapture(2);

					 }
					 else{
						 cs::UsbCamera cam1 = CameraServer::GetInstance()->StartAutomaticCapture(1);
					 }*/

					 rectangle(cruncher, cv::Point(0, 0), cv::Point(50, 50),cv::Scalar(255, 255, 255), 5);//draw some rectangles on that thing WOOT RECTANGLES
					// rectangle(cruncher2, cv::Point(100, 100), cv::Point(150, 150),cv::Scalar(255, 255, 255), 5);
					// cvtColor(cruncher,cruncher2,cv::COLOR_BGR2GRAY);

					//	 cheese.PutFrame(cruncher2);//finally put the final modified frame
// DrC lines for image basic basic reflector finder.
					 resolutionx = 2;                   // integer for ignoring pixels
					 resolutiony = 2;
					 screenheight = 30/resolutiony;     // how big is swipe
					 screenwidth  = 230/resolutionx;    // how wide is the swipe
					 cornerx    = 20;                  // where to locate the corner of the restricted view
					 cornery    = 130;
					 baselineI   = 25;                 // baseline background intensity we want to remove.
					 for (kk=0; kk<screenwidth; kk++){ // these loops perform the line integrals
						 xpict = cornerx+kk*resolutiony;
					     for (jj=0;jj<screenheight;jj++){
					     iint[kk] = iint[kk]+cruncher2.at<uchar>(xpict,cornery+jj*resolutionx)-baselineI;
					     }
					 }
					 window = 3;
					 jj = 1;
					 countI=0;
					 maxI[0]=iint[0];
					 maxIposn[0] = 0;
					 for (kk=0; kk<screenwidth; kk++){ // these lines find the robust maxima, and process the lineintegrals
                       if(((int)(0.8*maxI[jj-1])<iint[kk])&&(maxI[jj]<iint[kk])){
                    	   maxI[jj]=iint[kk];
                    	   maxIposn[jj] = kk;
                       }
                       else{
                    	   countI++;
                       }
                       if(countI==window){
                    	   jj++;
                    	   countI = 0;
                       }
					 }
					 max1 = maxI[0];
					 max2 = max1;
					 for (kk=1; kk<jj; kk++){    // find  largest maxima
						 if (max1<maxI[kk]){
							 max1 = maxI[kk];
							 max1posn=kk;
						 }
					 }
					 maxI[max1posn] = maxI[0];   //reset that top max
					 max1posn=maxIposn[max1posn];
					 for (kk=1; kk<jj; kk++){    // find second largest maxima
						 if (max2<maxI[kk]){
					 		max2 = maxI[kk];
					 		max2posn=maxIposn[kk];
					 	}
					 }
					 if(max1posn<max2posn){      // switch them into ordered by pixel number max's
						 temper = max2posn;
						 max2posn = max1posn;
						 max1posn = temper;
					 }
					 rectangle(cruncher, cv::Point(max1posn, cornery), cv::Point(max2posn, cornery+screenheight*resolutiony),cv::Scalar(255, 255, 255), 5);//draw rectangle around max's found.
// End of DrC lines
					 	SmartDashboard::PutNumber("Point 25,25",cruncher2.at<uchar>(25,25));

			 }
		}


	void RobotInit() {
		std::thread camthread(VisionThread);//makes a new thread
		camthread.detach();//snaps the thread off to do its own thing

		chooser.AddDefault(autoNameDefault, autoNameDefault);//I don't like this look into making it logical
		chooser.AddObject(autoNameCustom, autoNameCustom);
		frc::SmartDashboard::PutData("Auto Modes", &chooser);

	}

	/*
	 * This autonomous (along with the chooser code above) shows how to select
	 * between different autonomous modes using the dashboard. The sendable
	 * chooser code works with the Java SmartDashboard. If you prefer the
	 * LabVIEW Dashboard, remove all of the chooser code and uncomment the
	 * GetString line to get the auto name from the text box below the Gyro.
	 *
	 * You can add additional auto modes by adding additional comparisons to the
	 * if-else structure below with additional strings. If using the
	 * SendableChooser make sure to add them to the chooser code above as well.
	 */
	void AutonomousInit() override {
		autoSelected = chooser.GetSelected();
		// std::string autoSelected = SmartDashboard::GetString("Auto Selector", autoNameDefault);
		std::cout << "Auto selected: " << autoSelected << std::endl;

		if (autoSelected == autoNameCustom) {
			// Custom Auto goes here
		} else {
			// Default Auto goes here
		}
	}

	void AutonomousPeriodic() {
		if (autoSelected == autoNameCustom) {
			// Custom Auto goes here
		} else {
			// Default Auto goes here
		}
	}

	void TeleopInit() {
		Leftgo      =0;
		Rightgo     =0;
		light       =0;
		SparkUno    =0;
		SparkDue    =0;
		encRight->Reset();
		encLeft->Reset();

	}

	void TeleopPeriodic() {
		Leftgo =.75*leftDrive->GetRawAxis(1);
		Rightgo=.75*rightDrive->GetRawAxis(1);
		camswitcher=rightDrive->GetRawButton(0);

		robotDrive->TankDrive(Leftgo,Rightgo);

		light   =gamePad->GetRawButton(1);
		SparkUno=gamePad->GetRawButton(2);
		SparkDue=gamePad->GetRawButton(3);



		if(light){
			lightpwm->Set(1);
		}
		else if (SparkUno){
			X ->Set(1);
		}
		else if (SparkDue){
			B ->Set(1);
		}
		else{
			lightpwm->Set(0);
			B       ->Set(0);
			X       ->Set(0);
		}

		SmartDashboard::PutNumber("encRight",encRight->GetRaw());
		SmartDashboard::PutNumber("encLeft",encLeft->GetRaw());

		SmartDashboard::PutNumber("Leftgo",Leftgo);
		SmartDashboard::PutNumber("Rightgo",Rightgo);
	}

	void TestPeriodic() {
		lw->Run();
	}

private://why is this down here?
	frc::LiveWindow* lw = LiveWindow::GetInstance();
	frc::SendableChooser<std::string> chooser;
	const std::string autoNameDefault = "Default";
	const std::string autoNameCustom = "Dance Party";
	std::string autoSelected;
};

START_ROBOT_CLASS(Robot)

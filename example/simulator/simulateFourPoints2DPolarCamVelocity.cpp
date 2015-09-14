/****************************************************************************
 *
 * $Id$
 *
 * Copyright (C) 1998-2010 Inria. All rights reserved.
 *
 * This software was developed at:
 * IRISA/INRIA Rennes
 * Projet Lagadic
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * http://www.irisa.fr/lagadic
 *
 * This file is part of the ViSP toolkit
 *
 * This file may be distributed under the terms of the Q Public License
 * as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE included in the packaging of this file.
 *
 * Licensees holding valid ViSP Professional Edition licenses may
 * use this file in accordance with the ViSP Commercial License
 * Agreement provided with the Software.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Contact visp@irisa.fr if any conditions of this licensing are
 * not clear to you.
 *
 * Description:
 * Simulation of a visual servoing with visualization.
 *
 * Authors:
 * Eric Marchand
 * Fabien Spindler
 *
 *****************************************************************************/

/*!
  \example simulateFourPoints2DPolarCamVelocity.cpp
  \file simulateFourPoints2DPolarCamVelocity.cpp
  \brief Visual servoing experiment on 4 points with a visualization
  from the camera and from an external view using vpSimulator.

  Visual features are the polar coordinates \f$(\rho,\theta)\f$
  of the four points.
*/

#include <visp/vpConfig.h>
#include <visp/vpDebug.h>


#ifdef VISP_HAVE_COIN

#include <visp/vpImage.h>
#include <visp/vpCameraParameters.h>
#include <visp/vpTime.h>
#include <visp/vpSimulator.h>


#include <visp/vpMath.h>
#include <visp/vpHomogeneousMatrix.h>
#include <visp/vpFeaturePointPolar.h>
#include <visp/vpServo.h>
#include <visp/vpRobotCamera.h>
#include <visp/vpFeatureBuilder.h>
#include <visp/vpParseArgv.h>
#include <visp/vpIoTools.h>

#define GETOPTARGS	"di:h"
#define SAVE 0

/*!

Print the program options.

  \param name : Program name.
  \param badparam : Bad parameter name.
  \param ipath : Input image path.

*/
void usage(const char *name, const char *badparam, std::string ipath)
{
  fprintf(stdout, "\n\
Simulation Servo 4points.\n\
\n\
SYNOPSIS\n\
  %s [-i <input image path>] [-d] [-h]\n\
", name);

  fprintf(stdout, "\n\
OPTIONS:                                               Default\n\
  -i <input image path>                                %s\n\
     Set image input path.\n\
     From this path read \"ViSP-images/iv/4points.iv\"\n\
     cad model.\n\
     Setting the VISP_INPUT_IMAGE_PATH environment\n\
     variable produces the same behaviour than using\n\
     this option.\n\
\n\
  -d                                             \n\
     Disable the image display. This can be useful \n\
     for automatic tests using crontab under Unix or \n\
     using the task manager under Windows.\n\
\n\
  -h\n\
     Print the help.\n\n",
	  ipath.c_str());

  if (badparam)
    fprintf(stdout, "\nERROR: Bad parameter [%s]\n", badparam);
}

/*!

Set the program options.

  \param argc : Command line number of parameters.
  \param argv : Array of command line parameters.
  \param ipath : Input image path.
  \param display : Set as true, activates the image display. This is
  the default configuration. When set to false, the display is
  disabled. This can be usefull for automatic tests using crontab
  under Unix or using the task manager under Windows.

  \return false if the program has to be stopped, true otherwise.

*/
bool getOptions(int argc, const char **argv, std::string &ipath, bool &display)
{
  const char *optarg;
  int	c;
  while ((c = vpParseArgv::parse(argc, argv, GETOPTARGS, &optarg)) > 1) {

    switch (c) {
    case 'i': ipath = optarg; break;
    case 'd': display = false; break;
    case 'h': usage(argv[0], NULL, ipath); return false; break;

    default:
      usage(argv[0], optarg, ipath); return false; break;
    }
  }

  if ((c == 1) || (c == -1)) {
    // standalone param or error
    usage(argv[0], NULL, ipath);
    std::cerr << "ERROR: " << std::endl;
    std::cerr << "  Bad argument " << optarg << std::endl << std::endl;
    return false;
  }

  return true;
}

static
void *mainLoop (void *_simu)
{
  vpSimulator *simu = (vpSimulator *)_simu ;
  simu->initMainApplication() ;

  while (1) {
    vpServo task ;
    vpRobotCamera robot ;

    float sampling_time = 0.040f; // Sampling period in second
    robot.setSamplingTime(sampling_time);

    std::cout << std::endl ;
    std::cout << "-------------------------------------------------------" << std::endl ;
    std::cout << " Test program for vpServo "  <<std::endl ;
    std::cout << " Eye-in-hand task control,  articular velocity are computed" << std::endl ;
    std::cout << " Simulation " << std::endl ;
    std::cout << " task : servo 4 points " << std::endl ;
    std::cout << "-------------------------------------------------------" << std::endl ;
    std::cout << std::endl ;


    vpTRACE("sets the initial camera location " ) ;
    vpPoseVector vcMo ;

    vcMo[0] = 0. ;
    vcMo[1] = 0. ;
    vcMo[2] = 3 ;
    vcMo[3] = 0 ;
    vcMo[4] = vpMath::rad(0)  ;
    vcMo[5] = vpMath::rad(90) ;

    vpHomogeneousMatrix cMo(vcMo)  ;
    robot.setPosition(cMo) ;
    simu->setCameraPosition(cMo) ;

    simu->getCameraPosition(cMo) ;
    robot.setPosition(cMo) ;

    vpCameraParameters cam ;

    vpTRACE("sets the point coordinates in the world frame "  ) ;
    vpPoint point[4] ;
    point[0].setWorldCoordinates(-0.1,-0.1,0) ;
    point[1].setWorldCoordinates(0.1,-0.1,0) ;
    point[2].setWorldCoordinates(0.1,0.1,0) ;
    point[3].setWorldCoordinates(-0.1,0.1,0) ;

    vpTRACE("project : computes  the point coordinates in the camera frame and its 2D coordinates"  ) ;
    for (int i = 0 ; i < 4 ; i++) {
      point[i].changeFrame(cMo); // Compute point coordinates in the camera frame
      point[i].project(); // Compute desired point doordinates in the camera frame
    }

    vpTRACE("sets the desired position of the point ") ;
    vpFeaturePointPolar p[4] ;
    for (int i = 0 ; i < 4 ; i++)
      vpFeatureBuilder::create(p[i], point[i])  ;  //retrieve x,y and Z of the vpPoint structure to build the polar coordinates

    std::cout << "s: \n";
    for (int i=0; i < 4; i ++) {
      printf("[%d] rho %f theta %f Z %f\n",
	     i, p[i].get_rho(), p[i].get_theta(), p[i].get_Z()); 
    }

    vpTRACE("sets the desired position of the point ") ;
    vcMo[0] = 0 ;
    vcMo[1] = 0 ;
    vcMo[2] = 1 ;
    vcMo[3] = vpMath::rad(0);
    vcMo[4] = vpMath::rad(0);
    vcMo[5] = vpMath::rad(0);

    vpHomogeneousMatrix cMod(vcMo);

    vpFeaturePointPolar pd[4] ;
    vpPoint pointd[4]; // Desired position of the points
    pointd[0].setWorldCoordinates(-0.1,-0.1,0) ;
    pointd[1].setWorldCoordinates(0.1,-0.1,0) ;
    pointd[2].setWorldCoordinates(0.1,0.1,0) ;
    pointd[3].setWorldCoordinates(-0.1,0.1,0) ;
    for (int i=0; i < 4; i ++) {
      pointd[i].changeFrame(cMod); // Compute desired point doordinates in the camera frame
      pointd[i].project(); // Compute desired point doordinates in the camera frame

      vpFeatureBuilder::create(pd[i], pointd[i])  ;  //retrieve x,y and Z of the vpPoint structure to build the polar coordinates
    }
    std::cout << "s*: \n";
    for (int i=0; i < 4; i ++) {
      printf("[%d] rho %f theta %f Z %f\n",
	     i, pd[i].get_rho(), pd[i].get_theta(), pd[i].get_Z()); 
    }
    
    vpTRACE("define the task") ;
    vpTRACE("\t we want an eye-in-hand control law") ;
    vpTRACE("\t articular velocity are computed") ;
    task.setServo(vpServo::EYEINHAND_L_cVe_eJe) ;
    task.setInteractionMatrixType(vpServo::CURRENT) ;

    vpTRACE("Set the position of the camera in the end-effector frame ") ;
    vpHomogeneousMatrix cMe ;
    vpTwistMatrix cVe(cMe) ;
    task.set_cVe(cVe) ;

    vpTRACE("Set the Jacobian (expressed in the end-effector frame)") ;
    vpMatrix eJe ;
    robot.get_eJe(eJe) ;
    task.set_eJe(eJe) ;

    vpTRACE("\t we want to see a point on a point..") ;
    for (int i = 0 ; i < 4 ; i++)
      task.addFeature(p[i],pd[i]) ;

    vpTRACE("\t set the gain") ;
    task.setLambda(1.0) ;


    vpTRACE("Display task information " ) ;
    task.print() ;

    vpTime::wait(1000); // Sleep 1s
    std::cout << "\nEnter a character to continue or CTRL-C to quit... "
	      << std::endl ;
    {    char a ; std::cin >> a ; }


    char name[FILENAME_MAX];
    int iter=0 ;
    vpTRACE("\t loop") ;
    while(iter++ < 300) {
      double t = vpTime::measureTimeMs();

      vpColVector v ;

      robot.get_eJe(eJe) ;
      task.set_eJe(eJe) ;

      robot.getPosition(cMo) ;
      for (int i = 0 ; i < 4 ; i++)
	{
	  point[i].track(cMo) ;
	  vpFeatureBuilder::create(p[i],point[i])  ;
	}

      v = task.computeControlLaw() ;
      robot.setVelocity(vpRobot::CAMERA_FRAME, v) ;

      //vpTime::wait(100) ;


      simu->setCameraPosition(cMo) ;


      if(SAVE==1)
	    {
	      sprintf(name,"/tmp/image.%04d.external.png",iter) ;
	      std::cout << name << std::endl ;
	      simu->write(name) ;
	      sprintf(name,"/tmp/image.%04d.internal.png",iter) ;
	      simu->write(name) ;
	    }

      vpTime::wait(t, sampling_time * 1000); // Wait 40 ms

    }
    vpTRACE("Display task information " ) ;
    task.print() ;
    task.kill() ;

    std::cout << "cMo:\n" << cMo << std::endl;
    vpPoseVector pose(cMo);
    std::cout << "final pose:\n" << pose.t() << std::endl;
    
    std::cout << "\nEnter a character to continue..." <<std::endl ;
    {    char a ; std::cin >> a ; }
  }

  simu->closeMainApplication() ;


  void *a=NULL ;
  return a ;
  // return (void *);
}


int
main(int argc, const char ** argv)
{
  std::string env_ipath;
  std::string opt_ipath;
  std::string ipath;
  std::string filename;
  std::string username;
  bool opt_display = true;

  // Get the VISP_IMAGE_PATH environment variable value
  char *ptenv = getenv("VISP_INPUT_IMAGE_PATH");
  if (ptenv != NULL)
    env_ipath = ptenv;

  // Set the default input path
  if (! env_ipath.empty())
    ipath = env_ipath;

  // Read the command line options
  if (getOptions(argc, argv, opt_ipath, opt_display) == false) {
    exit (-1);
  }

  // Get the option values
  if (!opt_ipath.empty())
    ipath = opt_ipath;

  // Compare ipath and env_ipath. If they differ, we take into account
  // the input path comming from the command line option
  if (!opt_ipath.empty() && !env_ipath.empty()) {
    if (ipath != env_ipath) {
      std::cout << std::endl
	   << "WARNING: " << std::endl;
      std::cout << "  Since -i <visp image path=" << ipath << "> "
	   << "  is different from VISP_IMAGE_PATH=" << env_ipath << std::endl
	   << "  we skip the environment variable." << std::endl;
    }
  }

  // Test if an input path is set
  if (opt_ipath.empty() && env_ipath.empty()){
    usage(argv[0], NULL, ipath);
    std::cerr << std::endl
	 << "ERROR:" << std::endl;
    std::cerr << "  Use -i <visp image path> option or set VISP_INPUT_IMAGE_PATH "
	 << std::endl
	 << "  environment variable to specify the location of the " << std::endl
	 << "  image path where test images are located." << std::endl << std::endl;
    exit(-1);
  }

  vpCameraParameters cam ;
  vpHomogeneousMatrix fMo ; fMo[2][3] = 0 ;


  if (opt_display) {
    vpSimulator simu ;
    simu.initInternalViewer(300, 300) ;
    simu.initExternalViewer(300, 300) ;

    vpTime::wait(1000) ;
    simu.setZoomFactor(0.2f) ;

    // Load the cad model
    filename = ipath +  vpIoTools::path("/ViSP-images/iv/4points.iv");
    simu.load(filename.c_str()) ;

    simu.setInternalCameraParameters(cam) ;
    simu.setExternalCameraParameters(cam) ;
    simu.initApplication(&mainLoop) ;

    simu.mainLoop() ;
  }
}

#else
int
main()
{  vpTRACE("You should install Coin3D and SoQT or SoWin or SoXt") ;

}
#endif
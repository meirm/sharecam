#include <boost/program_options.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<iostream>
#include <chrono>
#include <thread>

#define NOBUG 1
/*
 * * from http://opencvkazukima.codeplex.com/SourceControl/changeset/view/19055#interprocess/src/client.cpp
 * 
 * g++ -Wall  -c  frameCapture.cpp
 * g++ -Wall -o "frameCapture" frameCapture.o -I /usr/include/boost  `pkg-config --libs --cflags opencv` -std=gnu++11 -lrt -lboost_program_options  -pthread
 *
 * g++ -Wall  -c  frameCapture.cpp  -std=gnu++11
 * g++ -o  frameCapture frameCapture.o -I /usr/local/opencv-2.4.13/include -O2 -g -Wall -L /usr/local/opencv-2.4.13/lib -lopencv_core  -lopencv_imgcodecs -lopencv_videoio -lopencv_highgui -lopencv_video  -lopencv_photo  -std=gnu++11  -lboost_program_options -std=gnu++11 -lrt -pthread
 *
 * LD_LIBRARY_PATH=/usr/local/opencv-2.4.13/lib/ ./frameCapture --video 0 --xres 1920 --yres 1080
 */
using namespace cv;
using namespace std;
namespace po = boost::program_options;
int videoport =0;
int x=800;
int y=600;
int frames=2;
String output;
int verbose=0;
void init_prog(int ac, char * av[] ){
try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("video", po::value<int>(), "set video device <N>")
            ("xres", po::value<int>(), "x resolution")
            ("yres", po::value<int>(), "y resolution")
            ("frames", po::value<int>(), "frames")
            ("verbose", po::value<int>(), "verbose")
            ("output", po::value<string>(), "outputFile")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            exit(0);
	}
       
        if (vm.count("verbose")) {
            verbose=1;
	}
       
        if (vm.count("video")) {
            if (verbose) cout << "INFO: video set to "
                 << vm["video"].as<int>() << ".\n";
		videoport=vm["video"].as<int>();
        } else {
           if (verbose) cout << "INFO: video was not set. Using default\n";
        }
       
	if (vm.count("xres")) {
           if (verbose) cout << "INFO: xres set to "
                 << vm["xres"].as<int>() << ".\n";
		x=vm["xres"].as<int>();
        } else {
           if (verbose) cout << "INFO: xres was not set. Using default\n";
        }
        if (vm.count("yres")) {
          if (verbose)  cout << "INFO: yres set to "
                 << vm["yres"].as<int>() << ".\n";
		y=vm["yres"].as<int>();
        } else {
           if (verbose) cout << "INFO: yres was not set. Using default\n";
	}
        
        if (vm.count("frames")) {
           if (verbose) cout << "INFO: frames set to "
                 << vm["frames"].as<int>() << ".\n";
		frames=vm["frames"].as<int>();
        } else {
           if (verbose) cout << "INFO: frames was not set. Using default\n";
	}

        if (vm.count("output")) {
           if (verbose) cout << "INFO: output file set to "
                 << vm["output"].as<string>() << ".\n";
		output=vm["output"].as<string>();
        } else {
           if (verbose) cout << "INFO: output file was not set. Using default(/tmp/out.png)\n";
        }
    }
    catch(exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        exit(1);
    }
    catch(...) {
        cerr << "ERROR: Exception of unknown type!\n";
    }
}


int main(int argc, char* argv[])
{
  output="/tmp/out.png";
  int errcode=0;
  init_prog(argc, argv);
  
  // Open capture device
  cv::VideoCapture capture_device(videoport);
  if (!capture_device.isOpened()) {
      cerr << "ERROR: cannot open camera." << endl;
      exit(1);
  }
#if NOBUG
  capture_device.set(3,x);
  capture_device.set(4,y);
  if (x != int(capture_device.get(3)) || y != int(capture_device.get(4)) ){
      cerr << "ERROR: cannot change resolution." << endl;
      exit(2);
  }
#endif
  if (verbose) cerr << "INFO: Ready to capture first frame." << endl;
  // and capture one frame.
  cv::Mat  captured_image;
    //capture_device.grab();
    //capture_device.retrieve(captured_image) ;
    for (int i=0; i < frames; i++){
	    capture_device >> captured_image;
	    if(captured_image.empty()){
		errcode =3;
		exit(errcode);
	    }else if (captured_image.rows != y || captured_image.cols != x){
		errcode =4;
		exit(errcode);
	    }
    }
	cv::imwrite(output,captured_image);

     if (verbose) cerr << "INFO: Captured first frame." << endl;
  return 0;
}

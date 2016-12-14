#include <boost/program_options.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <chrono>
#include <thread>
/*
 * following forum post http://answers.ros.org/question/53564/zero-copy-passing-of-opencv-datatypes-with-nodelets/
 * from http://opencvkazukima.codeplex.com/SourceControl/changeset/view/19055#interprocess/src/client.cpp
 * 
 * g++ -Wall  -c  cli4.cpp
 * g++ -Wall -o "cli4" cli4.o -I /usr/include/boost  `pkg-config --libs --cflags opencv` -std=gnu++11 -lrt -pthread
 *
 * g++ -Wall  -c  cli4.cpp
 * g++ -o  cli4  -I /usr/local/opencv-2.4.13/include -O2 -g -Wall cli4.cpp -L /usr/local/opencv-2.4.13/lib -lopencv_core  -lopencv_imgcodecs -lopencv_videoio -lopencv_highgui -lopencv_video  -lopencv_photo -lopencv_imgproc -std=gnu++11  -lboost_program_options -std=gnu++11 -lrt -pthread
 *
 * LD_LIBRARY_PATH=/usr/local/opencv-2.4.13/lib/ ./cli4 
 */
#include "common.h"
using namespace std;
namespace po = boost::program_options;
float fxfy = 1.0;

void init_prog(int ac, char * av[] ){
try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("resize", po::value<float>(), "set resize <N>")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            exit(0);
	}
        if (vm.count("resize")) {
            cout << "window size set to "
                 << vm["resize"].as<float>() << ".\n";
		fxfy=vm["resize"].as<float>();
        } else {
            cout << "resize was not set. Using default\n";
        }
    }
    catch(exception& e) {
        cerr << "error: " << e.what() << "\n";
        exit(1);
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }
}

int main(int argc, char* argv[])
{
  init_prog(argc, argv);
  namespace ip  = boost::interprocess;

  // Open Shared Memory Manager
  ip::managed_shared_memory msm(ip::open_or_create, MEMORY_NAME,10000 * 1024);

  // Open Shared Mat Header
  SharedImageHeader* shared_image_header
    = msm.find<SharedImageHeader>("MatHeader").first;

  // Make shared and read only (const) cv::Mat 
  // Header information (size and type) is also provided from Shared Memory  
  cv::Mat shared;
   shared=cv::Mat(
    shared_image_header->size,
    shared_image_header->type,
   msm.get_address_from_handle(shared_image_header->handle));

   char ch;
   cv::Mat LocalFrame;
   //shared.copyTo(LocalFrame);
   LocalFrame.create(480,640,3);
   int automode= 0;
   unsigned long nrto=0; // number of timeouts.
  while (true) {
	ch= (cv::waitKey(10) &  0xFF )  ;
  	if (ch == ' ' || automode == 1 ) {
		shared_image_header->buffPosition=0;
		int timeout=100;
		while(shared_image_header->buffPosition == 0 ){
			this_thread::sleep_for(chrono::milliseconds(1));
			timeout--;
			if (timeout == 0) {
				nrto++;
				break;
			}
		}
		if (timeout > 0 ) shared.copyTo(LocalFrame);
	}

  	if (ch == 'a' ){
		automode = 1 - automode;
	}
	else if (ch == 'q' || ch == 27 ){
		shared_image_header->buffPosition = 2;
	       	break;
	}
	//if (ch < 255 ) cout << "c=" << int(ch) << endl;
    // Spinning until shared image buffPosition is updated
    // Update Version and show image.
    if (fxfy != 1.0){
	    cv::Mat ReducedFrame;
	    cv::resize(LocalFrame,ReducedFrame,cv::Size(),fxfy,fxfy);
	    cv::imshow("Client Window", ReducedFrame);
    }else{
	    cv::imshow("Client Window", LocalFrame);
    }
  }
  
  cerr << "Number of timeouts: " << nrto << endl;
  return 0;

}


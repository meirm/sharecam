#include <boost/program_options.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include<iostream>

#include "common.h"
#define NOBUG 1
/*
 * * from http://opencvkazukima.codeplex.com/SourceControl/changeset/view/19055#interprocess/src/client.cpp
 * 
 * g++ -Wall  -c  srv3.cpp
 * g++ -Wall -o "srv3" srv3.o -I /usr/include/boost  `pkg-config --libs --cflags opencv` -std=gnu++11 -lrt -lboost_program_options  -pthread
 *
 * g++ -Wall  -c  srv3.cpp
 * g++ -o  srv3  -I /usr/local/opencv-2.4.13/include -O2 -g -Wall srv3.cpp -L /usr/local/opencv-2.4.13/lib -lopencv_core  -lopencv_imgcodecs -lopencv_videoio -lopencv_highgui -lopencv_video  -lopencv_photo  -std=gnu++11  -lboost_program_options -std=gnu++11 -lrt -pthread
 *
 * LD_LIBRARY_PATH=/usr/local/opencv-2.4.13/lib/ ./srv3 --video 0 --xres 1920 --yres 1080
 */
using namespace cv;
using namespace std;
namespace po = boost::program_options;
int videoport =0;
int x=800;
int y=600;

void init_prog(int ac, char * av[] ){
try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("video", po::value<int>(), "set video device <N>")
            ("xres", po::value<int>(), "x resolution")
            ("yres", po::value<int>(), "y resolution")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            exit(0);
	}
        if (vm.count("video")) {
            cout << "video set to "
                 << vm["video"].as<int>() << ".\n";
		videoport=vm["video"].as<int>();
        } else {
            cout << "video was not set. Using default\n";
        }
       
	if (vm.count("xres")) {
            cout << "xres set to "
                 << vm["xres"].as<int>() << ".\n";
		x=vm["xres"].as<int>();
        } else {
            cout << "xres was not set. Using default\n";
        }
        
        if (vm.count("yres")) {
            cout << "yres set to "
                 << vm["yres"].as<int>() << ".\n";
		y=vm["yres"].as<int>();
        } else {
            cout << "yres was not set. Using default\n";
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

  // remove existed shared memory object
  ip::shared_memory_object::remove(MEMORY_NAME);
  
  // Open capture device
  cv::VideoCapture capture_device(videoport);
  if (!capture_device.isOpened()) {
      cerr << "cannot open camera" << endl;
      exit(1);
  }
#if NOBUG
  capture_device.set(3,x);
  capture_device.set(4,y);
  if (x != int(capture_device.get(3)) || y != int(capture_device.get(4)) ){
      cerr << "cannot change resolution" << endl;
      exit(2);
  }
#endif
  // and capture one frame.
  cv::Mat          captured_image;{
    capture_device >> captured_image;
  }


  // calculate total data size of matrixes data.
  // data_size = cols * rows * sizeof(a pixel)
  const int data_size
    = captured_image.total() * captured_image.elemSize();

  // Reserve shared memory
  ip::managed_shared_memory msm(ip::open_or_create, MEMORY_NAME,
    3 * data_size + sizeof(SharedImageHeader) + 1024 /* is it enough? */);


  // make a region named "Matheader"
  // and return its pointer
  // it's size is sizeof(SharedImageHeader)
  SharedImageHeader* shared_image_header
    = msm.find_or_construct<SharedImageHeader>("MatHeader")();

  // make a unnamed shared memory region.
  // Its size is data_size
  const SharedImageHeader* shared_image_data_ptr[2];
   shared_image_data_ptr[0]= (SharedImageHeader*)  msm.allocate(data_size);
   shared_image_data_ptr[1]= (SharedImageHeader*)  msm.allocate(data_size);


  // write the size, type and image buffPosition to the Shared Memory.
  shared_image_header->size    = captured_image.size();
  shared_image_header->type    = captured_image.type();
  shared_image_header->buffPosition = 0;

  // write the handler to an unnamed region to the Shared Memory
  shared_image_header->handle[0]
    =  msm.get_handle_from_address(shared_image_data_ptr[0]);
shared_image_header->handle[1]
    =  msm.get_handle_from_address(shared_image_data_ptr[1]);

   cv::Mat shared[2];
   shared[0]=cv::Mat(
    shared_image_header->size,
    shared_image_header->type,
   msm.get_address_from_handle(shared_image_header->handle[0]));
    shared[1]=cv::Mat(
    shared_image_header->size,
    shared_image_header->type,
   msm.get_address_from_handle(shared_image_header->handle[1]));
    
  int flag = 1;
  // Spinning until 'q' key is down
  while (true) {
    // Capture the image and show
    //capture_device.open(1);
    capture_device >> shared[ 1 - flag] ;
    //shared[ 1 - flag].release();
    
    

    // Increment the buffPosition
    shared_image_header->buffPosition= 1 - flag;
    flag = 1 - flag;
  }

  // Remove Shared Memory Object
  ip::shared_memory_object::remove(MEMORY_NAME);

  return 0;
}

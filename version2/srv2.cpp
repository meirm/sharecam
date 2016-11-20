#include <boost/interprocess/managed_shared_memory.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "common.h"
/*
 * * from http://opencvkazukima.codeplex.com/SourceControl/changeset/view/19055#interprocess/src/client.cpp
 * 
 * g++ -Wall  -c  srv.cpp
 * g++ -Wall -o "srv2" srv2.o -I /usr/include/boost  `pkg-config --libs --cflags opencv` -std=gnu++11 -lrt -pthread
 */
using namespace cv;
int main(int argc, char* argv[])
{
  namespace ip  = boost::interprocess;

  // remove existed shared memory object
  ip::shared_memory_object::remove(MEMORY_NAME);
  
  // Open capture device
  cv::VideoCapture capture_device(0);

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


  // write the size, type and image version to the Shared Memory.
  shared_image_header->size    = captured_image.size();
  shared_image_header->type    = captured_image.type();
  shared_image_header->version = 0;

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
  while (cv::waitKey(10) <1) {
    // Capture the image and show
    capture_device >> shared[ 1 - flag] ;
    cv::imshow("Server window", shared[1-flag]);

    // Increment the version
    shared_image_header->version= 1 - flag;
    flag = 1 - flag;
  }

  // Remove Shared Memory Object
  ip::shared_memory_object::remove(MEMORY_NAME);

  return 0;
}

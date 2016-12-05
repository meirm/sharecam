#include <boost/interprocess/managed_shared_memory.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
/*
 * following forum post http://answers.ros.org/question/53564/zero-copy-passing-of-opencv-datatypes-with-nodelets/
 * from http://opencvkazukima.codeplex.com/SourceControl/changeset/view/19055#interprocess/src/client.cpp
 * 
 * g++ -Wall  -c  cli3.cpp
 * g++ -Wall -o "cli3" cli3.o -I /usr/include/boost  `pkg-config --libs --cflags opencv` -std=gnu++11 -lrt -pthread
 */
#include "common.h"

int main(int argc, char* argv[])
{
  namespace ip  = boost::interprocess;

  // Open Shared Memory Manager
  ip::managed_shared_memory msm(ip::open_only, MEMORY_NAME);

  // Open Shared Mat Header
  const SharedImageHeader* shared_image_header
    = msm.find<SharedImageHeader>("MatHeader").first;

  // Make shared and read only (const) cv::Mat 
  // Header information (size and type) is also provided from Shared Memory  
  cv::Mat shared[2];
   shared[0]=cv::Mat(
    shared_image_header->size,
    shared_image_header->type,
   msm.get_address_from_handle(shared_image_header->handle[0]));
    shared[1]=cv::Mat(
    shared_image_header->size,
    shared_image_header->type,
   msm.get_address_from_handle(shared_image_header->handle[1]));
  // final_buffPosition of shared matrix
  int final_buffPosition = 0;

  while (cv::waitKey(10) < 1) {
    // Spinning until shared image buffPosition is updated
    if (final_buffPosition != shared_image_header->buffPosition){
    // Update Version and show image.
    final_buffPosition = shared_image_header->buffPosition;
    cv::imshow("Client Window", shared[final_buffPosition]);
	}
  }

  return 0;
}

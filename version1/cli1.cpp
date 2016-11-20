#include <boost/interprocess/managed_shared_memory.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
/*
 * following forum post http://answers.ros.org/question/53564/zero-copy-passing-of-opencv-datatypes-with-nodelets/
 * from http://opencvkazukima.codeplex.com/SourceControl/changeset/view/19055#interprocess/src/client.cpp
 * 
 * g++ -Wall  -c  cli.cpp
 * g++ -Wall -o "cli1" cli1.o -I /usr/include/boost  `pkg-config --libs --cflags opencv` -std=gnu++11 -lrt -pthread
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
  const cv::Mat shared(
    shared_image_header->size,
    shared_image_header->type,
    msm.get_address_from_handle(shared_image_header->handle));
    
  // final_version of shared matrix
  int final_version = 0;

  while (cv::waitKey(10) < 1) {
    // Spinning until shared image version is updated
    if (final_version != shared_image_header->version){
    // Update Version and show image.
    final_version = shared_image_header->version;
    cv::imshow("Client Window", shared);
	}
  }

  return 0;
}

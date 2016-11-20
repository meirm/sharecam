#ifndef INTERPROCESS_COMMON_H
#define INTERPROCESS_COMMON_H 1

#include <opencv2/core/core.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
/*
 * from http://opencvkazukima.codeplex.com/SourceControl/changeset/view/19055#interprocess/src/client.cpp
 *  
 */
typedef struct{
  cv::Size  size;
  int       type;
  int       version;
  boost::interprocess::managed_shared_memory::handle_t handle[2];
} SharedImageHeader;

const char *MEMORY_NAME = "MySharedMemory";

#endif // #ifndef INTERPROCESS_COMMON_H

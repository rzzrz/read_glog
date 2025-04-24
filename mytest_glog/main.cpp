#include <glog/logging.h>
#include <chrono>
#include <thread>
#include <iostream>
int main(int argc, char* argv[]){
  google::InitGoogleLogging(argv[0]);
  for (int i = 0; i < 2048; i++) {
    LOG_EVERY_T(INFO, 0.01) << "获得一个cookie"; // 每10ms
    LOG_EVERY_T(INFO, 2.35) << "获得一个cookie"; // 每2.35秒
  }
  while (1) {
    std::cout<<"this thread runing"<<std::endl;
  }
  return 0;
}

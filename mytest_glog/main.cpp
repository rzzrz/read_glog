#include <glog/logging.h>

int main(int argc, char* argv[]){
  google::InitGoogleLogging(argv[0]);
  for (int i = 0; i < 30; i++) {
    LOG_EVERY_N(INFO, 10) << "会在执行第1, 11, 21次log";
  }
    
return 0;
}

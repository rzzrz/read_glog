#include <glog/logging.h>

int main(int argc, char* argv[]){
  google::InitGoogleLogging(argv[0]);

  CHECK_EQ(1, 1) << "不好1等于1啦!";
  
  return 0;
}

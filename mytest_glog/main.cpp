#include <glog/logging.h>
int main(int argc, char* argv[]){
  google::InitGoogleLogging(argv[0]);

  LOG(INFO) << "这是一条测试glog日志流的日志";
  return 0;
}

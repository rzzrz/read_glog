#include <glog/logging.h>
#include <iostream>
int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    int num_cookies = 1;
    LOG(INFO) << "Found " << num_cookies << " cookies"; // (2)!
    std::cout<<"glog success"<<std::endl;
    return 0;
}
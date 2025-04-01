#include <glog/logging.h>
#include <iostream>
#include <ostream>

// void myformatter(std::basic_ostream<char>& ostream ,const google::LogMessage& messageinfo,void* ptr)
// {
//     std::cout<<"this is my formatter"<<std::endl;
// }

int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    int num_cookies = 1;
    
    //google::InstallPrefixFormatter(myformatter);
    //LOG_IF(INFO, num_cookies > 10) << "Got lots of cookies";
    LOG_EVERY_N(INFO, 10) << "Got the " << google::COUNTER << "th cookie";
    std::cout<<"glog success"<<std::endl;
    
    return 0;
}

#include<iostream>

#define MY_MACRO(condition,message) \
        !condition                  \
        ?void(0)                    \
        :(std::cout<<message<<std::endl,void(0))


int main(){
    MY_MACRO(0, "this macro worked");
    return 0;
}
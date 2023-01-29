#include <iostream>
#include <gtest/gtest.h>
#include "cansocket/cansocket.hpp"
#include <vector>
#include <string>
TEST(Socket, WriteFunctionTest){
  std::vector<unsigned int> canIds = {0x01,0x02};
  try{
    auto canSocket = cansocket::CanSocket(std::string("vcan0"),canIds,cansocket::CanSocket::eSocketType::READ_WRITE);
    auto newMsg = cansocket::CanFDMsg();
    newMsg.id = 0xF1;
    newMsg.data.push_back(0x01);
    newMsg.data.push_back(0xFF);
    canSocket.write(newMsg,false);
    SUCCEED();
  }catch(const std::exception& e)
  {
    std::cerr<<e.what()<<std::endl;
    FAIL();
  }
}
TEST(Socket, ReadFunctionTest){
  std::vector<unsigned int> canIds = {0x01,0x02};
  try{
    auto canSocket = cansocket::CanSocket(std::string("vcan0"),canIds,cansocket::CanSocket::eSocketType::READ_WRITE);
    for(int i=0;i<10;i++){
      std::cout<<"Message no "<<i<<":"<<"\r\n";
      std::cout<<canSocket.read()<<std::endl;
    }
    SUCCEED();
  }catch(const std::exception& e)
  {
    std::cerr<<e.what()<<std::endl;
    FAIL();
  }
}
int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
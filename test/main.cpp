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
int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
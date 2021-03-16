#include "MyUDPManager.h"
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include "src/logging.h"
MyUDPManager::MyUDPManager() {
    LOG(INFO) << "MyUDPManager(), jotaro:UDP Socket Opening";
    r = std::thread([&] { client.Receiver(); });
    LOG(INFO) << "thread started";
    
    //std::thread r = std::thread([&] { client.Receiver(); });
    
}

MyUDPManager::~MyUDPManager()
{
    r.join();
}

void MyUDPManager::init()
{
}

std::string MyUDPManager::getLastMessage()
{
    if (client.messageAvail) {
        client.messageAvail = false;
        return client.lastMessage;
    }
    else {
        return "";
    }
}

void MyUDPManager::Dummy() {
}

void MyUDPManager::OpenThread() {
}

void MyUDPManager::Receiver(int a)
{
}
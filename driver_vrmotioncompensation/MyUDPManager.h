#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>
#include "./src/logging.h"
#define IPADDRESS "127.0.0.1" // "192.168.1.64"
#define UDP_PORT 13251
using boost::asio::ip::udp;
using boost::asio::ip::address;



struct MyUDPClient {
    boost::asio::io_service io_service;
    udp::socket socket{ io_service };
    boost::array<char, 1024> recv_buffer;
    udp::endpoint remote_endpoint;
    std::string lastMessage = "";
    bool messageAvail = false;

    int count = 3;

    void handle_receive(const boost::system::error_code& error, size_t bytes_transferred) {
        if (error) {
            LOG(INFO) << "Receive failed: " << error.message() << "\n";
            return;
        }
        LOG(INFO) << "Received: '" << std::string(recv_buffer.begin(), recv_buffer.begin() + bytes_transferred) << "' (" << error.message() << ")\n";
        lastMessage = std::string(recv_buffer.begin(), recv_buffer.begin() + bytes_transferred);
        messageAvail = true;

        if (count > 0) {
            LOG(INFO) << "Count: " << count << "\n";
            wait();
        }
    }

    void wait() {
        socket.async_receive_from(boost::asio::buffer(recv_buffer),
            remote_endpoint,
            boost::bind(&MyUDPClient::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void Receiver()
    {
        socket.open(udp::v4());
        socket.bind(udp::endpoint(address::from_string(IPADDRESS), UDP_PORT));

        wait();

        LOG(INFO) << "Receiving\n";
        io_service.run();
        LOG(INFO) << "Receiver exit\n";
    }
};


class MyUDPManager
{
public:
    MyUDPClient client;
	MyUDPManager();
	~MyUDPManager();
    std::thread r;
	void init();
	std::string getLastMessage();
    void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);
    void wait();
    void Dummy();
    void OpenThread();
    void Receiver(int a);
	std::string lastReceivedMessage="";
    bool messageAvailable = false;
private:
	int sock = 0;
};



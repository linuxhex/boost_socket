/* Copyright(C) Gaussian Automation. All rights reserved.
 */

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <ros/ros.h>
#include <string>
#include <boost/exception/all.hpp>
#include "SPort.h"
#include "Comm.h"
#include "parameter.h"

Socket::Socket() {
  socket_ = NULL;
  thread_ = NULL;

  m_lReadBuffer.Init(1024);
  memset(m_szWriteBuffer, 0, 1024);
  m_nWriteBufferSize = 0;

  host_name_ = "192.168.1.199";
  port_ = "5000";
}

Socket::~Socket() {
  if (socket_ != NULL) {
    delete socket_;
    socket_ = NULL;
  }
  if(thread_ != NULL){
      delete thread_;
      thread_ = NULL;
  }
}

bool Socket::open() {

   if (socket_ != NULL) {
      delete socket_;
      socket_ = NULL;
   }
   try {
      socket_ = new boost::asio::ip::tcp::socket(ios_);
      boost::asio::ip::tcp::resolver resolver(ios_);
      boost::asio::connect(*socket_, resolver.resolve({host_name_.c_str(), port_.c_str()}));
      ROS_INFO("[SOCKET] open socket successfully");
      return true;
  } catch (std::exception &e) {
    ROS_ERROR("[SOCKET] open socket failed");
    if (socket_ != NULL) {
       delete socket_;
       socket_ = NULL;
    }
    return false;
  }
}

void Socket::Send_data(unsigned char* s_data, unsigned short len) {
  m_lReadBuffer.Clear();
  memset(m_szWriteBuffer, 0, 1024);
  m_nWriteBufferSize = len;
  memcpy(m_szWriteBuffer, s_data, len);
  write();
}

void Socket::read_callback(const boost::system::error_code& error, std::size_t bytes_transferred) {
  if (error) {  // No data was read!
    ROS_ERROR("[SOCKET] read data error");
    return;
  }
  m_lReadBuffer.Write(m_szReadTemp, bytes_transferred);
  read();
}

void Socket::Read_data(unsigned char* r_data, int &len, int need, int timeout) {
  len = 0;
  int len_tmp = 0;
  connection_status = 0;

  while (1) {
    len_tmp = m_lReadBuffer.Size();
    if (len_tmp >= need){
      connection_status = 1;
      break;
    }
    if (timeout--) {
      SLEEP(1);
    } else {
      ROS_ERROR("[SOCKET] time out");
      break;
    }
  }
  m_lReadBuffer.Read(r_data, len);
}

int Socket::ThreadRun() {
  try {
    while (1) {
      if (open()) {
        read();
        ios_.run();
        ios_.reset();
      }
      boost::this_thread::interruption_point();
      SLEEP(1000);
    }
    return 0;
  } catch(boost::thread_interrupted) {
    return -1;
  } 
}

bool Socket::BeginThread() {
  EndThread();
  thread_ = new boost::thread(boost::bind(&Socket::ThreadRun, this));
  return thread_ != NULL;
}

void Socket::EndThread() {
  if (thread_ != NULL) {
    ios_.stop();
    thread_->interrupt();
    thread_->join();
    delete thread_;
    thread_ = NULL;
  }
}

void Socket::read() {
  socket_->async_read_some(boost::asio::buffer(m_szReadTemp, 1024),
                            boost::bind(&Socket::read_callback, this,
                                        boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

}

void Socket::write() {
    if (socket_) {
        try{
            socket_->write_some(boost::asio::buffer(m_szWriteBuffer, m_nWriteBufferSize));
         }catch(boost::exception &e){
            cout << diagnostic_information(e)<<endl;
         }
    }
}

void Socket::Init(const std::string& host_name, const std::string& port) {
  port_ = port;
  host_name_ = host_name;
  BeginThread();

  while (!socket_) {
    ROS_INFO("[SOCKET] waitting for socket %s:%s", host_name.c_str(), port.c_str());
    SLEEP(1000);
  }
}

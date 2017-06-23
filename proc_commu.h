/*
 * proc_commu.h
 *
 *  Created on: 2017年6月16日
 *      Author: zjbpoping
 */
#ifndef PROC_COMMU_H_
#define PROC_COMMU_H_

#include "message.h"
#include "threadqueue.h"
#include "manager.h"
#include "endpoint.h"
#include <thread>
#include <functional>
#include <vector>
#include <mutex>
#include <map>
#include <iostream>

using namespace std;
 

namespace ps{
	/*无论是Worker还是Server，所有的Proc都会有一个Communication通信类*/
	class Customer{
	public:
		using RecvHandle = std::function<void(message& msg)>;
		Customer(int id,const RecvHandle& handle);
		~Customer();
		int Getid(){
			return id_;
		}
		/*将消息推入该Customer的缓存队列*/
		void PushToQueue(message& msg){
			recv_queue_.Push(msg);
		}
		/*生成tracker_项，tracker_的下标就是时间戳*/
		int NewRequest(int recver);
		/*等待，直到timestamp时间戳的请求完成，就是收到回复*/
		void WaitRequest(int timestamp);
		/*返回收到的对应请求的回复个数*/
		int NumResponse(int timestamp);
		/*为对应时间戳也就是下标的tracker_项的回复数添加num*/
		void AddResponse(int timestamp,int num=1);
	private:
		int id_;
		/*接收消息的处理handle，在接收消息的处理线程中使用*/
		RecvHandle recv_handle_;

		/*接收消息的处理线程*/
		void Receiving();
		std::unique_ptr<std::thread> recv_thread_;
		/*接收消息的缓存队列*/
		ThreadsafeQueue<message> recv_queue_;

		std::mutex tracker_mu_;
		std::condition_variable tracker_cond_;
		std::vector<std::pair<int, int>> tracker_;
	};
}

#endif
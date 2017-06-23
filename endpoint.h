/*
 * endpoint.h
 *
 *  Created on: 2017年6月2日
 *      Author: zjbpoping
 */
#ifndef ENDPOINT_H_
#define ENDPOINT_H_

#include "node.h"
#include "message.h"
#include <stdlib.h>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <ctime>
#include <unordered_map>


namespace ps{

	class Endpoint{
	public:
		Endpoint(){}
		~Endpoint(){}
		void Start();
		void Stop();
		void Send(message& msg);
		Node* Current(){
			return &current_;
		}
	private:
		void Receive(message& msg);
		void Serialize(message& msg, char** meta_buf, int* meta_size);
		void DeSerialize(message& msg, const char* meta_buf, int meta_size);
		/*连接某个节点*/
		void Connect(const Node& node);
		/*收发线程对应的function*/
		void Receiving();
		void Heartbeat();
		/*收发消息的线程*/
		std::unique_ptr<std::thread> receiver_thread_;
		std::unique_ptr<std::thread> heartbeat_thread;
		Node scheduler_;
		Node current_;
		bool is_scheduler_;
		void *context_ = nullptr;
		void *receiver_ = nullptr;
		/*存储不同连接对象的socket连接*/
		std::unordered_map<int, void*> senders_;
		std::mutex mu_;
		/*Lamport timestamp*/
		std::atomic<int> timestamp={0};
		/*存储node信息,只有Scheduler会存储*/
		std::vector<Node> nodes;
	};
}

#endif

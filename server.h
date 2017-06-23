/*
 * server.h
 *
 *  Created on: 2017年6月15日
 *      Author: zjbpoping
 */
#ifndef SERVER_H_
#define SERVER_H_

#include "proc.h"
#include "value.h"
#include <utility>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

namespace ps{

	template <typename Val>
	class Server:public Proc{
	public:
		using Proc::customer;
		explicit Server(int proc_id):Proc(){
			using namespace std::placeholders;
			customer=new Customer(proc_id,std::bind(&Server<Val>::Process,this,_1));
		}

		virtual ~Server(){
			delete customer;
			customer=nullptr;
		}
		using ReqHandle=std::function<void(KVMeta& req_meta,
			const KVPairs<Val>& req_data,
			Server* server)>;

		void set_request_handle(const ReqHandle& handle){
			request_handle_=handle;
		}
		/*对Push/Pull消息的回复，传入的是请求者的KVMeta*/
		void Response(const KVMeta& req,const KVPairs<Val>& res=KVPairs<Val>());

	private:
		/*customer中消息处理的handle*/
		void Process(message& msg);
		/*消息请求handle, 消息处理process中需要*/
		ReqHandle request_handle_;
	};

	template <typename Val>
	void Server<Val>::Response(const KVMeta& req,const KVPairs<Val>& res){
		///
		message msg;
		msg.customer_id = customer->Getid();
		msg.request 	= false;
		msg.push 		= req.push;
		//msg.cmd     	= message::Command(req.cmd);
		msg.timestamp 	= req.timestamp;
		msg.receiver	= req.sender;
		msg.sender		= Manager::Get()->GetEndpoint()->Current()->id;
		if(res.keys.size()){
			msg.AddData(res.keys);
			msg.AddData(res.vals);
			if(res.lens.size())
				msg.AddData(res.lens);
		}
		Manager::Get()->GetEndpoint()->Send(msg);
	}

	template <typename Val>
	void Server<Val>::Process(message& msg){
		KVMeta meta;
		meta.cmd		= msg.cmd;
		meta.push 		= msg.push;
		meta.sender		= msg.sender;
		meta.timestamp	= msg.timestamp;
		KVPairs<Val> data;
		int n=msg.data.size();
		if(n){
			data.keys=msg.data[0];
			data.vals=msg.data[1];
			if(n>2)
			{
				data.lens=msg.data[2];
			}
		}
		request_handle_(meta,data,this);
	}

}

#endif 
/*
 * manager.h
 *
 *  Created on: 2017年6月2日
 *      Author: zjbpoping
 */
#ifndef MANAGER_H_
#define MANAGER_H_

#include "proc_commu.h"
#include "endpoint.h"
#include "state.h"
#include "sarray.h"
#include <unordered_map>
#include <mutex>
#include <iostream>
using namespace std;

namespace ps{
	class Customer;
	/*parameter Server节点管理器，思路：节点崩溃和添加可以通过Scheduler节点通信来确定*/
	class Manager{
	public:
		/*返回一个静态的Manager对象,一个节点只有这么一个*/
		static Manager* Get(){
			static Manager manager;
			return &manager;
		}
		/*返回endpoint指针*/
		Endpoint* GetEndpoint(){
			return ep_;	
		}

		int NumWorkers(){
			return num_workers_;
		}

		int NumServers(){
			return num_servers_;
		}

		void AddWorkers();
		void AddServers();
		
		/*判断当前节点类型*/
		bool IsWorker(){return is_worker_;}
		bool IsServer(){return is_server_;}
		bool IsScheduler(){return is_scheduler_;}
		/*启动节点管理器，同时会启动负责通信的endpoint*/
		void Start();
		/*终止节点管理器*/
		void Stop();

		int GetServerID(int num){
			return num*2+8;
		}

		int GetWorkerID(int num){
			return num*2+9;
		}

		void SetWorkerGroup(int id);

		void SetServerGroup(int id);

		int NodeIDSize(){
			return node_ids_.size();
		}

		std::vector<Range>& GetRange(int keys_size_);

		Customer* GetCustomer(int id,int timeout=0) const;
		void AddCustomer(Customer* customer);
		void RemoveCustomer(Customer* customer);
		const std::vector<int>& GetNodeIDs(int node_id) const{
			const auto it=node_ids_.find(node_id);
			//it是空的话会出错，以后解决
			return it->second;
		}

	private:
		Manager();
		~Manager(){delete ep_;}
		/*该节点的通信断端点，也是一个节点只有一个*/
		Endpoint* ep_;
		int num_workers_=0,num_servers_=0;
		bool is_worker_,is_server_,is_scheduler_;
		//bool trans=true;//考虑用来作为允许传输标识符
		time_t start_time_;
		std::unordered_map<int,Customer*> customers_;
		//还需要添加node_ids的初始化过程，思路可以是每次计算好num_workers_之后调用
		std::unordered_map<int,std::vector<int>> node_ids_;
		mutable std::mutex mu_;
		std::vector<Range> key_range_;
	};


}
#endif

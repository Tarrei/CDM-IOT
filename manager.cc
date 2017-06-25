/*
 * manager.cc
 *
 *  Created on: 2017年6月2日
 *      Author: zjbpoping
 */
#include "manager.h"
#include "environment.h"
#include "state.h"
#include <unistd.h>
#include <cstdlib>
#include <thread>
#include <chrono>

namespace ps{
	/*读取环境变量初始化Manager对象*/
	Manager::Manager(){

		ep_=new Endpoint();
		//const char* temp = NULL;
		//temp=Environment::Get()->find("NUM_WORKER");
		//num_workers_=atoi(temp);
		//temp=Environment::Get()->find("NUM_SERVER");
		//num_servers_ =atoi(temp);
		std::string role=Environment::Get()->find("ROLE");
		is_worker_ = (role=="worker"||role=="WORKER");
		is_server_ = (role=="server"||role=="SERVER");
		is_scheduler_ = (role=="scheduler"||role=="SCHEDULER");

	}

	void Manager::Start(){
		ep_->Start();
		start_time_ = time(NULL);
	}

	void Manager::Stop(){
		ep_->Stop();
	}

	void Manager::AddWorkers(){
		std::lock_guard<std::mutex> lk(mu_);
		num_workers_++;
	}
	
	void Manager::AddServers(){
		std::lock_guard<std::mutex> lk(mu_);
		num_servers_++;
	}

	void Manager::SetWorkerGroup(int id){
		for (int g : {id, WorkerGroupID, WorkerGroupID + ServerGroupID,
	        WorkerGroupID + SchedulerID,
	        WorkerGroupID + ServerGroupID + SchedulerID}) {
      		node_ids_[g].push_back(id);
    	}
	}

	void Manager::SetServerGroup(int id){
		for (int g : {id, ServerGroupID, WorkerGroupID + ServerGroupID,
	        ServerGroupID + SchedulerID,
	        WorkerGroupID + ServerGroupID + SchedulerID}) {
      		node_ids_[g].push_back(id);
    	}
	}

	Customer* Manager::GetCustomer(int id,int timeout) const{
		Customer* obj = nullptr;
		//cout<<"customers: "<<customers_.size()<<endl;
		for (int i = 0; i < timeout*1000+1; ++i) {
		    std::lock_guard<std::mutex> lk(mu_);
		    const auto it = customers_.find(id);
		    if (it != customers_.end()) {
		    	obj = it->second;
		        break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		return obj;
	}

	void Manager::AddCustomer(Customer* customer){
		std::lock_guard<std::mutex> lk(mu_);
		int id=customer->Getid();
		customers_[id]=customer;
	}

	void Manager::RemoveCustomer(Customer* customer){
		std::lock_guard<std::mutex> lk(mu_);
		int id=customer->Getid();
		customers_.erase(id);
	}

	std::vector<Range>& Manager::GetRange(int keys_size_){
		std::lock_guard<std::mutex> lk(mu_);
		int size=key_range_.size();
		if(size!=num_servers_){
			key_range_.clear();
			for(int i=0;i<num_servers_;i++){
				key_range_.push_back(Range(
					keys_size_/num_servers_*i,
					keys_size_/num_servers_*(i+1)));
			}
		}
		return key_range_;
	}	
}

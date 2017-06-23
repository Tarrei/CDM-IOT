/*
 * proc_commu.cc
 *
 *  Created on: 2017年6月16日
 *      Author: zjbpoping
 */
#include "proc_commu.h"

namespace ps{
	Customer::Customer(int id, const Customer::RecvHandle& handle)
	:id_(id),recv_handle_(handle){
		Manager::Get()->AddCustomer(this);
		recv_thread_ = std::unique_ptr<std::thread>(new std::thread(&Customer::Receiving, this));
	}

	Customer::~Customer(){
		Manager::Get()->RemoveCustomer(this);
		message msg;
		/*将终止命令推入缓存队列*/
		msg.cmd = message::TERMINATE;
		recv_queue_.Push(msg);
		recv_thread_->join();
	}

	int Customer::NewRequest(int recver){
		std::lock_guard<std::mutex> lk(tracker_mu_);
		int num=Manager::Get()->GetNodeIDs(recver).size();
		tracker_.push_back(std::make_pair(num,0));
		return tracker_.size()-1;
	}

	void Customer::WaitRequest(int timestamp) {
	  std::unique_lock<std::mutex> lk(tracker_mu_);
	  tracker_cond_.wait(lk, [this, timestamp]{
	      return tracker_[timestamp].first == tracker_[timestamp].second;
	    });
	}

	int Customer::NumResponse(int timestamp) {
  		std::lock_guard<std::mutex> lk(tracker_mu_);
  		return tracker_[timestamp].second;
	}

	void Customer::AddResponse(int timestamp, int num) {
  		std::lock_guard<std::mutex> lk(tracker_mu_);
  		tracker_[timestamp].second += num;
	}

	void Customer::Receiving(){
		while(true){
			message msg;
			recv_queue_.WaitAndPop(&msg);
			if(msg.cmd==message::TERMINATE)
				break;
			recv_handle_(msg);
			if(!msg.request){
				std::lock_guard<std::mutex> lk(tracker_mu_);
      			tracker_[msg.timestamp].second++;
      			tracker_cond_.notify_all();
			}
		}
	}
}
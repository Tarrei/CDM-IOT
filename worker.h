/*
 * worker.h
 *
 *  Created on: 2017年6月15日
 *      Author: zjbpoping
 */
#ifndef WORKER_H_
#define WORKER_H_
#include "proc.h"
#include "value.h"
#include "state.h"
#include <algorithm>
#include <utility>
#include <unordered_map>
#include <iostream>
using namespace std;

namespace ps{
	template <typename Val>
	class Worker:public Proc{
	public:
		using Proc::customer;
		using Callback=std::function<void()>;
		using SlicedKVs = std::vector<std::pair<bool, KVPairs<Val>>>;
		using Slicer = std::function<void(
			const KVPairs<Val>& send, const std::vector<Range>& ranges,
			SlicedKVs* sliced)>;

		explicit Worker(int proc_id):Proc(){
			using namespace std::placeholders;
			slicer_=std::bind(&Worker<Val>::DefaultSlicer,this,_1,_2,_3);
			customer=new Customer(proc_id,std::bind(&Worker<Val>::Process,this,_1));
		}
		virtual ~Worker(){
			delete customer;
			customer=nullptr;
		}

		int Push(const std::vector<Key>& keys,
			const std::vector<Val>& vals,
			const std::vector<int>& lens ={},
			int cmd = 0,
			const Callback& cb=nullptr);

		/*两种Pull*/
		int Pull(const SArray<Key>& keys,
			SArray<Val>* vals,
			SArray<int>* lens =nullptr,
			int cmd = 0,
			const Callback& cb=nullptr){
			return Pull_(keys, vals, lens, cmd, cb);
		}
		int Pull(const std::vector<Key>& keys,
           std::vector<Val>* vals,
           std::vector<int>* lens = nullptr,
           int cmd = 0,
           const Callback& cb = nullptr){
    	   return Pull_(SArray<Key>(keys), vals, lens, cmd, cb);
  		}

		void Wait(int timestamp){
			customer->WaitRequest(timestamp);
		}

	private:
		template <typename C, typename D>
  		int Pull_(const SArray<Key>& keys, C* vals, D* lens, 
  			int cmd, const Callback& cb);
		void Send(int timestamp,bool push, int cmd,const KVPairs<Val>& kvs);
		void Process(const message& msg);
		void DefaultSlicer(const KVPairs<Val>& send,
                     const std::vector<Range>& ranges,
                     SlicedKVs* sliced);
		void RunCallback(int timestamp);
		void AddCallback(int timestamp,const Callback& cb){
			if(!cb) return;
			std::lock_guard<std::mutex> lk(mu_);
			callbacks_[timestamp]=cb;
		}
		/*每个时间戳的接收kvs的缓冲*/
		std::unordered_map<int, std::vector<KVPairs<Val>>> recv_kvs_;
		std::unordered_map<int, Callback> callbacks_;
		std::mutex mu_;
		Slicer slicer_;
	};

	template <typename Val>
	void Worker<Val>::Send(int timestamp, bool push, int cmd, const KVPairs<Val>& kvs){
		
		SlicedKVs sliced;
		int keys_size=kvs.keys.size();
		slicer_(kvs, Manager::Get()->GetRange(keys_size), &sliced);

		int skipped = 0;
		for (size_t i = 0; i < sliced.size(); ++i) {
    		if (!sliced[i].first) ++skipped;
  		}
  		customer->AddResponse(timestamp, skipped);
  		if ((size_t)skipped == sliced.size()) {
    		RunCallback(timestamp);
  		}

		for (size_t i = 0; i < sliced.size(); ++i) {
		    if (!sliced[i].first) continue;
		    message msg;
		    msg.sender 		= Manager::Get()->GetEndpoint()->Current()->id;
		    msg.receiver	= Manager::Get()->GetServerID(i);
		    //cout<<msg.receiver<<endl;
		    msg.customer_id = customer->Getid();
		    msg.request     = true;
		    msg.push        = push;
		    //msg.head        = cmd;
		    msg.timestamp   = timestamp;
		    const auto& kvs = sliced[i].second;
		    if (kvs.keys.size()) {
		      	msg.AddData(kvs.keys);
		      	msg.AddData(kvs.vals);
		      	if (kvs.lens.size()) {
		        	msg.AddData(kvs.lens);
		   		}
			}

			Manager::Get()->GetEndpoint()->Send(msg);
  		}
	}

	template <typename Val>
	void Worker<Val>::Process(const message& msg) {
	  	int ts=msg.timestamp;
	  	if(!msg.push&&msg.data.size()){
	  		KVPairs<Val> kvs;
	  		kvs.keys=msg.data[0];
	  		kvs.vals=msg.data[1];
	  		if(msg.data.size()>(size_t)2){
	  			kvs.lens=msg.data[3];
	  		}
	  		mu_.lock();
	  		recv_kvs_[ts].push_back(kvs);
	  		mu_.unlock();
	  	}
	  	// cout<<"num response: "<<customer->NumResponse(ts)<<" num servers: "<<Manager::Get()->NumServers()<<endl;
	  	if (customer->NumResponse(ts) == Manager::Get()->NumServers()-1){
		    RunCallback(ts);
		}
	}

	template <typename Val>
	int Worker<Val>::Push(const std::vector<Key>& keys,
			const std::vector<Val>& vals,
			const std::vector<int>& lens,
			int cmd,
			const Callback& cb){

		// while(true){
		// 	if(Manager::Get()->GetEndpoint()->Current()->id!=Node::EmptyID)
		// 		break;
		// }	

		int ts=customer->NewRequest(ServerGroupID);

		AddCallback(ts,cb);
		KVPairs<Val> kvs;
		kvs.keys=SArray<Key>(keys);
		kvs.vals=SArray<Val>(vals);
		kvs.lens=SArray<int>(lens); 
		Send(ts,true,cmd,kvs);
		return ts;
	}

	template <typename Val>
	template <typename C, typename D>
	int Worker<Val>::Pull_(const SArray<Key>& keys,
			C* vals, D* lens, int cmd, const Callback& cb){
		int ts=customer->NewRequest(ServerGroupID);
		AddCallback(ts, [this, ts, keys, vals, lens, cb]() mutable {
      		mu_.lock();
		    auto& kvs = recv_kvs_[ts];
		    mu_.unlock();

      		// do check
      		size_t total_key = 0, total_val = 0;
      		for (const auto& s : kvs) {
        		//Range range = FindRange(keys, s.keys.front(), s.keys.back()+1);
        		total_key += s.keys.size();
        		total_val += s.vals.size();
      		}

      		// fill vals and lens
      		std::sort(kvs.begin(), kvs.end(), [](
          		const KVPairs<Val>& a, const KVPairs<Val>& b) {
                  	return a.keys.front() < b.keys.front();
        	});

      		if (vals->empty()) {
        		vals->resize(total_val);
      		} 
      		Val* p_vals = vals->data();
      		int *p_lens = nullptr;
      		if (lens) {
        		if (lens->empty()) {
          			lens->resize(keys.size());
        		} 
        		p_lens = lens->data();
      		}
      		for (const auto& s : kvs) {
        		memcpy(p_vals, s.vals.data(), s.vals.size() * sizeof(Val));
       	 		p_vals += s.vals.size();
        		if (p_lens) {
          			memcpy(p_lens, s.lens.data(), s.lens.size() * sizeof(int));
          			p_lens += s.lens.size();
        		}
      		}

      		mu_.lock();
      		recv_kvs_.erase(ts);
      		mu_.unlock();
      		if (cb) cb();
    	});

  		KVPairs<Val> kvs; 
  		kvs.keys = keys;

  		Send(ts, false, cmd, kvs);
  		return ts;
	}

	template <typename Val>
	void Worker<Val>::RunCallback(int timestamp){
		mu_.lock();
		auto it=callbacks_.find(timestamp);
		if(it!=callbacks_.end()){
			mu_.unlock();

			it->second();

			mu_.lock();
			callbacks_.erase(it);
		}
		mu_.unlock();
	}

	template <typename Val>
	void Worker<Val>::DefaultSlicer(
	    const KVPairs<Val>& send, const std::vector<Range>& ranges,
	    typename Worker<Val>::SlicedKVs* sliced) {
	  
	  sliced->resize(ranges.size());

	  // find the positions in msg.key
	  size_t n = ranges.size();
	  std::vector<size_t> pos(n+1);
	  const Key* begin = send.keys.begin();
	  const Key* end = send.keys.end();
	  for (size_t i = 0; i < n; ++i) {
	    if (i == 0) {
	      pos[0] = std::lower_bound(begin, end, ranges[0].begin()) - begin;
	      begin += pos[0];
	    } 
	    size_t len = std::lower_bound(begin, end, ranges[i].end()) - begin;
	    begin += len;
	    pos[i+1] = pos[i] + len;
	    // don't send it to severs for empty kv
	    sliced->at(i).first = (len != 0);
	  }
	  if (send.keys.empty()) return;

	  // the length of value
	  size_t k = 0, val_begin = 0, val_end = 0;
	  if (send.lens.empty()) {
	    k = send.vals.size() / send.keys.size();
	  } 

	  // slice
	  for (size_t i = 0; i < n; ++i) {
	    if (pos[i+1] == pos[i]) {
	      sliced->at(i).first = false;
	      continue;
	    }
	    sliced->at(i).first = true;
	    auto& kv = sliced->at(i).second;
	    kv.keys = send.keys.segment(pos[i], pos[i+1]);
	    if (send.lens.size()) {
	      kv.lens = send.lens.segment(pos[i], pos[i+1]);
	      for (int l : kv.lens) val_end += l;
	      kv.vals = send.vals.segment(val_begin, val_end);
	      val_begin = val_end;
	    } else {
	      kv.vals = send.vals.segment(pos[i]*k, pos[i+1]*k);
	    }
	  }
	}
}

#endif
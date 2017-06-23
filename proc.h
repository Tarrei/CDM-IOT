/*
 * proc.h
 *
 *  Created on: 2017年6月15日
 *      Author: zjbpoping
 */
#ifndef COMPUTE_H_
#define COMPUTE_H_
#include "node.h"
#include "message.h"
#include "manager.h"
#include <string>

namespace ps{
	class Proc{
	public:
		explicit Proc(int proc_id);
		virtual ~Proc(){
			delete customer;
			customer=nullptr;
		}
		
		virtual inline void Wait(int timestamp){
			customer->WaitRequest(timestamp);
		};
		
		virtual inline Customer* get_customer(){
			return customer;
		}
	protected:
		inline Proc():customer(nullptr){};
		Customer* customer;
	};
}

#endif
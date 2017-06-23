/*
 * params.h
 *
 *  Created on: 2017年6月2日
 *      Author: zjbpoping
 */
#ifndef PARAMS_H_
#define PARAMS_H_

#include "manager.h"
#include "server.h"
#include "worker.h"

namespace ps {

	inline int NumWorkers(){return Manager::Get()->NumWorkers();}
	inline int NumServers(){return Manager::Get()->NumServers();}
	inline bool IsWorker(){return Manager::Get()->IsWorker();}
	inline bool IsServer(){return Manager::Get()->IsServer();}
	inline bool IsScheduler(){return Manager::Get()->IsScheduler();}
	inline void Start(){Manager::Get()->Start();}
	inline void Stop(){Manager::Get()->Stop();}
}

#endif

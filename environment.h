/**
 * Copyright (c) 2016 by Contributors
 */
#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_
#include <string>
#include <vector>
#include <cstdlib>
#include <unordered_map>
#include <memory>

namespace ps {

const static std::string ENVIRONMENTS_[]={"NUM_WORKER","NUM_SERVER","ROLE","NODE_URI","NODE_PORT","PS_ROOT_URI","PS_ROOT_PORT"};
static size_t ENVIRONMENTS_COUNT=sizeof(ENVIRONMENTS_)/sizeof(std::string);
const static std::vector<std::string> env_k(ENVIRONMENTS_,ENVIRONMENTS_+ ENVIRONMENTS_COUNT);

/**
 * 获取环境变量
 */
class Environment {
public:
	static Environment* Get(){
		static Environment e;
		return &e;
	}
    const char* find(const char* k){
        return envs.find(k)==envs.end()? getenv(k):envs[k].c_str();
    }
    int envronments_num()
    {
        return envs.size();
    }
private:
	Environment(){
        for(unsigned int i=0;i<env_k.size();i++)
        {
            std::string k=env_k[i];
            auto v=getenv(env_k[i].c_str());
            if(v!=NULL)
                envs.insert(std::make_pair(k,v));
        }
	}
	explicit Environment(const std::unordered_map<std::string, std::string>* e) {
        if (e) envs = *e;
    }
	std::unordered_map<std::string,std::string> envs;
};


}
#endif  // ENV_H_

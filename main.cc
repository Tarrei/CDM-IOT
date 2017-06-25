#include "params.h"
#include <iostream>
#include <fstream>  
#include <sstream>
#include <cmath> 
#include <unistd.h>
using namespace std;
using namespace ps;
/*
 * server logic function
 */
template<typename Val>
class ServerHandle{
public:
    void operator()(KVMeta& req_meta,
            const KVPairs<Val>& req_data,
            Server<Val>* server){

        float alpha=0.01;
        size_t n = req_data.keys.size();
        KVPairs<Val> res;
        int ts=req_meta.timestamp;
        int num=Manager::Get()->NumWorkers();
        cout<<req_meta.push<<"   "<<req_meta.timestamp<<": "<<num<<endl;
        if (req_meta.push){
        //这里还有一点问题，就是更新gradient如何考虑各个worker的情况 
            record[ts]++;
            for (size_t i = 0;i < n; ++i) {
                Key key = req_data.keys[i];
                w[key] += alpha*req_data.vals[i]/num; 
            }
            server->Response(req_meta, res);
        }
        else //收到的是pull请求
        {
            record[ts]++;
            res.keys = req_data.keys;
            res.vals.resize(n);
            if(record[ts]==num){
                for (size_t i = 0;i < n; ++i){
                    Key key = req_data.keys[i];
                    res.vals[i] = w[key];
                }
                for (int id_:Manager::Get()->GetNodeIDs(WorkerGroupID)){
                    req_meta.sender=id_;
                    server->Response(req_meta, res);
                }
            }
        }     
    }
private:
    std::unordered_map<Key, Val> w;
    std::unordered_map<int,int> record;
};

void StartServer(){
    if(!IsServer())
        return;
    Server<float> server(0);
    server.set_request_handle(ServerHandle<float>());
    sleep(1);
    while(true){}
}

/*
 * worker logic function
 */
float Sigmod(float x) {return 1.0/(1+exp(-x));}

// void Gradient(Worker<float>& worker, vector<vector<float>>& x,
//  vector<int>& y,vector<float>& w){

//     int num=x.size();
//     if(num==0) return;
//     int size=x[0].size();
    
//     /*Init keys*/
//     vector<Key> keys(size);
//     for(int i=0;i<size;i++)
//         keys[i]=i;

//     for(int k=0;k<150;k++){
//      cout<<k<<endl;
//         //这里加得到rec_kvs_得到W
//         vector<float> g(size);
//         //float lamda=0.1;
//         for(int i=0;i<num;i++){
//             if(k*num+i>=5)
//             worker.Wait((k*num+i-5)*2+1);
//             float alpha = 4/(1.0+k+i)+0.0001;
//             float h=0;
//             vector<float> row=x[i];
//             int size=row.size();
//             for(int j=0;j<size;j++){
//                 h+=(w[j]*row[j]);
//             }
//             for(int j=0;j<size;j++){
//                 g[j]+=(y[i]-Sigmod(h))*row[j];
//             }
//             worker.Push(keys,g);
//             worker.Pull(keys,&w);
//             for(int j=0;j<size;j++){
//                 w[j]+=alpha*g[j];
//             }
//         }       
//     }    
// }

void Gradient(Worker<float>& worker, vector<vector<float>>& x,
    vector<int>& y,vector<float>& w){

    int num=x.size();
    if(num==0) return;
    int size=x[0].size();
    
    /*Init keys*/
    vector<Key> keys(size);
    for(int i=0;i<size;i++)
        keys[i]=i;
    float alpha =0.01;
    for(int k=0;k<150;k++){
        cout<<"round "<<k<<endl;
        if(k>=5)
            worker.Wait((k-5)*2+1);
        //这里加得到rec_kvs_得到W
        vector<float> g(size);
        //float lamda=0.1;
        for(int i=0;i<num;i++){
            float h=0;
            vector<float> row=x[i];
            int size=row.size();
            for(int j=0;j<size;j++){
                h+=(w[j]*row[j]);
            }
            for(int j=0;j<size;j++){
                g[j]+=(y[i]-Sigmod(h))*row[j];
            }
        }    
        worker.Push(keys,g);
        worker.Pull(keys,&w);
        for(int j=0;j<size;j++){
            w[j]+=alpha*g[j];
        }   
    }   
}

float Predict(vector<vector<float>>& x, vector<int>& y, vector<float>& w){
    
    int num=x.size();
    if(num==0) return -1;
    int size=x[0].size();
    int count=0;
    vector<int> predict;

    for(int i=0;i<num;i++){ 
        float result=0;
        for(int j=0;j<size;j++){
            result+=(x[i][j]*w[j]);
        }
        if (Sigmod(result)>0.5){
            predict.push_back(1);
        }else{
            predict.push_back(0);
        }

        if(predict[i]==y[i])
            count++;
    }

    return float(count)/float(num);
}

void LoadData(vector<vector<float>>& x, vector<int>& y,
    string file_x, string file_y){

    ifstream fin_x(file_x);
    ifstream fin_y(file_y);
    string line,label;

    while (getline(fin_x, line)&&getline(fin_y, label))
    {  
        y.push_back(stoi(label)); 
        /*write line to istringstream*/
        istringstream sin(line); 
        float val=0;  
        vector<float> row;
        /*read from string stream*/
        while (sin>>val)  
            row.push_back(val); 
        /*add the bias*/
        row.push_back(1);
        x.push_back(row);            
    }
}

void StartWorker(){
    if(!IsWorker())
        return;
    Worker<float> worker(0);
    while(true){
        if(NumServers()) break;
    }
    
    vector<vector<float>> x;
    vector<int> y;
    LoadData(x,y,"../data/data/split/train_x_1","../data/data/split/train_y_1");

    int size=x[0].size();
    vector<float> w(size);
    Gradient(worker,x,y,w);

    float precision=Predict(x,y,w);
    cout<<"precision: "<<precision<<endl;
    
    while(true){}
}

int main(){
    /*Start the parameter server*/
    Start();
    StartServer();
    StartWorker();
    Stop();
    return 0;
}
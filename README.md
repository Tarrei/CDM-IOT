# param_server

# how to use
## 1. Download and install zeromq
## 2. Download and install protobuf
-Serialize and Deserialize the message
## 3. Modify message.proto and compile
protoc --proto_path=./ --cpp_out=./ message.proto
## 4. Ensure your GCC compiler support c++11 or higher
## 5. Modify main.cc according to your needs
## 6. Make
usage: ./env.sh ROLE NODE_URI NODE_PORT PS_ROOT_URI PS_ROOT_PORT

Just like:

sh env.sh SCHEDULER 162.105.80.238 3000 162.105.80.238 3000

sh env.sh WORKER 192.168.199.200 1500 162.105.80.238 3000


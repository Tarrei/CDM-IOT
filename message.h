/*
 * message.h
 *
 *  Created on: 2017年6月2日
 *      Author: zjbpoping
 */
#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "node.h"
#include "sarray.h"
#include <string>
#include <vector>

namespace ps{

	enum DataType {
	  CHAR, INT8, INT16, INT32, INT64,
	  UINT8, UINT16, UINT32, UINT64,
	  FLOAT, DOUBLE, OTHER
	};
	/** \brief data type name */
	// static const char* DataTypeName[] = {
	//   "CHAR", "INT8", "INT16", "INT32", "INT64",
	//   "UINT8", "UINT16", "UINT32", "UINT64",
	//   "FLOAT", "DOUBLE", "OTHER"
	// };
	/**
	 * \brief compare if V and W are the same type
	 */
	template<typename V, typename W>
	inline bool SameType() {
	  return std::is_same<typename std::remove_cv<V>::type, W>::value;
	}
	/**
	 * \brief return the DataType of V
	 */
	template<typename V>
	DataType GetDataType() {
	  if (SameType<V, int8_t>()) {
	    return INT8;
	  } else if (SameType<V, int16_t>()) {
	    return INT16;
	  } else if (SameType<V, int32_t>()) {
	    return INT32;
	  } else if (SameType<V, int64_t>()) {
	    return INT64;
	  } else if (SameType<V, uint8_t>()) {
	    return UINT8;
	  } else if (SameType<V, uint16_t>()) {
	    return UINT16;
	  } else if (SameType<V, uint32_t>()) {
	    return UINT32;
	  } else if (SameType<V, uint64_t>()) {
	    return UINT64;
	  } else if (SameType<V, float>()) {
	    return FLOAT;
	  } else if (SameType<V, double>()) {
	    return DOUBLE;
	  } else {
	    return OTHER;
	  }
	}

	struct message{
		/*消息类型*/
		static const int empty = 0;
		enum Command {EMPTY, ADD_NODE, TERMINATE, ACK, HEARTBEAT};
		message():cmd(EMPTY),sender(empty),receiver(empty),
			timestamp(empty),request(false),push(false),customer_id(empty){}
		Command cmd;

		int sender;
		int receiver;
		/*Lamport timestamp for message*/
		int timestamp;
		bool request;
		bool push;
		int customer_id;
		/*向各个节点传输的数据类型及数据*/
		std::vector<Node> node;
		std::vector<DataType> data_type;
		std::vector<SArray<char>> data;
		template <typename V>
  		void AddData(const SArray<V>& val) {
    		data_type.push_back(GetDataType<V>());
    		data.push_back(SArray<char>(val));
  		}
	};	
}

#endif
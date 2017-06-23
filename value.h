/*
 * value.h
 *
 *  Created on: 2017年6月16日
 *      Author: zjbpoping
 */
#ifndef VALUE_H_
#define VALUE_H_

#include "sarray.h"

namespace ps{

	#if USE_KEY32
		using Key = uint32_t;
	#else
		using Key = uint64_t;
	#endif

	template <typename Val>
	struct KVPairs{
		SArray<Key> keys;
		SArray<Val> vals;
		SArray<int> lens;
	};

	struct KVMeta{
		int cmd;
		bool push;
		int sender;
		int timestamp;
	};

}

#endif
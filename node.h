/*
 * node.h
 *
 *  Created on: 2017Äê6ÔÂ2ÈÕ
 *      Author: zjbpoping
 */

#ifndef NODE_H_
#define NODE_H_

#include <string>

namespace ps{

	struct Node{
		static const int EmptyID = 0;
		Node() : id(EmptyID), port(EmptyID), is_recovery(false) {}
	    /** \brief node roles */
	    enum Role { SCHEDULER, SERVER, WORKER };
	    /** \brief the role of this node */
	    Role role;
	    /** \brief node id */
	    int id;
	    /** \brief hostname or ip */
	    std::string hostname;
	    /** \brief the port this node is binding */
	    int port;
	    /** \brief whether this node is created by failover */
	    bool is_recovery;
	};
}



#endif /* NODE_H_ */

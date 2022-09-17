#pragma once

//#include "FileStream.h"
#include <cstdint>
#include <iostream>
#include <map>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>
#include <stack>

// @Author: Pratik Naik

// Developing a simulator framework which can be used for functional modeling
// as well as timing modeling.
// It can used in fields like computer architecture, networking, traffic simulation,
// and many similar domains etc.
//
// It uses data structure and algorithms to build the framework
// Data structure used : link list, fifo, stack
// Algorithms used : Depth first search
//

//
// Hierarchical class structure  for the framework
//      sim_conn_base -> sim_conn_fifo -> sim_conn_module
//
// sim_conn_base - Contains link list implementation. And all APIs to rearrange the
//                 fifos and modules.
// sim_conn_fifo - Contains fifo implementation.
// sim_conn_module - Contains router implementation where multiple links can be
//                   processed.
//
// Components of the simulator:
// fifo - its a queue with a link list which can connect to other fifos and modules.
//        A fifo is also a conn.
// module - its a processing unit with multiple fifos connecting to it. Its like a
//          router. A module is also a conn as it is derived from the same class.
//
// 2 types of conn:
// fifo conn
// module conn
//

template <class PACKET_TYPE>
class sim_conn_framework;

template <class PACKET_TYPE>
class sim_conn_base
{
public:
    // Simulation framework object
    sim_conn_framework<PACKET_TYPE>* m_sim_object{nullptr};

private:
    // Sim conn Identification members
    std::string m_sim_conn_name{""};
    uint32_t m_sim_conn_id{UINT16_MAX};

public:
    // Next and previous sim_conn_fifo
    sim_conn_base<PACKET_TYPE>* m_downstream_this{nullptr}; // Next sim_conn to this
    sim_conn_base<PACKET_TYPE>* m_upstream_this{nullptr};   // Previous sim_conn to this

    // Constructor
    sim_conn_base(sim_conn_framework<PACKET_TYPE>* sim_obj, std::string const name, uint32_t const id) :
        m_sim_object(sim_obj), m_sim_conn_name(name), m_sim_conn_id(id)
    { 
        m_sim_object->m_buffer_netlist.push_back(this);
    }

    // Destructor
    virtual ~sim_conn_base() {}

    // Gettr for the name
    std::string get_sim_conn_name() { return m_sim_conn_name; }

    // Gettr for the id
    uint32_t get_sim_conn_id() { return m_sim_conn_id; }

    // Update the m_downstream_this pointer with the next conn or module
    virtual void update_downstream_conn(sim_conn_base* next_conn) {}

    // Update the m_upstream_this pointer with the previous conn or module
    virtual void update_upstream_conn(sim_conn_base* prev_conn) {}

    // Connect a (conn*.connect_downstream -> conn) or (conn*.connect_downstream -> module)
    void connect_downstream(sim_conn_base* next_conn)
    {
        // 1) Update the m_downstream_this pointer in this conn or the downstream_vector in this
        //    module.
        // 2) Update the m_upstream_this pointer in the next_conn or the upstream_vector in the
        //    next_conn module.

        update_downstream_conn(next_conn);
        next_conn->update_upstream_conn(this);
    }

    // Connect a (conn -> link*.connect_upstream) or (link -> module*.connect_upstream)
    void connect_upstream(sim_conn_base* prev_conn)
    {
        // 1) Update the m_upstream_this pointer in this conn or the usptream_vector in this
        //    module.
        // 2) Update the m_downstream_this pointer in the prev_conn or the downstream_vector in
        //    prev_conn module

        update_upstream_conn(prev_conn);
        prev_conn->update_downstream_conn(this);
    }

    // Return if the sim conn is a module. Return false as this is just a conn.
    virtual bool is_sim_conn_module() { return false; };

    // All below APIs are used by the algorithms for DFS and processing
    virtual void track_upstream_modules_to_this_module(sim_conn_base<PACKET_TYPE>* upstream_module) {}

    virtual void track_sequenced_upstream_conn_fifo_to_module(sim_conn_base<PACKET_TYPE>* upstream_conn) {}

    virtual void get_direct_upstream_conn_fifo_to_module(std::vector<sim_conn_base<PACKET_TYPE>*>& prev_links_to_module)
    {
    }

    virtual void get_sequenced_upstream_conn_fifos_chain_to_module(std::vector<sim_conn_base<PACKET_TYPE>*>& conns_vec)
    {
    }

    virtual void get_adj_upstream_modules_to_this_module(std::vector<sim_conn_base<PACKET_TYPE>*>& modules_vec) {}

    // To process all the links or modules in the model.
    virtual void process_clock() {}
};

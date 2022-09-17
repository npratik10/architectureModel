#pragma once

#include "sim_conn_fifo.hpp"

template <class PACKET_TYPE>
class sim_conn_module : public sim_conn_fifo<PACKET_TYPE>
{
private:
    // Below diagram indicates a module and all its adjacent fifo and modules.
    //
    //	|module_a| -> prev_conn_fifo_0 -> |        | -> next_conn_fifo_0
    //				  prev_conn_fifo_1 -> |module_c| -> next_conn_fifo_1 -> |module_d|
    //                     ....           |        |    ....
    //  |module_b| -> prev_conn_fifo_n -> |        | -> next_conn_fifo_n
    //

    // The vectors below hold all the conn fifos adjacent to this module.

    std::vector<sim_conn_base<PACKET_TYPE>*> m_downstream_conn_fifos_vec; // this vector holds all adjacent downstream fifos to this module (like next_conn_fifo_n)
    std::vector<sim_conn_base<PACKET_TYPE>*> m_upstream_conn_fifos_vec;   // this vector holds all adjacent upstream fifos to this module (like prev_conn_fifo_n)

    // This vector holds all adjacent previous modules to this module (module_a and module_b are upstream modules to
    // module_c) (module_c is upstream module to module_d)
    std::vector<sim_conn_base<PACKET_TYPE>*> m_all_adj_upstream_modules;

    // Below diagram connects a chain of links finally connecting to module_a.
    // prev_conn_fifo_0 -> prev_conn_fifo_1 -> |        |
    //					                       |module_a|
    // prev_conn_fifo_4 -> prev_conn_fifo_3 -> |        |
    //
    // Below structure holds the above chain of links as shown below:
    // {prev_conn_fifo_1, prev_conn_fifo_2, prev_conn_fifo_3, prev_conn_fifo_4}
    //
    // This structure is needed to process all the fifo conn correctly in sequence.
    std::vector<sim_conn_base<PACKET_TYPE>*> m_all_upstream_conn_fifo_chains; // this vector holds the chain of all upstream fifos connecting this module

public:
    // Constructor
    sim_conn_module(sim_conn_framework<PACKET_TYPE>* sim_obj, std::string const name, uint32_t const id);

    // Destructor
    virtual ~sim_conn_module();

    // Return if the link is a module. Return true as this is a module.
    virtual bool is_sim_conn_module() override;

    virtual void update_downstream_conn(sim_conn_base<PACKET_TYPE>* downstream_conn) override;

    virtual void update_upstream_conn(sim_conn_base<PACKET_TYPE>* upstream_conn) override;

    // All below APIs are used by the algorithms for DFS and processing
    virtual void track_upstream_modules_to_this_module(sim_conn_base<PACKET_TYPE>* upstream_module) override;

    virtual void track_sequenced_upstream_conn_fifo_to_module(sim_conn_base<PACKET_TYPE>* upstream_conn) override;

    virtual void
    get_direct_upstream_conn_fifo_to_module(std::vector<sim_conn_base<PACKET_TYPE>*>& prev_links_to_module) override;

    virtual void
    get_sequenced_upstream_conn_fifos_chain_to_module(std::vector<sim_conn_base<PACKET_TYPE>*>& conns_vec) override;

    virtual void get_adj_upstream_modules_to_this_module(std::vector<sim_conn_base<PACKET_TYPE>*>& modules_vec) override;

    // Process clock of a module. This is just an example. Anyone can create a module and do its own processing
    // function.
    virtual void process_clock() override
    {
        // If nothing is connected downstream to the module then return without processing
        if (m_downstream_conn_fifos_vec.empty()) return;

        // This is just temporary implementation of the process_clock for the module
        auto po = static_cast<sim_conn_fifo<PACKET_TYPE>*>(m_downstream_conn_fifos_vec.at(0));
        for (auto pi : m_upstream_conn_fifos_vec)
        {
            if (static_cast<sim_conn_fifo<PACKET_TYPE>*>(pi)->ready() && !po->is_full())
            {
                uint32_t front = static_cast<sim_conn_fifo<PACKET_TYPE>*>(pi)->front();
                po->push(front + 3);
                static_cast<sim_conn_fifo<PACKET_TYPE>*>(pi)->pop();
            }
        }
    }
};

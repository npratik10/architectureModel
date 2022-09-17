#pragma once

#include <vector>
#include "sim_conn_base.hpp"
#include "sim_conn_clock.hpp"
#include "sim_conn_trace.hpp"

#define SIM_CONN_CLOCK sim_conn_clock

template <class PACKET_TYPE>
class sim_conn_framework
{
public:
    // Clock pointer for the FIFO
    SIM_CONN_CLOCK* m_clock{nullptr};

    // Trace object
    sim_conn_trace<PACKET_TYPE> m_trace_obj;

    // Constructor
    sim_conn_framework()
    { 
        m_clock = new sim_conn_clock("clock");
    }

    ~sim_conn_framework() { delete m_clock; }

    // Evaluate the clock
    void clock_tick()
    {
        m_clock->eval();
        process_all_connections();
    }

    // Get current clock
    uint64_t get_curr_clock() { return m_clock->GetCurrCycle(); }

    //
    //    l1->l2->|M1|->l3->|M3|->l6
    //        l4->|M2|->l5->|  |
    // All prev links/modules to this module should be clocked before clocking this module. Before clocking
    // M3 l3 and l5 need to be clocked. Before l3 and l5, M1 and M2 needs to be clocked. And befor M1 is clocked
    // l2 and l1 needs to be clocked.And before M2 is clocked, l4 needs to be clocked.
    //
    // The below simulator processing function takes care of the above the rules.
    //
    void process_all_connections()
    { 
        // Process all the modules in the sequenced vector.
        for (auto conn_module : m_all_modules_sequence)
        {
            // Clock all the chains of prev links before the module.
            std::vector<sim_conn_base<PACKET_TYPE>*> module_upstream_conns_vec;
            conn_module->get_sequenced_upstream_conn_fifos_chain_to_module(module_upstream_conns_vec);

            for (auto conn = module_upstream_conns_vec.rbegin(); conn != module_upstream_conns_vec.rend(); ++conn)
            {
                (*conn)->process_clock();
            }

            // Finally clock the module
            conn_module->process_clock();
        }

        // Clock the last connections in the models.
        for (auto last_conn : m_last_connections)
        {
            // Check if the last connections has a prev links sequence chain
            std::vector<sim_conn_base<PACKET_TYPE>*> last_conn_sequence;
            sim_conn_base<PACKET_TYPE>* temp = last_conn;
            while (temp)
            {
                if (temp->is_sim_conn_module())
                {
                    break;
                }
                else
                {
                    last_conn_sequence.push_back(temp);
                }
                temp = temp->m_upstream_this;
            }

            // Finally clock the last links sequence
            for (auto conn = last_conn_sequence.rbegin(); conn != last_conn_sequence.rend(); ++conn)
            {
                (*conn)->process_clock();
            }
        }
    }

    // Simulator model processing APIs

    // APIs to correctly sort all links and modules so they can processed in a sequence.

    /// <summary>
    /// All the modules in the model needs to be clocked in a particular order. To arrange
    /// these modules in the model correctly, use DFS(Depth First Search) on all the upstream
    /// modules.
    /// The criteria to clock a module is that all the modules on the left should be clocked
    /// before this module is clocked.
    ///
    /// DFS
    /// 1) All the modules should create a map of its immediate neighbouring upstream modules.
    /// 2) Perform a depth search on each of the neighbouring modules in the map.
    /// 3) Create a sequence of all the visited modules
    /// </summary>
    /// 
    
    // To identify all the modules in the model. Also identify the last connections in the model.
    void get_all_modules()
    {
        for (auto conn : m_buffer_netlist)
        {
            if (conn->is_sim_conn_module())
            {
                m_all_modules.push_back(conn);
            }
            else
            {
                if (conn->m_downstream_this == nullptr)
                {
                    m_last_connections.push_back(conn);
                }
            }
        }
    }

    // This is a static API to get all the immediate adjacent upstream modules to a given module
    // This discovery of adjacent modules is needed to perform depth first search on all the
    // modules. The API also sequences the chain of links connected to the module. This sequencing
    // is needed by the clocking unit to process all the links.
    void get_all_adj_prev_modules()
    {
        for (auto conn_module : m_all_modules) // Scan through all the modules in the modules
        {
            std::vector<sim_conn_base<PACKET_TYPE>*> upstream_module_vec;
            conn_module->get_direct_upstream_conn_fifo_to_module(upstream_module_vec); // Get all prev links connected to the module

            for (auto& ll_imp_upstream : upstream_module_vec) // Scan through all the prev links
            {
                std::set<sim_conn_base<PACKET_TYPE>*> visited_module; // To track all previously visited modules
                sim_conn_base<PACKET_TYPE>* temp = ll_imp_upstream;

                // Go through the link list for the link
                while (temp)
                {
                    if (temp->is_sim_conn_module() && (visited_module.find(temp) == visited_module.end()))
                    {
                        // If a new module is found then, add to the neighbouring module list
                        conn_module->track_upstream_modules_to_this_module(temp);
                        visited_module.insert(temp);
                        break;
                    }
                    else
                    {
                        // Add all the links to in sequence for clocking
                        conn_module->track_sequenced_upstream_conn_fifo_to_module(temp);
                    }

                    temp = temp->m_upstream_this;
                }
            }
        }
    }

    // This is a recursive depth first search API to go through the model to get a sequece of all the modules
    // to be clocked.
    // As dfs is performed on all the prev modules and the prev modules need to clocked before the current module.
    // So stack is used to get the sequence in the correct order.
    void dfs(sim_conn_base<PACKET_TYPE>* conn_module)
    {
        // Get all previous modules to the visiting module
        std::vector<sim_conn_base<PACKET_TYPE>*> upstream_modules_vec;
        conn_module->get_adj_upstream_modules_to_this_module(upstream_modules_vec);

        m_visited_modules.insert(conn_module); // Add this module the visited module
        m_dfs_traversal.push(conn_module);     // Push to the stack

        // If there prev modules are not present the dfs cannot be performed on the module.
        if (upstream_modules_vec.empty())
        {
            return;
        }

        // Recursively perform dfs on all the not visited prev modules. Once the dfs on a module is done then
        // push the top of the stack on the sequence vector and pop the stack.
        for (auto upstream_module : upstream_modules_vec)
        {
            if (m_visited_modules.find(upstream_module) == m_visited_modules.end())
            {
                dfs(upstream_module);
                m_all_modules_sequence.push_back(m_dfs_traversal.top());
                m_dfs_traversal.pop();
            }
        }
    }

    // Go through all the not visited modules in the model.
    void dfs_on_modules()
    {
        for (auto conn_module : m_all_modules)
        {
            if (m_visited_modules.find(conn_module) == m_visited_modules.end())
            {
                dfs(conn_module);
                // Pop the last element in the stack and psuh is on the sequenced modules.
                m_all_modules_sequence.push_back(m_dfs_traversal.top());
                m_dfs_traversal.pop();
            }
        }
    }
    
	// To print all the connections in the model
    void print_all_connections()
    {
        std::cout << "Print All Connections:" << std::endl;
        for (auto conn : m_buffer_netlist)
            std::cout << conn->get_sim_conn_name() << std::endl;
    }

    // To print all the modules in the model
    void print_all_module()
    {
        std::cout << "Print All Modules:" << std::endl;
        for (auto conn_module : m_all_modules)
        {
            std::cout << conn_module->get_sim_conn_name() << std::endl;
        }
    }

    // To print all the sequenced modules in the model
    void print_all_module_sequence()
    {
        std::cout << "Print All Modules Sequence:" << std::endl;
        for (auto conn_module : m_all_modules_sequence)
        {
            std::cout << conn_module->get_sim_conn_name() << std::endl;
        }
    }

    // Add all the buffer to the netlist
    std::vector<sim_conn_base<PACKET_TYPE>*> m_buffer_netlist; // Contains all the modules and links in the model
    std::vector<sim_conn_base<PACKET_TYPE>*> m_last_connections; // All the last connections in the model
    std::vector<sim_conn_base<PACKET_TYPE>*> m_all_modules;      // All modules in the model
    std::vector<sim_conn_base<PACKET_TYPE>*> m_all_modules_sequence; // The sequence of modules in the model for processing

    // For DFS algorithm.
    std::set<sim_conn_base<PACKET_TYPE>*> m_visited_modules;
    std::stack<sim_conn_base<PACKET_TYPE>*> m_dfs_traversal;
};

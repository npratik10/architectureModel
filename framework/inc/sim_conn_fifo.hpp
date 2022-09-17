#pragma once
/*
#include "FileStream.h"
#include <cstdint>
#include <iostream>
#include <map>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>*/

#include "sim_conn_base.hpp"

template <class PACKET_TYPE>
class sim_conn_fifo : public sim_conn_base<PACKET_TYPE>
{
private:

    // Fifo Properties
    uint32_t m_buffer_size{0};
    uint32_t m_buffer_delay{1};

    // Buffer Container
    PACKET_TYPE* m_buffer{nullptr};

    // Maturity Fifo
    uint64_t* m_maturity_fifo{nullptr};

    // Buffer queue implementation
    int32_t m_front{0};
    int32_t m_rear{-1};
    uint32_t m_buffer_count{0};

    // Tracking push and pop events last clocked
    int64_t m_last_clk_pushed{-1};
    int64_t m_last_clk_poped{-1};

    bool m_enable_trace{false};

public:
    sim_conn_fifo(sim_conn_framework<PACKET_TYPE>* sim_obj, std::string const name, uint32_t const id);

    sim_conn_fifo(sim_conn_framework<PACKET_TYPE>* sim_obj, std::string const name, uint32_t const id, uint32_t size, uint32_t delay);

    ~sim_conn_fifo();

    // Buffer APIs
    bool push(PACKET_TYPE req);
    bool pop();
    bool ready();
    bool is_full();
    PACKET_TYPE front();
    bool is_empty();
    uint32_t buffer_size();
    void enable_sim_conn_trace();
    void disable_sim_conn_trace();

    // Update the m_downstream_this pointer with the next conn or module
    virtual void update_downstream_conn(sim_conn_base<PACKET_TYPE> *next_conn) override;

    // Update the m_upstream_this pointer with the previous conn or module
    virtual void update_upstream_conn(sim_conn_base<PACKET_TYPE>* prev_conn) override;

    // Simulator process clock
    void process_clock() override;
};

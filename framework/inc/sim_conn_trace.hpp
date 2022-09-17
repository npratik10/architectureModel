#pragma once

#include <vector>
#include <fstream>
#include <sstream>

template <class PACKET_TYPE>
class sim_conn_trace
{
public:
    std::ofstream m_trace_file;

    enum trace_operation_t
    {
        PUSH,
        POP,
        READY
    };

    // Constructor
    sim_conn_trace()
    { 
        m_trace_file.open("sim_conn_trace.txt");
    }

    ~sim_conn_trace()
    { 
        m_trace_file.close();
    }

    void notify(std::string const conn_name, uint64_t const curr_time, trace_operation_t operation, PACKET_TYPE const pkt)
    {
        const std::string op_str = (operation == trace_operation_t::PUSH) ? "PUSH": "POP";
        std::stringstream trace;
        trace << " " << op_str;
        trace << " " << curr_time;
        trace << ": " << conn_name;
        trace << " " << print_packet(pkt) << std::endl;

        m_trace_file << trace.str();
    }

    std::string const print_packet(PACKET_TYPE const pkt)
    {
        std::stringstream temp;
        temp << pkt;
        return temp.str();
    }
};

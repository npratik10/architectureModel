#pragma once
#include <cstdint>
#include <string>

class sim_conn_clock
{
    std::string m_name;
    uint64_t m_cycle_count;

public:
    sim_conn_clock(std::string name) : m_name(name) { this->m_cycle_count = 0; }

    void eval() { m_cycle_count++; }

    std::string GetClockName() { return m_name; }

    uint64_t GetCurrCycle() { return m_cycle_count; }
};

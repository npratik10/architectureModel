#pragma once

#include "sim_conn_fifo.hpp"
#include "sim_conn_framework.hpp"

template <class PACKET_TYPE>
sim_conn_fifo<PACKET_TYPE>::sim_conn_fifo(sim_conn_framework<PACKET_TYPE>* sim_obj, std::string const name, uint32_t const id):
    sim_conn_base<PACKET_TYPE>(sim_obj, name, id)
{
}

template <class PACKET_TYPE>
sim_conn_fifo<PACKET_TYPE>::sim_conn_fifo(sim_conn_framework<PACKET_TYPE>* sim_obj, std::string const name, uint32_t const id, uint32_t size, uint32_t delay):
    sim_conn_base<PACKET_TYPE>(sim_obj, name, id),
    m_buffer_size(size),
    m_buffer_delay(delay)
{
    m_buffer = new PACKET_TYPE[m_buffer_size];
    m_maturity_fifo = new uint64_t[m_buffer_size];
}

template <class PACKET_TYPE>
sim_conn_fifo<PACKET_TYPE>::~sim_conn_fifo()
{
    delete[] m_buffer;
    delete[] m_maturity_fifo;
}

template <class PACKET_TYPE>
bool sim_conn_fifo<PACKET_TYPE>::push(PACKET_TYPE req)
{
    uint64_t current_clock = this->m_sim_object->get_curr_clock();

    if (is_full() || (m_last_clk_pushed == current_clock))
        return false;

    m_rear = ++m_rear % m_buffer_size;
    m_buffer[m_rear] = req;
    m_maturity_fifo[m_rear] = current_clock + m_buffer_delay;
    m_buffer_count++;
    m_last_clk_pushed = current_clock;
    if (m_enable_trace)
        this->m_sim_object->m_trace_obj.notify(this->get_sim_conn_name(), current_clock, sim_conn_trace<PACKET_TYPE>::PUSH, req);
    return true;
}

template <class PACKET_TYPE>
bool sim_conn_fifo<PACKET_TYPE>::pop()
{
    uint64_t current_clock = this->m_sim_object->get_curr_clock();

    if (is_empty() || !ready() || (m_last_clk_poped == current_clock))
        return false;

    if (m_enable_trace)
        this->m_sim_object->m_trace_obj.notify(this->get_sim_conn_name(), current_clock, sim_conn_trace<PACKET_TYPE>::POP, m_buffer[m_front]);
    m_front = ++m_front % m_buffer_size;
    m_buffer_count--;
    m_last_clk_poped = current_clock;
    return true;
}

template <class PACKET_TYPE>
bool sim_conn_fifo<PACKET_TYPE>::ready()
{
    uint64_t current_clock = this->m_sim_object->get_curr_clock();
    return ((m_maturity_fifo[m_front] <= current_clock) && (!is_empty()));
}

template <class PACKET_TYPE>
bool sim_conn_fifo<PACKET_TYPE>::is_full()
{
    return (m_buffer_count == m_buffer_size);
}

template <class PACKET_TYPE>
PACKET_TYPE sim_conn_fifo<PACKET_TYPE>::front()
{
    return m_buffer[m_front];
}

template <class PACKET_TYPE>
bool sim_conn_fifo<PACKET_TYPE>::is_empty()
{
    return (m_buffer_count == 0);
}

template <class PACKET_TYPE>
uint32_t sim_conn_fifo<PACKET_TYPE>::buffer_size()
{
    return m_buffer_count;
}

template <class PACKET_TYPE>
void sim_conn_fifo<PACKET_TYPE>::enable_sim_conn_trace()
{
    m_enable_trace = true;
}

template <class PACKET_TYPE>
void sim_conn_fifo<PACKET_TYPE>::disable_sim_conn_trace()
{
    m_enable_trace = false;
}

template <class PACKET_TYPE>
void sim_conn_fifo<PACKET_TYPE>::update_downstream_conn(sim_conn_base<PACKET_TYPE>* next_conn)
{
    if (next_conn == this)
    {
        throw std::logic_error("Trying to connect the same sim_conn");
        return;
    }

    if (this->m_downstream_this)
    {
        if (this->m_downstream_this != next_conn)
            throw std::logic_error("The sim_conn is already connected downstream to a different sim_conn");
        return;
    }

    this->m_downstream_this = next_conn;
}

template <class PACKET_TYPE>
void sim_conn_fifo<PACKET_TYPE>::update_upstream_conn(sim_conn_base<PACKET_TYPE>* prev_conn)
{
    if (prev_conn == this)
    {
        throw std::logic_error("Trying to connect the same sim_conn");
        return;
    }

    if (this->m_upstream_this)
    {
        if (this->m_upstream_this != prev_conn)
            throw std::logic_error("The sim_conn is already connected upstream to a different sim_conn");
        return;
    }

    this->m_upstream_this = prev_conn;
}

template <class PACKET_TYPE>
void sim_conn_fifo<PACKET_TYPE>::process_clock()
{
    if ((this->m_downstream_this != nullptr) && !(this->m_downstream_this->is_sim_conn_module()))
    {
        if (this->ready() && !static_cast<sim_conn_fifo*>(this->m_downstream_this)->is_full())
        {
            static_cast<sim_conn_fifo*>(this->m_downstream_this)->push(this->front());
            this->pop();
        }
    }
}

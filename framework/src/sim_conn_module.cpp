#pragma once

#include "sim_conn_module.hpp"

template <class PACKET_TYPE>
sim_conn_module<PACKET_TYPE>::sim_conn_module(sim_conn_framework<PACKET_TYPE>* sim_obj, std::string const name, uint32_t const id):
    sim_conn_fifo<PACKET_TYPE>(sim_obj, name, id)
{
}

template <class PACKET_TYPE>
sim_conn_module<PACKET_TYPE>::~sim_conn_module()
{
}

template <class PACKET_TYPE>
bool sim_conn_module<PACKET_TYPE>::is_sim_conn_module()
{
    return true;
}

template <class PACKET_TYPE>
void sim_conn_module<PACKET_TYPE>::update_downstream_conn(sim_conn_base<PACKET_TYPE>* downstream_conn)
{
    m_downstream_conn_fifos_vec.push_back(downstream_conn);
}

template <class PACKET_TYPE>
void sim_conn_module<PACKET_TYPE>::update_upstream_conn(sim_conn_base<PACKET_TYPE>* upstream_conn)
{
    m_upstream_conn_fifos_vec.push_back(upstream_conn);
}

template <class PACKET_TYPE>
void sim_conn_module<PACKET_TYPE>::track_upstream_modules_to_this_module(sim_conn_base<PACKET_TYPE>* upstream_module)
{
    m_all_adj_upstream_modules.push_back(upstream_module);
}

template <class PACKET_TYPE>
void sim_conn_module<PACKET_TYPE>::track_sequenced_upstream_conn_fifo_to_module(
    sim_conn_base<PACKET_TYPE>* upstream_conn)
{
    m_all_upstream_conn_fifo_chains.push_back(upstream_conn);
}

template <class PACKET_TYPE>
void sim_conn_module<PACKET_TYPE>::get_direct_upstream_conn_fifo_to_module(
    std::vector<sim_conn_base<PACKET_TYPE>*>& prev_links_to_module)
{
    prev_links_to_module = m_upstream_conn_fifos_vec;
}

template <class PACKET_TYPE>
void sim_conn_module<PACKET_TYPE>::get_sequenced_upstream_conn_fifos_chain_to_module(
    std::vector<sim_conn_base<PACKET_TYPE>*>& conns_vec)
{
    conns_vec = m_all_upstream_conn_fifo_chains;
}

template <class PACKET_TYPE>
void sim_conn_module<PACKET_TYPE>::get_adj_upstream_modules_to_this_module(
    std::vector<sim_conn_base<PACKET_TYPE>*>& modules_vec)
{
    modules_vec = m_all_adj_upstream_modules;
}

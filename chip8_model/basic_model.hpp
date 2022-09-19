#pragma once

#include "sim_conn_framework.hpp"
#include "sim_conn_fifo.cpp"
#include "sim_conn_module.cpp"

class basic_model
{
public:

	sim_conn_framework<int>* m_sim_obj{ new sim_conn_framework<int>() };
	sim_conn_fifo<int>   buffer_a{ m_sim_obj, "buffer_a", 0, 2, 1 };
	sim_conn_module<int> module_a{ m_sim_obj, "module_a", 1 };
	sim_conn_fifo<int>   buffer_b{ m_sim_obj, "buffer_b", 2, 3, 2 };

	basic_model() {}
	~basic_model() {}

	void basic_module_fifo_chain_sim()
	{
        buffer_a.connect_downstream(&module_a);
        module_a.connect_downstream(&buffer_b);

        m_sim_obj->get_all_modules();              // Get All modules in the model
        m_sim_obj->get_all_adj_prev_modules();     // Find all upstream neighbouring modules
        m_sim_obj->dfs_on_modules();               // Perform DFS
        //m_sim_obj->print_all_module_sequence();    // Print Final DFS result

        ASSERT_TRUE(buffer_a.is_empty()); // Initially the FIFO is empty
        ASSERT_TRUE(buffer_a.push(2));    // Expect a successful push to the buffer

        m_sim_obj->clock_tick();

        ASSERT_TRUE(buffer_a.push(3)); // Expect a successful push to the buffer

        ASSERT_FALSE(buffer_b.ready()); // Should be ready after the delay of 1 clk
        m_sim_obj->clock_tick();
        ASSERT_FALSE(buffer_b.ready()); // Should be ready after the delay of 1 clk
        m_sim_obj->clock_tick();

        ASSERT_TRUE(buffer_b.ready()); // Should be ready after the delay of 1 clk
        EXPECT_EQ(buffer_b.front(), 5);
        ASSERT_TRUE(buffer_b.pop()); // Pop the buffer once the Fifo is ready

        m_sim_obj->clock_tick();

        ASSERT_TRUE(buffer_b.ready()); // Should be ready after the delay of 1 clk
        EXPECT_EQ(buffer_b.front(), 6);
        ASSERT_TRUE(buffer_b.pop()); // Pop the buffer once the Fifo is ready

        delete m_sim_obj;
	}

	void print_hello()
	{
		std::cout << "hello" << std::endl;
	}
};

//#include "pch.h"
#include "sim_conn_gtest.hpp"

sim_conn_gtest::sim_conn_gtest()
{
}

sim_conn_gtest::~sim_conn_gtest()
{
}

TEST_F(sim_conn_gtest, check_clock_functionality)
{
    sim_conn_framework<int>* m_sim_obj{new sim_conn_framework<int>()};

    EXPECT_EQ(m_sim_obj->get_curr_clock(), 0);
    m_sim_obj->clock_tick();

    EXPECT_EQ(m_sim_obj->get_curr_clock(), 1);
    m_sim_obj->clock_tick();

    EXPECT_EQ(m_sim_obj->get_curr_clock(), 2);
    m_sim_obj->clock_tick();

    delete m_sim_obj;
}

TEST_F(sim_conn_gtest, check_basic_buffer)
{
    sim_conn_framework<int>* m_sim_obj{new sim_conn_framework<int>()};
    sim_conn_fifo<int> buffer{m_sim_obj, "buffer", 0, 4, 1};

    ASSERT_TRUE(buffer.is_empty());

    //  Front           Back
    //    2    5    8    1    push 3 fails
    ASSERT_TRUE(buffer.push(2));
    m_sim_obj->clock_tick();

    ASSERT_TRUE(buffer.push(5));
    m_sim_obj->clock_tick();

    ASSERT_TRUE(buffer.push(8));
    m_sim_obj->clock_tick();

    ASSERT_TRUE(buffer.push(1));
    ASSERT_FALSE(buffer.push(3));

    ASSERT_TRUE(buffer.is_full());
    ASSERT_FALSE(buffer.is_empty());

    ASSERT_TRUE(buffer.buffer_size() == 4);
    ASSERT_TRUE(buffer.front() == 2);
    ASSERT_TRUE(buffer.pop());

    m_sim_obj->clock_tick();
    ASSERT_TRUE(buffer.buffer_size() == 3);
    ASSERT_TRUE(buffer.front() == 5);
    ASSERT_TRUE(buffer.pop());

    ASSERT_FALSE(buffer.is_full());
    ASSERT_FALSE(buffer.is_empty());

    m_sim_obj->clock_tick();
    ASSERT_TRUE(buffer.buffer_size() == 2);
    ASSERT_TRUE(buffer.front() == 8);
    ASSERT_TRUE(buffer.pop());

    m_sim_obj->clock_tick();
    ASSERT_TRUE(buffer.buffer_size() == 1);
    ASSERT_TRUE(buffer.front() == 1);
    ASSERT_TRUE(buffer.pop());

    m_sim_obj->clock_tick();
    ASSERT_TRUE(buffer.is_empty());
    ASSERT_FALSE(buffer.pop());

    delete m_sim_obj;
}

TEST_F(sim_conn_gtest, check_buffer_functionality)
{
    sim_conn_framework<int>* m_sim_obj{new sim_conn_framework<int>()};
    sim_conn_fifo<int> buffer{m_sim_obj, "buffer", 0, 2, 1};

    ASSERT_TRUE(buffer.is_empty());     // Initially the FIFO is empty
    ASSERT_TRUE(buffer.push(2));        // Expect a successful push to the buffer
    ASSERT_FALSE(buffer.push(1));       // Cannot push to the buffer in the same clock

    EXPECT_EQ(buffer.buffer_size(), 1); // Size is 1 -> only one successful push
    ASSERT_FALSE(buffer.push(1));       // Cannot push to the buffer in the same clock

    EXPECT_EQ(buffer.front(), 2);       // 2 at the front of the FIFO
    ASSERT_FALSE(buffer.ready());       // Cannot be ready in the same clock as the delay is 1
    ASSERT_FALSE(buffer.pop());         // Cannot pop the buffer until the Fifo is ready

    m_sim_obj->clock_tick();

    ASSERT_TRUE(buffer.push(3));        // Expect a successful push to the buffer
    ASSERT_TRUE(buffer.ready());        // Should be ready after the delay of 1 clk
    ASSERT_TRUE(buffer.pop());          // Pop the buffer once the Fifo is ready

    EXPECT_EQ(buffer.front(), 3);       // 3 at the front of the FIFO

    ASSERT_FALSE(buffer.ready());       // Cannot be ready in the same clock as the delay is 1
    ASSERT_FALSE(buffer.pop());         // Cannot pop the buffer until the Fifo is ready

    m_sim_obj->clock_tick();

    ASSERT_TRUE(buffer.ready());        // Should be ready after the delay of 1 clk
    ASSERT_TRUE(buffer.pop());          // Pop the buffer once the Fifo is ready

    ASSERT_TRUE(buffer.is_empty());     // The FIFO is empty after 2 pops
    ASSERT_FALSE(buffer.pop());         // Cannot pop an empty buffer

    delete m_sim_obj;
}

TEST_F(sim_conn_gtest, check_buffer_chain)
{
    sim_conn_framework<int>* m_sim_obj{new sim_conn_framework<int>()};
    sim_conn_fifo<int> buffer_a{m_sim_obj, "buffer_a", 0, 2, 1};
    sim_conn_fifo<int> buffer_b{m_sim_obj, "buffer_b", 0, 3, 2};
    sim_conn_fifo<int> buffer_c{m_sim_obj, "buffer_c", 0, 2, 1};

    buffer_a.connect_downstream(&buffer_b);
    buffer_c.connect_upstream(&buffer_b);

    m_sim_obj->get_all_modules();

    ASSERT_TRUE(buffer_a.is_empty()); // Initially the FIFO is empty
    ASSERT_TRUE(buffer_a.push(2));    // Expect a successful push to the buffer
    ASSERT_FALSE(buffer_a.push(1));   // Cannot push to the buffer in the same clock

    EXPECT_EQ(buffer_a.buffer_size(), 1); // Size is 1 -> only one successful push

    m_sim_obj->clock_tick();

    ASSERT_TRUE(buffer_a.push(3)); // Expect a successful push to the buffer
    
    m_sim_obj->clock_tick();
    m_sim_obj->clock_tick();
    m_sim_obj->clock_tick();

    ASSERT_TRUE(buffer_c.ready()); // Should be ready after the delay of 1 clk
    EXPECT_EQ(buffer_c.front(), 2);
    ASSERT_TRUE(buffer_c.pop());   // Pop the buffer once the Fifo is ready
    ASSERT_FALSE(buffer_c.ready());

    m_sim_obj->clock_tick();

    ASSERT_TRUE(buffer_c.ready()); // Should be ready after the delay of 1 clk
    EXPECT_EQ(buffer_c.front(), 3);
    ASSERT_TRUE(buffer_c.pop()); // Pop the buffer once the Fifo is ready

    //-----------------------------//
    uint64_t initial_clk_ticks  = m_sim_obj->get_curr_clock();
    bool success_push = false;
    uint64_t num_sucessful_push = 0, num_unsucessful_push = 0;
    for (int i = 0; i < 8; i++)
    {
        do
        {
            success_push = false;
            if (!buffer_a.is_full())
            {
                success_push = buffer_a.push(i);
                num_sucessful_push++;
            }
            else
            {
                num_unsucessful_push++;
            }

            if (num_unsucessful_push == 3)
                break;

            m_sim_obj->clock_tick();

        } while (!success_push);
    }

    uint64_t num_clk_ticks = m_sim_obj->get_curr_clock() - initial_clk_ticks;
    EXPECT_EQ(num_clk_ticks, 9);

    EXPECT_EQ(num_sucessful_push, 7);
    EXPECT_EQ(num_unsucessful_push, 3);

    delete m_sim_obj;
}

TEST_F(sim_conn_gtest, check_basic_module_fifo_chain)
{
    sim_conn_framework<int>* m_sim_obj{new sim_conn_framework<int>()};
    sim_conn_fifo<int>   buffer_a{m_sim_obj, "buffer_a", 0, 2, 1};
    sim_conn_module<int> module_a{m_sim_obj, "module_a", 1};
    sim_conn_fifo<int>   buffer_b{m_sim_obj, "buffer_b", 2, 3, 2};

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

TEST_F(sim_conn_gtest, check_basic_model)
{
    sim_conn_framework<int>* m_sim_obj{new sim_conn_framework<int>()};
    sim_conn_fifo<int> buffer_1{m_sim_obj, "buffer_1", 1, 2, 1};
    sim_conn_fifo<int> buffer_2{m_sim_obj, "buffer_2", 2, 2, 1};
    sim_conn_fifo<int> buffer_3{m_sim_obj, "buffer_3", 3, 2, 1};
    sim_conn_fifo<int> buffer_4{m_sim_obj, "buffer_4", 4, 2, 1};
    sim_conn_fifo<int> buffer_5{m_sim_obj, "buffer_5", 5, 2, 1};
    sim_conn_fifo<int> buffer_6{m_sim_obj, "buffer_6", 6, 2, 1};
    sim_conn_fifo<int> buffer_7{m_sim_obj, "buffer_7", 7, 2, 1};
    sim_conn_fifo<int> buffer_8{m_sim_obj, "buffer_8", 8, 2, 1};
    sim_conn_fifo<int> buffer_9{m_sim_obj, "buffer_9", 9, 2, 1};

    sim_conn_module<int> module_1{m_sim_obj, "module_1", 10};
    sim_conn_module<int> module_2{m_sim_obj, "module_2", 11};
    sim_conn_module<int> module_3{m_sim_obj, "module_3", 12};
    sim_conn_module<int> module_4{m_sim_obj, "module_4", 13};
    sim_conn_module<int> module_5{m_sim_obj, "module_5", 14};

    buffer_1.connect_downstream(&buffer_2);
    buffer_2.connect_downstream(&module_1);
    module_1.connect_downstream(&buffer_3);
    module_2.connect_upstream(&buffer_3);
    module_2.connect_downstream(&buffer_4);
    buffer_4.connect_downstream(&module_3);
    module_3.connect_downstream(&buffer_5);
    buffer_5.connect_downstream(&module_4);
    module_4.connect_downstream(&buffer_6);

    module_1.connect_downstream(&buffer_7);
    module_2.connect_downstream(&buffer_8);
    buffer_7.connect_downstream(&module_5);
    buffer_8.connect_downstream(&module_5);
    module_5.connect_downstream(&buffer_9);
    buffer_9.connect_downstream(&module_4);


    // DFS to arrange all connection for model trigger
    m_sim_obj->get_all_modules();               // Get All modules in the model
    m_sim_obj->get_all_adj_prev_modules();      // Find all upstream neighbouring modules
    m_sim_obj->dfs_on_modules();                // Perform DFS
    m_sim_obj->print_all_module_sequence();     // Print Final DFS result

    buffer_1.push(4);

    while (!buffer_6.ready())
    {
        m_sim_obj->clock_tick();
    }

    uint32_t final_op = buffer_6.front();
    EXPECT_EQ(final_op, 16);

    uint64_t op_ready_cycle = m_sim_obj->get_curr_clock();
    EXPECT_EQ(op_ready_cycle, 6);

    delete m_sim_obj;
}
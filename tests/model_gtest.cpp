#include "pch.h"
#include "model_gtest.hpp"

model_gtest::model_gtest()
{
}

model_gtest::~model_gtest()
{
}

TEST_F(model_gtest, basic_model_print_test)
{
	basic_model model;
	model.print_hello();
}

TEST_F(model_gtest, basic_module_fifo_chain_test)
{
	basic_model model;
	model.basic_module_fifo_chain_sim();
}
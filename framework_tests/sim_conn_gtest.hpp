#pragma once

#include "gtest/gtest.h"
#include "sim_conn_framework.hpp"
#include "sim_conn_fifo.cpp"
#include "sim_conn_module.cpp"

using namespace std;

class sim_conn_gtest : public ::testing::Test
{
public:
  sim_conn_gtest();
  virtual ~sim_conn_gtest();
};

#pragma once

#include "basic_model.hpp"
#include "gtest/gtest.h"

using namespace std;

class model_gtest : public ::testing::Test
{
public:
	model_gtest();
	virtual ~model_gtest();
};
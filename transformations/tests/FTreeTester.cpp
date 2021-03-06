/***********************************************************************************
 * Copyright (c) 2017, UT-Battelle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the xacc nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributors:
 *   Initial API and implementation - Alex McCaskey
 *
 **********************************************************************************/
#include <gtest/gtest.h>
#include "Fenwick.hpp"

TEST(FTreeTester, checkSimple) {


	FenwickTree f(8);

	std::map<int, std::set<int>> expectedParity {
		{0, {}},
		{1, {0}},
		{2, {1}},
		{3, {2,1}},
		{4, {3}},
		{5, {4,3}},
		{6, {5,3}},
		{7, {6,5,3}}
	};

	for (auto& kv : expectedParity) {
		auto pSet = f.getParitySet(kv.first);
		std::set<int> ids;
		for (auto& node : pSet) {
			ids.insert(node->index);
		}

		EXPECT_TRUE(ids == kv.second);
	}

	std::map<int, std::set<int>> expectedUpdate {
		{0, {1,3,7}},
		{1, {3,7}},
		{2, {3,7}},
		{3, {7}},
		{4, {5,7}},
		{5, {7}},
		{6, {7}},
		{7, {}}
	};

	for (auto& kv : expectedUpdate) {
		auto uSet = f.getUpdateSet(kv.first);
		std::set<int> ids;
		for (auto& node : uSet) {
			ids.insert(node->index);
		}

		EXPECT_TRUE(ids == kv.second);
	}

	std::map<int, std::set<int>> expectedChildren {
		{0, {}},
		{1, {0}},
		{2, {}},
		{3, {2,1}},
		{4, {}},
		{5, {4}},
		{6, {}},
		{7, {6,5,3}}
	};

	for (auto& kv : expectedChildren) {
		auto cSet = f.getChildrenSet(kv.first);
		std::set<int> ids;
		for (auto& node : cSet) {
			ids.insert(node->index);
		}

		EXPECT_TRUE(ids == kv.second);
	}

}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  return ret;
}


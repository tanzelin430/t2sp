/*******************************************************************************
* Copyright 2021 Intel Corporation
*
* Licensed under the BSD-2-Clause Plus Patent License (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* https://opensource.org/licenses/BSDplusPatent
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions
* and limitations under the License.
*
*
* SPDX-License-Identifier: BSD-2-Clause-Patent
*******************************************************************************/
#include "util.h"
#define SIZE 10

int main(void) {
  // A simple 2-D loop
  Var i, j;
  Func A(Int(32), {i, j}), B(Int(32), {i, j});
  A(i, j) = select(i == 0, 1,
                   select(i == 1, 1, A(i-1, j) + A(i-2, j)));
  B(i, j) = A(i, j);

  // Space-time-transform
  A.merge_ures(B)
   .set_bounds(i, 0, SIZE)
   .set_bounds(j, 0, SIZE);

  // Compile and run
  Target target = get_host_target();
  Buffer<int> golden = B.realize({SIZE, SIZE}, target);

  // A simple 2-D loop
  Var i2, j2;
  Func A2(Int(32), {i2, j2}), B2(Int(32), {i2, j2});
  A2(i2, j2) = select(i2 == 0, 1,
                      select(i2 == 1, 1, A2(i2-1, j2) + A2(i2-2, j2)));
  B2(i2, j2) = A2(i2, j2);

  // Space-time-transform
  A2.merge_ures(B2)
    .set_bounds(i2, 0, SIZE)
    .set_bounds(j2, 0, SIZE);

#ifdef CASE1
  A2.space_time_transform({i2, j2},  // the source loops
                          {s, t},    // the destination loops
                          {0, 1,     // space loop: s = j2
                           1, 1},    // time loop: t = i2 + j2
                          {j2, s,    // reverse transform
                           i2, t - j2});
#elif CASE2
  A2.space_time_transform({i2, j2},  // the source loops
                          {s, t},    // the destination loops
                          {1, 0,     // space loop: s = i2
                           1, 1},    // time loop: t = i2 + j2
                          {i2, s,    // reverse transform
                           j2, t - i2});
#elif CASE3
  A2.space_time_transform({i2, j2},  // the source loops
                          {s, t},    // the destination loops
                          {1, 1,     // space loop: s = i2 + j2
                           1, 0},    // time loop: t = i2
                          {i2, t,    // reverse transform
                           j2, s - i2});
#elif CASE4
  A2.space_time_transform({i2, j2},          // the source loops
                          {s, t},            // the destination loops
                          {-1, 1,            // space loop: s = j2 - i2
                           1, 1},            // time loop: t = j2 + i2
                          {j2, (s + t) / 2,  // reverse transform
                           i2, j2 - s});
#endif

  Buffer<int> result = B2.realize({SIZE, SIZE}, target);
  // Check correctness.
  check_equal_2D<int>(golden, result);
  cout << "Success!\n";
}

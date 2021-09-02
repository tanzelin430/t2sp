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
#include "Halide.h"
#include "util.h"

using namespace Halide;

#define IIC 16
#define IC  1
#define OC  1
#define Q   5

int main(void) {
    ImageParam x(Int(32), 1), w(Int(32), 1);
    x.set(new_data<int, Q+IIC-1>(VALUES::SEQUENTIAL));
    w.set(new_data<int, Q>(VALUES::SEQUENTIAL));

    Var q2, oc2, ic2, iic2;
    Func Z2(Int(32), {q2, iic2, ic2, oc2});
    Func out2;
    Z2(q2, iic2, ic2, oc2) = select(q2 == 0, 0, Z2(q2 - 1, iic2, ic2, oc2)) 
                                            + x(q2 + iic2 + ic2*IIC + oc2*IIC*IC) * w(q2);
    out2(iic2, ic2, oc2) = select(q2 == Q - 1, Z2(q2, iic2, ic2, oc2));
    Z2.merge_ures(out2)
      .set_bounds(oc2, 0, OC)
      .set_bounds(ic2, 0, IC)
      .set_bounds(iic2, 0, IIC)
      .set_bounds(q2, 0, Q)
      .reorder(iic2, q2, ic2, oc2);
    // Compile and run
    Target target = get_host_target();
    Buffer<int> golden = out2.realize({IIC, IC, OC}, target);

    Var q, oc, ic, iic;
    Func Z(Int(32), {q, iic, ic, oc});
    Func out;
    Z(q, iic, ic, oc) = select(q == 0, 0, Z(q-1, iic, ic, oc))
                                        + x(q + iic + ic*IIC + oc*IIC*IC) * w(q);
    out(iic, ic, oc) = select(q == Q-1, Z(q, iic, ic, oc));

    Var k, t;
    // Compile and run
    Z.merge_ures(out)
     .set_bounds(oc, 0, OC)
     .set_bounds(ic, 0, IC)
     .set_bounds(iic, 0, IIC)
     .set_bounds(q, 0, Q)
     .reorder(iic, q, ic, oc)
     .space_time_transform({iic, q},
                           {k, t},
                          {1, 1,
                           0, 1},
                          {q, t,
                           iic, k-q},
                           SpaceTimeTransform::NoCheckTime);

    Buffer<int> result = out.realize({IIC, IC, OC}, target);
    check_equal_2D<int>(golden, result);
    cout << "Success!\n";
    return 0;
}

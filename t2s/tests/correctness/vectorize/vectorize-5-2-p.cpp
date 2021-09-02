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
// A positive test: split before merge_ures and then vectorize.

#include "util.h"

#define SIZE 32
#define II   4

int main(void) {
    // Define the compute.
    ImageParam a(Int(32), 1, "a");
    Func A(PLACE0), B(PLACE1);
    Var i, oi, ii;
    B(i) = a(i);
    A(i) = B(i);

    Target target = get_host_target();
    target.set_feature(Target::IntelFPGA);

    // Generate input and run.
    Buffer<int> in = new_data<int, SIZE>(SEQUENTIAL); //or RANDOM
    a.set(in);
    Buffer<int> golden = A.realize(SIZE, target);

    // Isolate. Re-compile and run.
    Func A_feeder(PLACE2);
    A.split(i, oi, ii, II);
    B.split(i, oi, ii, II);
    A.merge_ures(B)
     .isolate_producer_chain(B, A_feeder)
     .vectorize(ii);
    remove(getenv("BITSTREAM"));
    Buffer<int> out = A.realize(SIZE, target);

    // Check correctness.
    check_equal<int>(golden, out);
    cout << "Success!\n";
}



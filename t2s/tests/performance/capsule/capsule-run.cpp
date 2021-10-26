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
// The header file generated by gemm.cpp
#include "capsule-interface.h"

// Loop bounds
#include "sizes.h"

// The only header file needed for including T2S.
#include "HalideBuffer.h"

// For printing output
#include <stdio.h>
#include <iostream>

// For validation of results.
#include <assert.h>

using namespace std;

int main()
{
    Halide::Runtime::Buffer<float> i(MK, MH, TOTAL_CI, TOTAL_IW, TOTAL_IH, TOTAL_B), k(MW, MK, TOTAL_CI, TOTAL_CO, KW, KH);
    for (size_t b = 0; b < TOTAL_B; b++)
    for (size_t h = 0; h < TOTAL_IH; h++)
    for (size_t w = 0; w < TOTAL_IW; w++)
    for (size_t ci = 0; ci < TOTAL_CI; ci++) {
        for (size_t mh = 0; mh < MH; mh++) {
            for (size_t mk = 0; mk < MK; mk++) {
                i(mk, mh, ci, w, h, b) = random();
            }
        }
    }
    for (size_t kh = 0; kh < KH; kh++)
    for (size_t kw = 0; kw < KW; kw++)
    for (size_t co = 0; co < TOTAL_CO; co++)
    for (size_t ci = 0; ci < TOTAL_CI; ci++) {
        for (size_t mk = 0; mk < MK; mk++) {
            for (size_t mw = 0; mw < MW; mw++) {
                k(mw, mk, ci, co, kw, kh) = random();
            }
        }
    }
    Halide::Runtime::Buffer<float> o(COO, BB, MW, H*W, CO, MH, B);
    CAPSULE(i, k, o);

#ifdef TINY
    // Validate the results
    for (int b = 0; b < B; b++)
    for (int h = 0; h < H; h++)
    for (int w = 0; w < W; w++) {
        for (int bb = 0; bb < BB; bb++)
        for (int co = 0; co < CO; co++)
        for (int mh = 0; mh < MH; mh++)
        for (int coo = 0; coo < COO; coo++)
        for (int mw = 0; mw < MW; mw++) {
            float golden = 0.0f;
            for (int r_kh = 0; r_kh < KH; r_kh++)
            for (int r_kw = 0; r_kw < KW; r_kw++)
            for (int r_mk = 0; r_mk < MK; r_mk++)
            for (int r_ci = 0; r_ci < TOTAL_CI; r_ci++) {
                size_t total_iw = w*2 + r_kw;
                size_t total_ih = h*2 + r_kh;
                size_t total_co = coo + COO * co;
                size_t total_b = bb + BB * b;
                golden += i(r_mk, mh, r_ci, total_iw, total_ih, total_b) * k(mw, r_mk, r_ci, total_co, r_kw, r_kh);
            }
            size_t hw = w + W * h;
            assert(fabs(golden - o(coo, bb, mw, hw, co, mh, b)) < 0.005*fabs(golden));
        }
    }
#else
    // Report performance. DSPs, FMax and ExecTime are automatically figured out from the static analysis
    // during FPGA synthesis and and the dynamic profile during the FGPA execution.
    // A10PAC on DevCloud has 33GB/s memory bandwidth
    double mem_bandwidth = 33;
    double compute_roof = 2 * DSPs() * FMax();
     // Total operations (GFLOP for CONV), independent of designs
    double number_ops = 2 * (long)(B * H * W) * (long)(BB * CO * MH * MW * COO) * (long)(CI * KH * KW * MK * CII);
    double number_bytes = (long)(TOTAL_IW * TOTAL_IH * TOTAL_CI * TOTAL_B * MH * MK) * 4
                        + (long)(KW * KH * TOTAL_CI * TOTAL_CO * MK * MW) * 4
                        + (long)(W * H * TOTAL_CO * TOTAL_B * MH * MW) * 4;
    double exec_time = ExecTime();
    roofline(mem_bandwidth, compute_roof, number_ops, number_bytes, exec_time);
    if (fopen("roofline.png", "r") == NULL) {
        cout << "Failed to draw roofline!\n";
        return 1;
    }
#endif

    printf("Success\n");
    return 0;
}

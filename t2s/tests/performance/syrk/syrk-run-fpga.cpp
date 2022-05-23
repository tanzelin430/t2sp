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
// The header file generated by syrk.cpp
#include "syrk-interface.h"

// Constant parameters (inner loop bounds) of the design
#include "const-parameters.h"

// Roofline utilities
#include "Roofline.h"

// The only header file needed for including T2S.
#include "HalideBuffer.h"

// For printing output
#include <stdio.h>
#include <iostream>

// For validation of results.
#include <assert.h>

using namespace std;

#define MATRIX_OP_IDENTITY 0;
#define MATRIX_OP_TRANSPOSE 1;

int main()
{
    const int TOTAL_I = III * II * I;
    const int TOTAL_J = III * II * I;
    const int TOTAL_K = KKK * KK * K;

    int opa;
    opa = MATRIX_OP_IDENTITY;

    float alpha, beta;
    Halide::Runtime::Buffer<float> a(TOTAL_K, TOTAL_J), cc(TOTAL_J, TOTAL_I);

    alpha = random();
    beta = random();
    for (size_t i = 0; i < TOTAL_I; i++) {
        for (size_t k = 0; k < TOTAL_K; k++) {
            a(k, i) = random();
        }
    }

    for (size_t i = 0; i < TOTAL_I; i++) {
        for (size_t j = 0; j < TOTAL_I; j++) {
            cc(j, i) = random() * random();
        }
    }

    Halide::Runtime::Buffer<float> c(III, III, II, II, I+1, I);
    syrk(opa, alpha, beta, a, cc, c);

#ifdef TINY
    // Validate the results
    for (int i = 0; i < I/2; i++)
    for (int j = 0; j < I+1; j++)
        for (int ii = 0; ii < II; ii++)
        for (int jj = 0; jj < II; jj++)
            for (int iii = 0; iii < III; iii++)
            for (int jjj = 0; jjj < III; jjj++) {
                size_t total_i = iii + III * ii + III * II * i;
                size_t total_j = jjj + III * jj + III * II * j;
                if (total_j >= TOTAL_I + III*II) continue;
                if (total_i < total_j && total_j - total_i < III*II) continue;

                size_t aRow, bRow;
                if (total_i >= total_j) {
                    // bottom
                    // cout << "bot ";
                    aRow = TOTAL_I - total_i - 1;
                    bRow = TOTAL_I - total_j - 1;
                } else {
                    // top
                    // continue;
                    // cout << "top ";
                    aRow = total_i;
                    bRow = total_j - III*II;
                }

                float golden = beta * cc(bRow, aRow);
                for (int k = 0; k < TOTAL_K; k++) {
                    float aa = a(k, aRow);
                    float bb = a(k, bRow);
                    // cout << aa << " " << bb << endl;
                    golden += alpha * aa * bb;
                }

                // cout << i << " " << j << " " << total_i << "\t" << total_j << "\t" << fabs(golden - c(jjj, iii, jj, ii, j, i)) << "\t" << golden << "\t" << c(jjj, iii, jj, ii, j, i) << "\t" << beta * cc(bRow, aRow) << endl;
                assert(fabs(golden - c(jjj, iii, jj, ii, j, i)) < 0.005*fabs(golden));
            }
#else
    // Report performance. DSPs, FMax and ExecTime are automatically figured out from the static analysis
    // during FPGA synthesis and and the dynamic profile during the FGPA execution.
    float mem_bandwidth = 34; // pac_a10 on DevCloud has 34GB/s memory bandwidth
    float compute_roof = 2 * DSPs() * FMax();
    float number_ops = 2 * (float)(III * II * I) * (float)(III * II * I) * (float)(KKK * KK * K); // Total operations (GFLOP for GEMM), independent of designs
    float number_bytes = (float)(KKK * KK * K * III * II * I) * 4 + (float)(III * III * II * II * I * I) * 4;
    float exec_time= ExecTime();
    roofline(mem_bandwidth, compute_roof, number_ops, number_bytes,exec_time);
    if (fopen("roofline.png", "r") == NULL) {
        cout << "Failed to draw roofline!\n";
        return 1;
    }
#endif

    printf("Success\n");
    return 0;
}
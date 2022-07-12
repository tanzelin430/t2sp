
# Change to clang include directory
CWD="$PWD"
cd ${PWD}/sample_01/
# echo "CWD: $PWD"
if [ "$1" == "FPGA" ];
then
echo "Building T2S...";
echo "CMD: g++ post.t2s.test.t2s.cpp -I ${T2S_PATH}/t2s/src/ -I ${T2S_PATH}/t2s/tests/correctness/util -I ${T2S_PATH}/Halide/include -L ${T2S_PATH}/Halide/bin -lz -lpthread -ldl -std=c++11 -lHalide -DTINY -DFPGA_EMULATOR -DFPGA;";
g++ post.t2s.test.t2s.cpp \
    -I ${T2S_PATH}/t2s/src/ \
    -I ${T2S_PATH}/t2s/tests/correctness/util \
    -I ${T2S_PATH}/Halide/include \
    -L ${T2S_PATH}/Halide/bin \
    -lz -lpthread -ldl -std=c++11 -lHalide -DTINY -DFPGA_EMULATOR -DFPGA;

echo "Executing T2S...";
./a.out;

echo "Building OneAPI...";
echo "CMD: dpcpp post.run.test.t2s.cpp -I ${T2S_PATH}/Halide/include -L ${T2S_PATH}/Halide/bin -lHalide -lz -lpthread -ldl -fintelfpga -fsycl -fsycl-device-code-split=off -DTINY -DFPGA_EMULATOR -DFPGA -o ./test.fpga_emu;";
dpcpp post.run.test.t2s.cpp \
    -I ${T2S_PATH}/Halide/include \
    -L ${T2S_PATH}/Halide/bin \
    -lHalide -lz -lpthread -ldl \
    -fintelfpga -fsycl -fsycl-device-code-split=off \
    -DTINY -DFPGA_EMULATOR -DFPGA \
    -o ./test.fpga_emu;

echo "Executing final binary...";
./test.fpga_emu;

# Return back to the directory
# cd ${CWD}
echo "CWD: $PWD"
elif [ "$1" == "GPU" ];
then
echo "Building T2S for Intel GPU...";
echo "CMD: g++ post.t2s.test.t2s.gpu.cpp -I ${T2S_PATH}/t2s/src/ -I ${T2S_PATH}/t2s/tests/correctness/util -I ${T2S_PATH}/Halide/include -L ${T2S_PATH}/Halide/bin -lz -lpthread -ldl -std=c++11 -lHalide -DTINY -DGPU;";
g++ post.t2s.test.t2s.gpu.cpp \
    -I ${T2S_PATH}/t2s/src/ \
    -I ${T2S_PATH}/t2s/tests/correctness/util \
    -I ${T2S_PATH}/Halide/include \
    -L ${T2S_PATH}/Halide/bin \
    -lz -lpthread -ldl -std=c++11 -lHalide -DTINY -DGPU;

echo "Executing T2S...";
HL_DEBUG_CODEGEN=1 ./a.out;
#format the file
clang-format -style=LLVM -i *_genx.cpp 
# Return back to the directory
# cd ${CWD}
echo "CWD: $PWD"
fi
#!/usr/bin/env python
# coding=utf-8
# Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


import os

# Make sure that $BIN_DIR envvar points to the folder containing this script.
# It allows "$BIN_DIR" to be used in paths to benchmarks so that this cript
# can be run from anywhere.
bin_dir = os.path.dirname(os.path.realpath(__file__))
os.environ["BIN_DIR"] = bin_dir

#List of test benchmarks.
# Benchmarks: 21
RODINIA="backprop bfs b+tree cfd dwt2d gaussian heartwall hotspot hotspot3D "
RODINIA+="hybridsort kmeans lavaMD leukocyte lud myocyte nn nw "
RODINIA+="particlefilter pathfinder srad streamcluster"

# Benchmarks: 14
SHOC="DeviceMemory MaxFlops BFS FFT GEMM MD md5hash Reduction Sort Scan Spmv "
SHOC+="Stencil2D Triad S3D"

# Benchmarks: 4
PHORONIX="juliaGPU mandelbulbGPU smallptGPU MandelGPU"

# Benchmarks: 7
OPENDWARFS="bwa_hmm crc csr nqueens swat tdm gem"
#errored on swat

# Benchmarks: 6
PROXYAPPS="CoMD CoMD-LJ lulesh XSBench SNAP SNAP_MPI"

# Benchmarks: 40
AMDAPP="AdvancedConvolution BlackScholes BlackScholesDP BinomialOption "
AMDAPP+="BitonicSort BoxFilter BufferBandwidth BufferImageInterop DCT "
AMDAPP+="DwtHaar1D EigenValue FastWalshTransform FloydWarshall "
AMDAPP+="FluidSimulation2D GaussianNoise HDRToneMapping Histogram "
AMDAPP+="ImageBandwidth ImageBinarization ImageOverlap KmeansAutoClustering "
AMDAPP+="Mandelbrot MatrixMultiplication MatrixMulDouble MatrixTranspose "
AMDAPP+="MersenneTwister MonteCarloAsian MonteCarloAsianDP NBody PrefixSum "
AMDAPP+="QuasiRandomSequence RadixSort RecursiveGaussian Reduction-AMD "
AMDAPP+="ScanLargeArrays SimpleConvolution SobelFilter StringSearch "
AMDAPP+="UnsharpMask URNG"

# Benchmarks: 6
AMDAPP_CL2="BinarySearchDeviceSideEnqueue CalcPie DeviceEnqueueBFS "
AMDAPP_CL2+="ExtractPrimes RangeMinimumQuery SVMBinaryTreeSearch"

# Benchmarks: 11
PARBOIL="pb-bfs stencil mri-gridding lbm sad histo mri-q cutcp pb-sgemm "
PARBOIL=="pb-spmv tpacf"
#mri-gridding has an overflow by default

# Benchmarks: 6
PANNOTIA="bc color fw mis prk sssp"

# Benchmarks: 4
STREAMMR="mr-kmeans mr-matrixmul stringmatch wordcount"
#mr-kmeans overflow, non det
#wordcount overflow, non det

# Benchmarks: 21
POLYBENCH="correlation covariance 2mm 3mm atax bicg doitgen poly-gemm gemver "
POLYBENCH+="gesummv gramschmidt mvt syr2k syrk lu adi convolution-2d "
POLYBENCH+="convolution-3d fdtd-2d jacobi-1d-imper jacobi-2d-imper"

# Benchmarks: 2
FINANCEBENCH="FB-Black-Scholes FB-Monte-Carlo"

# Benchmarks: 1
GPUSTREAM="gpustream"

# Benchmarks: 4
MANTEVO="cloverleaf cloverleaf3d tealeaf tealeaf3d"

# Benchmarks: 7
HETEROMARK="aes_hm fir_hm hmm_hm iir_hm kmeans_hm pagerank_hm sw_hm"
#kmeans_hm overflow
#sw_hm overflow

# Benchmarks: 7
HETEROMARK_CL2="aes_hm_cl2 fir_hm_cl2 hmm_hm_cl2 iir_hm_cl2 kmeans_hm_cl2 "
HETEROMARK_CL2+="pagerank_hm_cl2 sw_hm_cl2"

# Benchmarks: 1
LINPACK="hpl"

# Benchmarks: 8
VIENNACL="dense_blas-bench-opencl amg bisect custom-kernels matrix-free nmf "
VIENNACL+="blas3_solve-test-opencl sparse_prod-test-opencl"

# Benchmarks: 8
NPB_OCL="npb_ocl_bt npb_ocl_cg npb_ocl_ep npb_ocl_ft npb_ocl_is npb_ocl_lu "
NPB_OCL+="npb_ocl_mg npb_ocl_sp"

ALL_BENCHMARKS=""
for bench in [RODINIA, SHOC, PHORONIX, OPENDWARFS, PROXYAPPS, AMDAPP,
    AMDAPP_CL2, PARBOIL, PANNOTIA, STREAMMR, POLYBENCH, FINANCEBENCH,
    GPUSTREAM, MANTEVO, HETEROMARK, HETEROMARK_CL2, LINPACK, VIENNACL,
    NPB_OCL]:
    ALL_BENCHMARKS+=bench + " "

#Declare HashMaps
benchCD = {}
benchPrefix = {}
benchCMD = {}

#echo $codexl
path_to_benchmark=os.path.abspath(os.path.expanduser("~/benchmarks"))

#----------------------------------RODINIA-------------------------------------
benchCD["backprop"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/backprop")
benchCMD["backprop"]="backprop 8388608"
#
benchCD["bfs"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/bfs")
benchCMD["bfs"]="bfs ../../data/bfs/graph1MW_6.txt"
#
benchCD["b+tree"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/b+tree")
benchCMD["b+tree"]="b+tree.out file ../../data/b+tree/mil.txt command ../../data/b+tree/command.txt"
#
benchCD["cfd"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/cfd")
benchCMD["cfd"]="euler3d ../../data/cfd/fvcorr.domn.193K -t gpu -d 0"
#
benchCD["dwt2d"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/dwt2d")
benchCMD["dwt2d"]="dwt2d rgb.bmp -d 1024x1024 -f 5 -l 3"
#
benchCD["gaussian"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/gaussian")
benchCMD["gaussian"]="gaussian -s 256 -p 0 -d 0 > /dev/null"
#
benchCD["heartwall"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/heartwall")
benchCMD["heartwall"]="heartwall 20"
#
benchCD["hotspot"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/hotspot")
benchCMD["hotspot"]="hotspot 1024 2 2 ../../data/hotspot/temp_1024 ../../data/hotspot/power_1024 output.out"
#
benchCD["hotspot3D"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/hotspot3D")
benchCMD["hotspot3D"]="3D 512 8 100 ../../data/hotspot3D/power_512x8 ../../data/hotspot3D/temp_512x8 output.out"
#
benchCD["hybridsort"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/hybridsort")
benchCMD["hybridsort"]="hybridsort r"
#
benchCD["kmeans"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/kmeans")
benchCMD["kmeans"]="kmeans -o -i ../../data/kmeans/819200.txt"
#
benchCD["lavaMD"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/lavaMD")
benchCMD["lavaMD"]="lavaMD -boxes1d 16"
#
benchCD["leukocyte"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/leukocyte/OpenCL")
benchCMD["leukocyte"]="leukocyte ../../../data/leukocyte/testfile.avi 1"
#
benchCD["lud"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/lud/ocl")
benchCMD["lud"]="lud -i ../../../data/lud/2048.dat"
#
benchCD["myocyte"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/myocyte")
benchCMD["myocyte"]="myocyte.out -time 5"
#
benchCD["nn"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/nn")
benchCMD["nn"]="nn filelist.txt -r 5 -lat 30 -lng 90 -p 0 -d 0"
#
benchCD["nw"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/nw")
benchCMD["nw"]="nw 2048 10 ./nw.cl"
#
benchCD["particlefilter"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/particlefilter")
benchCMD["particlefilter"]="OCL_particlefilter_single -x 128 -y 128 -z 2 -np 400000"
#
benchCD["pathfinder"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/pathfinder")
benchCMD["pathfinder"]="pathfinder 100000 100 20 > /dev/null"
#
benchCD["streamcluster"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/streamcluster")
benchCMD["streamcluster"]="streamcluster 10 20 256 65536 65536 1000 none output.txt 1 -t gpu"
#
benchCD["srad"]=os.path.join(path_to_benchmark, "rodinia_3.1/opencl/srad")
benchCMD["srad"]="srad 1 0.5 5020 4580"
#------------------------------------------------------------------------------


#----------------------------------SHOC----------------------------------------
benchCD["DeviceMemory"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level0")
benchCMD["DeviceMemory"]="DeviceMemory -n 1"
#
benchCD["MaxFlops"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level0")
benchCMD["MaxFlops"]="MaxFlops -n 1"
#
benchCD["BFS"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level1/bfs")
benchCMD["BFS"]="BFS --graph_file ldoor.graph -n 1 --algo 2"
#
benchCD["FFT"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level1/fft")
benchCMD["FFT"]="FFT --MB 512 -n 1"
#
benchCD["GEMM"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level1/gemm")
benchCMD["GEMM"]="GEMM --KiB 16 -n 1"
#
benchCD["MD"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level1/md")
benchCMD["MD"]="MD -s 2 -n 1"
#
benchCD["md5hash"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level1/md5hash")
benchCMD["md5hash"]="MD5Hash -s 4 -n 1"
#
benchCD["Reduction"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level1/reduction")
benchCMD["Reduction"]="Reduction -s 2 -n 1"
#
benchCD["Scan"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level1/scan")
benchCMD["Scan"]="Scan -s 4 -n 1"
#
benchCD["Sort"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level1/sort")
benchCMD["Sort"]="Sort -s 4 -n 1"
#
benchCD["Spmv"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level1/spmv")
benchCMD["Spmv"]="Spmv -n 1 --iterations 10 --mm_filename pdb1HYS.mtx"
#
benchCD["Stencil2D"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level1/stencil2d")
benchCMD["Stencil2D"]="Stencil2D -n 1 --num-iters 5"
#
benchCD["Triad"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level1/triad")
benchCMD["Triad"]="Triad -n 1"
#
benchCD["S3D"]=os.path.join(path_to_benchmark, "shoc/src/opencl/level2/s3d")
benchCMD["S3D"]="S3D --size 4 -n 1"
#------------------------------------------------------------------------------


#----------------------------------PHORONIX------------------------------------
benchCD["juliaGPU"]=os.path.join(path_to_benchmark, "phoronix/JuliaGPU-v1.2pts")
benchPrefix["juliaGPU"]="DISPLAY=:0 LD_LIBRARY_PATH=/opt/AMDAPP/lib/x86_64/:$LD_LIBRARY_PATH "
benchCMD["juliaGPU"]="juliaGPU 0 1 preprocessed_rendering_kernel.cl 1024 768"
#
benchCD["mandelbulbGPU"]=os.path.join(path_to_benchmark, "phoronix/mandelbulbGPU-v1.0pts")
benchPrefix["mandelbulbGPU"]="DISPLAY=:0 LD_LIBRARY_PATH=/opt/AMDAPP/lib/x86_64/:$LD_LIBRARY_PATH "
benchCMD["mandelbulbGPU"]="mandelbulbGPU"
#
benchCD["smallptGPU"]=os.path.join(path_to_benchmark, "phoronix/SmallptGPU-v1.6pts")
benchPrefix["smallptGPU"]="DISPLAY=:0 LD_LIBRARY_PATH=/opt/AMDAPP/lib/x86_64/:$LD_LIBRARY_PATH "
benchCMD["smallptGPU"]="smallptGPU 1 0 rendering_kernel.cl 1920 1080 ./scenes/cornell_large.scn"
#
benchCD["MandelGPU"]=os.path.join(path_to_benchmark, "phoronix/MandelGPU-v1.3pts")
benchPrefix["MandelGPU"]="DISPLAY=:0 LD_LIBRARY_PATH=/opt/AMDAPP/lib/x86_64/:$LD_LIBRARY_PATH "
benchCMD["MandelGPU"]="mandelGPU 0 1 ./preprocessed_rendering_kernel_float4.cl 3840 2160 500"
#------------------------------------------------------------------------------


#----------------------------------OPENDWARFS----------------------------------
benchCD["bwa_hmm"]=os.path.join(path_to_benchmark, "OpenDwarfs/build")
benchCMD["bwa_hmm"]="bwa_hmm -t 50 -v t"
#
benchCD["crc"]=os.path.join(path_to_benchmark, "OpenDwarfs/build")
benchCMD["crc"]="crc -i ./crcfile_N256_S128K"
#
benchCD["csr"]=os.path.join(path_to_benchmark, "OpenDwarfs/build")
benchCMD["csr"]="csr -i ./csr_n131072_d1000_s01"
#
benchCD["nqueens"]=os.path.join(path_to_benchmark, "OpenDwarfs/build")
benchCMD["nqueens"]="nqueens 18"
#
benchCD["swat"]=os.path.join(path_to_benchmark, "OpenDwarfs/build")
benchCMD["swat"]="swat ../test/dynamic-programming/swat/query5K1 ../test/dynamic-programming/swat/sampledb5K1 10.0 0.5 USE_NUM_ACTIVE_CUS"
#
benchCD["tdm"]=os.path.join(path_to_benchmark, "OpenDwarfs/build")
benchCMD["tdm"]="tdm ../test/finite-state-machine/tdm/sim-64-size500.csv ../test/finite-state-machine/tdm/ivl.txt ../test/finite-state-machine/tdm/30-episodes.txt 128"
#
benchCD["gem"]=os.path.join(path_to_benchmark, "OpenDwarfs/build")
benchCMD["gem"]="gemnoui ../test/n-body-methods/gem/nucleosome 80 1 0 -dumpVertices phi.out"
#------------------------------------------------------------------------------


#----------------------------------PROXYAPPS-----------------------------------
benchCD["CoMD"]=os.path.join(path_to_benchmark, "proxyapps/CoMD")
benchCMD["CoMD"]="CoMD-ocl -x 40 -y 40 -z 40 -g -e -N 1"
#
#Note this is a special case where the benchmark is the same but with different command-line.
benchCD["CoMD-LJ"]=os.path.join(path_to_benchmark, "proxyapps/CoMD")
benchCMD["CoMD-LJ"]="CoMD-ocl -x 40 -y 40 -z 40 -g -N 1"
#
benchCD["lulesh"]=os.path.join(path_to_benchmark, "proxyapps/LULESH")
benchCMD["lulesh"]="lulesh -s 100 -i 1"
#
benchCD["XSBench"]=os.path.join(path_to_benchmark, "proxyapps/XSBench/src")
benchCMD["XSBench"]="XSBench -s small"
#
benchCD["SNAP"]=os.path.join(path_to_benchmark, "proxyapps/SNAP-OpenCL/src")
benchCMD["SNAP"]="snap ./snap-16.in ./bench.out"
#
benchCD["SNAP_MPI"]=os.path.join(path_to_benchmark, "proxyapps/SNAP_MPI_OpenCL/src")
benchCMD["SNAP_MPI"]="snap ./snap-16.in"
#------------------------------------------------------------------------------


#----------------------------------AMDAPP--------------------------------------
benchCD["AdvancedConvolution"]=os.path.join(path_to_benchmark, "AMDAPP/AdvancedConvolution/bin/x86_64/Release")
benchCMD["AdvancedConvolution"]="AdvancedConvolution -m 5 -q"
#
benchCD["BlackScholes"]=os.path.join(path_to_benchmark, "AMDAPP/BlackScholes/bin/x86_64/Release")
benchCMD["BlackScholes"]="BlackScholes -x 67108864 -q"
#
benchCD["BlackScholesDP"]=os.path.join(path_to_benchmark, "AMDAPP/BlackScholesDP/bin/x86_64/Release")
benchCMD["BlackScholesDP"]="BlackScholesDP -x 67108864 -q"
#
benchCD["BinomialOption"]=os.path.join(path_to_benchmark, "AMDAPP/BinomialOption/bin/x86_64/Release")
benchCMD["BinomialOption"]="BinomialOption -x 1048576 -q"
#
benchCD["BitonicSort"]=os.path.join(path_to_benchmark, "AMDAPP/BitonicSort/bin/x86_64/Release")
benchCMD["BitonicSort"]="BitonicSort -x 2097152 -q"
#
benchCD["BoxFilter"]=os.path.join(path_to_benchmark, "AMDAPP/BoxFilter/bin/x86_64/Release")
benchCMD["BoxFilter"]="BoxFilter -q"
#
benchCD["BufferBandwidth"]=os.path.join(path_to_benchmark, "AMDAPP/BufferBandwidth/bin/x86_64/Release")
benchCMD["BufferBandwidth"]="BufferBandwidth"
#
benchCD["BufferImageInterop"]=os.path.join(path_to_benchmark, "AMDAPP/BufferImageInterop/bin/x86_64/Release")
benchCMD["BufferImageInterop"]="BufferImageInterop"
#
benchCD["DCT"]=os.path.join(path_to_benchmark, "AMDAPP/DCT/bin/x86_64/Release")
benchCMD["DCT"]="DCT -x 16384 -y 16384 -q"
#
benchCD["DwtHaar1D"]=os.path.join(path_to_benchmark, "AMDAPP/DwtHaar1D/bin/x86_64/Release")
benchCMD["DwtHaar1D"]="DwtHaar1D -x 8388608 -q"
#
benchCD["EigenValue"]=os.path.join(path_to_benchmark, "AMDAPP/EigenValue/bin/x86_64/Release")
benchCMD["EigenValue"]="EigenValue -x 131072 -q"
#
benchCD["FastWalshTransform"]=os.path.join(path_to_benchmark, "AMDAPP/FastWalshTransform/bin/x86_64/Release")
benchCMD["FastWalshTransform"]="FastWalshTransform -x 33554432 -q"
#
benchCD["FloydWarshall"]=os.path.join(path_to_benchmark, "AMDAPP/FloydWarshall/bin/x86_64/Release")
benchCMD["FloydWarshall"]="FloydWarshall -x 8192 -q"
#
benchCD["FluidSimulation2D"]=os.path.join(path_to_benchmark, "AMDAPP/FluidSimulation2D/bin/x86_64/Release")
benchCMD["FluidSimulation2D"]="FluidSimulation2D -q -i 5"
#
benchCD["GaussianNoise"]=os.path.join(path_to_benchmark, "AMDAPP/GaussianNoise/bin/x86_64/Release")
benchCMD["GaussianNoise"]="GaussianNoise -q"
#
benchCD["HDRToneMapping"]=os.path.join(path_to_benchmark, "AMDAPP/HDRToneMapping/bin/x86_64/Release")
benchCMD["HDRToneMapping"]="HDRToneMapping"
#
benchCD["Histogram"]=os.path.join(path_to_benchmark, "AMDAPP/Histogram/bin/x86_64/Release")
benchCMD["Histogram"]="Histogram -q -x 2048 -y 2048"
#
benchCD["ImageBandwidth"]=os.path.join(path_to_benchmark, "AMDAPP/ImageBandwidth/bin/x86_64/Release")
benchCMD["ImageBandwidth"]="ImageBandwidth"
#
benchCD["ImageBinarization"]=os.path.join(path_to_benchmark, "AMDAPP/ImageBinarization/bin/x86_64/Release")
benchCMD["ImageBinarization"]="ImageBinarization"
#
benchCD["ImageOverlap"]=os.path.join(path_to_benchmark, "AMDAPP/ImageOverlap/bin/x86_64/Release")
benchCMD["ImageOverlap"]="ImageOverlap -q"
#
benchCD["KmeansAutoClustering"]=os.path.join(path_to_benchmark, "AMDAPP/KmeansAutoclustering/bin/x86_64/Release")
benchCMD["KmeansAutoClustering"]="KmeansAutoclustering -x 65536 -k 16 -q -i 1"
#
benchCD["Mandelbrot"]=os.path.join(path_to_benchmark, "AMDAPP/Mandelbrot/bin/x86_64/Release")
benchCMD["Mandelbrot"]="Mandelbrot -q -i 2 -xs 1024"
#
benchCD["MatrixMultiplication"]=os.path.join(path_to_benchmark, "AMDAPP/MatrixMultiplication/bin/x86_64/Release")
benchCMD["MatrixMultiplication"]="MatrixMultiplication -q -x 4096 -y 4096 -z 4096 -b 16"
#
benchCD["MatrixMulDouble"]=os.path.join(path_to_benchmark, "AMDAPP/MatrixMulDouble/bin/x86_64/Release")
benchCMD["MatrixMulDouble"]="MatrixMulDouble -q -x 4096 -y 4096 -z 4096 -i 10"
#
benchCD["MatrixTranspose"]=os.path.join(path_to_benchmark, "AMDAPP/MatrixTranspose/bin/x86_64/Release")
benchCMD["MatrixTranspose"]="MatrixTranspose -q -x 4096 -b 16"
#
benchCD["MersenneTwister"]=os.path.join(path_to_benchmark, "AMDAPP/MersenneTwister/bin/x86_64/Release")
benchCMD["MersenneTwister"]="MersenneTwister -x 33554432 -q"
#
benchCD["MonteCarloAsian"]=os.path.join(path_to_benchmark, "AMDAPP/MonteCarloAsian/bin/x86_64/Release")
benchCMD["MonteCarloAsian"]="MonteCarloAsian -q"
#
benchCD["MonteCarloAsianDP"]=os.path.join(path_to_benchmark, "AMDAPP/MonteCarloAsianDP/bin/x86_64/Release")
benchCMD["MonteCarloAsianDP"]="MonteCarloAsianDP -q"
#
benchCD["NBody"]=os.path.join(path_to_benchmark, "AMDAPP/NBody/bin/x86_64/Release")
benchCMD["NBody"]="NBody -q -i 10 -t -x 262144"
#
benchCD["PrefixSum"]=os.path.join(path_to_benchmark, "AMDAPP/PrefixSum/bin/x86_64/Release")
benchCMD["PrefixSum"]="PrefixSum -q -x 1048576"
#
benchCD["QuasiRandomSequence"]=os.path.join(path_to_benchmark, "AMDAPP/QuasiRandomSequence/bin/x86_64/Release")
benchCMD["QuasiRandomSequence"]="QuasiRandomSequence -q -x 8192 -y 8192"
#
benchCD["RadixSort"]=os.path.join(path_to_benchmark, "AMDAPP/RadixSort/bin/x86_64/Release")
benchCMD["RadixSort"]="RadixSort -q -x 262144"
#
benchCD["RecursiveGaussian"]=os.path.join(path_to_benchmark, "AMDAPP/RecursiveGaussian/bin/x86_64/Release")
benchCMD["RecursiveGaussian"]="RecursiveGaussian -q"
#
benchCD["Reduction-AMD"]=os.path.join(path_to_benchmark, "AMDAPP/Reduction/bin/x86_64/Release")
benchCMD["Reduction-AMD"]="Reduction -q -x 1048576"
#
benchCD["ScanLargeArrays"]=os.path.join(path_to_benchmark, "AMDAPP/ScanLargeArrays/bin/x86_64/Release")
benchCMD["ScanLargeArrays"]="ScanLargeArrays -q -x 16777216"
#
benchCD["SimpleConvolution"]=os.path.join(path_to_benchmark, "AMDAPP/SimpleConvolution/bin/x86_64/Release")
benchCMD["SimpleConvolution"]="SimpleConvolution -q -x 4096 -y 4096"
#
benchCD["SobelFilter"]=os.path.join(path_to_benchmark, "AMDAPP/SobelFilter/bin/x86_64/Release")
benchCMD["SobelFilter"]="SobelFilter -q"
#
benchCD["StringSearch"]=os.path.join(path_to_benchmark, "AMDAPP/StringSearch/bin/x86_64/Release")
benchCMD["StringSearch"]="StringSearch -q"
#
benchCD["UnsharpMask"]=os.path.join(path_to_benchmark, "AMDAPP/UnsharpMask/bin/x86_64/Release")
benchCMD["UnsharpMask"]="UnsharpMask -q -i 1"
#
benchCD["URNG"]=os.path.join(path_to_benchmark, "AMDAPP/URNG/bin/x86_64/Release")
benchCMD["URNG"]="URNG -f 1024 -q"
#------------------------------------------------------------------------------


#----------------------------AMDAPP OPENCL 2.0---------------------------------
benchCD["BinarySearchDeviceSideEnqueue"]=os.path.join(path_to_benchmark, "AMDAPP/BinarySearchDeviceSideEnqueue/bin/x86_64/Release")
benchCMD["BinarySearchDeviceSideEnqueue"]="BinarySearchDeviceSideEnqueue -q -eq 1 "
#
benchCD["CalcPie"]=os.path.join(path_to_benchmark, "AMDAPP/CalcPie/bin/x86_64/Release")
benchCMD["CalcPie"]="CalcPie -x 50000000"
#
benchCD["DeviceEnqueueBFS"]=os.path.join(path_to_benchmark, "AMDAPP/DeviceEnqueueBFS/bin/x86_64/Release")
benchCMD["DeviceEnqueueBFS"]="DeviceEnqueueBFS -q -n 16384"
#
benchCD["ExtractPrimes"]=os.path.join(path_to_benchmark, "AMDAPP/ExtractPrimes/bin/x86_64/Release")
benchCMD["ExtractPrimes"]="ExtractPrimes -eq 0 -q -x 268435456"
#
benchCD["RangeMinimumQuery"]=os.path.join(path_to_benchmark, "AMDAPP/RangeMinimumQuery/bin/x86_64/Release")
benchCMD["RangeMinimumQuery"]="RangeMinimumQuery -x 200000000"
#
benchCD["SVMBinaryTreeSearch"]=os.path.join(path_to_benchmark, "AMDAPP/SVMBinaryTreeSearch/bin/x86_64/Release")
benchCMD["SVMBinaryTreeSearch"]="SVMBinaryTreeSearch -s 1 -n 200000 -k 5000000"
#------------------------------------------------------------------------------


#-----------------------------------PARBOIL------------------------------------
benchCD["pb-bfs"]=os.path.join(path_to_benchmark, "parboil/benchmarks/bfs")
benchCMD["pb-bfs"]="build/opencl_base_default/bfs -i ../../datasets/bfs/NY/input/graph_input.dat -o ./blah.out --"
#
benchCD["stencil"]=os.path.join(path_to_benchmark, "parboil/benchmarks/stencil")
benchCMD["stencil"]="build/opencl_base_default/stencil -i ../../datasets/stencil/default/input/512x512x64x100.bin -o ./512x512x64.out -- 512 512 64 100"
#
benchCD["mri-gridding"]=os.path.join(path_to_benchmark, "parboil/benchmarks/mri-gridding")
benchCMD["mri-gridding"]="build/opencl_base_default/mri-gridding -i ../../datasets/mri-gridding/small/input/small.uks -o ./output.txt -- 32 0"
#
benchCD["lbm"]=os.path.join(path_to_benchmark, "parboil/benchmarks/lbm")
benchCMD["lbm"]="build/opencl_base_default/lbm -i ../../datasets/lbm/short/input/120_120_150_ldc.of -o ./reference.dat -- 10"
#
benchCD["sad"]=os.path.join(path_to_benchmark, "parboil/benchmarks/sad")
benchCMD["sad"]="build/opencl_base_default/sad -i ../../datasets/sad/large/input/reference.bin,../../datasets/sad/large/input/frame.bin -o ./out.bin"
#
benchCD["histo"]=os.path.join(path_to_benchmark, "parboil/benchmarks/histo")
benchCMD["histo"]="build/opencl_base_default/histo -i ../../datasets/histo/large/input/img.bin -o ./ref.bmp -- 10 4"
#
benchCD["mri-q"]=os.path.join(path_to_benchmark, "parboil/benchmarks/mri-q")
benchCMD["mri-q"]="build/opencl_default/mri-q -i ../../datasets/mri-q/large/input/64_64_64_dataset.bin -o ./64_64_64_dataset.out"
#
benchCD["cutcp"]=os.path.join(path_to_benchmark, "parboil/benchmarks/cutcp")
benchCMD["cutcp"]="build/opencl_base_default/cutcp -i ../../datasets/cutcp/large/input/watbox.sl100.pqr -o ./lattice.dat"
#
benchCD["pb-sgemm"]=os.path.join(path_to_benchmark, "parboil/benchmarks/sgemm")
benchCMD["pb-sgemm"]="build/opencl_base_default/sgemm -i ../../datasets/sgemm/medium/input/matrix1.txt,../../datasets/sgemm/medium/input/matrix2t.txt,../..//datasets/sgemm/medium/input/matrix2t.txt -o ./matrix3.txt"
#
benchCD["pb-spmv"]=os.path.join(path_to_benchmark, "parboil/benchmarks/spmv")
benchCMD["pb-spmv"]="build/opencl_base_default/spmv -i ../../datasets/spmv/large/input/Dubcova3.mtx.bin,../../datasets/spmv/large/input/vector.bin -o ./Dubcova3.mtx.out"
#
benchCD["tpacf"]=os.path.join(path_to_benchmark, "parboil/benchmarks/tpacf")
benchCMD["tpacf"]="build/opencl_base_default/tpacf -i ../../datasets/tpacf/large/input/Datapnts.1,../../datasets/tpacf/large/input/Randompnts.1,../../datasets/tpacf/large/input/Randompnts.2,../../datasets/tpacf/large/input/Randompnts.3,../../datasets/tpacf/large/input/Randompnts.4,../../datasets/tpacf/large/input/Randompnts.5,../../datasets/tpacf/large/input/Randompnts.6,../../datasets/tpacf/large/input/Randompnts.7,../../datasets/tpacf/large/input/Randompnts.8,../../datasets/tpacf/large/input/Randompnts.9,../../datasets/tpacf/large/input/Randompnts.10,../../datasets/tpacf/large/input/Randompnts.11,../../datasets/tpacf/large/input/Randompnts.12,../../datasets/tpacf/large/input/Randompnts.13,../../datasets/tpacf/large/input/Randompnts.14,../../datasets/tpacf/large/input/Randompnts.15,../../datasets/tpacf/large/input/Randompnts.16,../../datasets/tpacf/large/input/Randompnts.17,../../datasets/tpacf/large/input/Randompnts.18,../../datasets/tpacf/large/input/Randompnts.19,../../datasets/tpacf/large/input/Randompnts.20,../../datasets/tpacf/large/input/Randompnts.21,../../datasets/tpacf/large/input/Randompnts.22,../../datasets/tpacf/large/input/Randompnts.23,../../datasets/tpacf/large/input/Randompnts.24,../../datasets/tpacf/large/input/Randompnts.25,../../datasets/tpacf/large/input/Randompnts.26,../../datasets/tpacf/large/input/Randompnts.27,../../datasets/tpacf/large/input/Randompnts.28,../../datasets/tpacf/large/input/Randompnts.29,../../datasets/tpacf/large/input/Randompnts.30,../../datasets/tpacf/large/input/Randompnts.31,../../datasets/tpacf/large/input/Randompnts.32,../../datasets/tpacf/large/input/Randompnts.33,../../datasets/tpacf/large/input/Randompnts.34,../../datasets/tpacf/large/input/Randompnts.35,../../datasets/tpacf/large/input/Randompnts.36,../../datasets/tpacf/large/input/Randompnts.37,../../datasets/tpacf/large/input/Randompnts.38,../../datasets/tpacf/large/input/Randompnts.39,../../datasets/tpacf/large/input/Randompnts.40,../../datasets/tpacf/large/input/Randompnts.41,../../datasets/tpacf/large/input/Randompnts.42,../../datasets/tpacf/large/input/Randompnts.43,../../datasets/tpacf/large/input/Randompnts.44,../../datasets/tpacf/large/input/Randompnts.45,../../datasets/tpacf/large/input/Randompnts.46,../../datasets/tpacf/large/input/Randompnts.47,../../datasets/tpacf/large/input/Randompnts.48,../../datasets/tpacf/large/input/Randompnts.49,../../datasets/tpacf/large/input/Randompnts.50,../../datasets/tpacf/large/input/Randompnts.51,../../datasets/tpacf/large/input/Randompnts.52,../../datasets/tpacf/large/input/Randompnts.53,../../datasets/tpacf/large/input/Randompnts.54,../../datasets/tpacf/large/input/Randompnts.55,../../datasets/tpacf/large/input/Randompnts.56,../../datasets/tpacf/large/input/Randompnts.57,../../datasets/tpacf/large/input/Randompnts.58,../../datasets/tpacf/large/input/Randompnts.59,../../datasets/tpacf/large/input/Randompnts.60,../../datasets/tpacf/large/input/Randompnts.61,../../datasets/tpacf/large/input/Randompnts.62,../../datasets/tpacf/large/input/Randompnts.63,../../datasets/tpacf/large/input/Randompnts.64,../../datasets/tpacf/large/input/Randompnts.65,../../datasets/tpacf/large/input/Randompnts.66,../../datasets/tpacf/large/input/Randompnts.67,../../datasets/tpacf/large/input/Randompnts.68,../../datasets/tpacf/large/input/Randompnts.69,../../datasets/tpacf/large/input/Randompnts.70,../../datasets/tpacf/large/input/Randompnts.71,../../datasets/tpacf/large/input/Randompnts.72,../../datasets/tpacf/large/input/Randompnts.73,../../datasets/tpacf/large/input/Randompnts.74,../../datasets/tpacf/large/input/Randompnts.75,../../datasets/tpacf/large/input/Randompnts.76,../../datasets/tpacf/large/input/Randompnts.77,../../datasets/tpacf/large/input/Randompnts.78,../../datasets/tpacf/large/input/Randompnts.79,../../datasets/tpacf/large/input/Randompnts.80,../../datasets/tpacf/large/input/Randompnts.81,../../datasets/tpacf/large/input/Randompnts.82,../../datasets/tpacf/large/input/Randompnts.83,../../datasets/tpacf/large/input/Randompnts.84,../../datasets/tpacf/large/input/Randompnts.85,../../datasets/tpacf/large/input/Randompnts.86,../../datasets/tpacf/large/input/Randompnts.87,../../datasets/tpacf/large/input/Randompnts.88,../../datasets/tpacf/large/input/Randompnts.89,../../datasets/tpacf/large/input/Randompnts.90,../../datasets/tpacf/large/input/Randompnts.91,../../datasets/tpacf/large/input/Randompnts.92,../../datasets/tpacf/large/input/Randompnts.93,../../datasets/tpacf/large/input/Randompnts.94,../../datasets/tpacf/large/input/Randompnts.95,../../datasets/tpacf/large/input/Randompnts.96,../../datasets/tpacf/large/input/Randompnts.97,../../datasets/tpacf/large/input/Randompnts.98,../../datasets/tpacf/large/input/Randompnts.99,../../datasets/tpacf/large/input/Randompnts.100 -o ./tpacf.out -- -n 100 -p 10391"
#
##-----------------------------------------------------------------------------


#----------------------------------PANNOTIA------------------------------------
benchCD["bc"]=os.path.join(path_to_benchmark, "pannotia")
benchCMD["bc"]="graph_app/bc/bc ./dataset/bc/1k_128k.gr ./graph_app/bc/kernel/kernel.cl 0"
#
benchCD["color"]=os.path.join(path_to_benchmark, "pannotia")
benchCMD["color"]="graph_app/color/coloring_max ./dataset/color/G3_circuit.graph ./graph_app/color/kernel/kernel_max.cl 1"
#
benchCD["fw"]=os.path.join(path_to_benchmark, "pannotia")
benchCMD["fw"]="graph_app/fw/floyd-warshall ./dataset/floydwarshall/512_65536.gr ./graph_app/fw/kernel/kernel.cl"
#
benchCD["mis"]=os.path.join(path_to_benchmark, "pannotia")
benchCMD["mis"]="graph_app/mis/mis ./dataset/mis/G3_circuit.graph ./graph_app/mis/kernel/kernel.cl 1"
#
benchCD["prk"]=os.path.join(path_to_benchmark, "pannotia")
benchCMD["prk"]="graph_app/prk/pagerank ./dataset/pagerank/coAuthorsDBLP.graph ./graph_app/prk/kernel/kernel.cl 1"
#
benchCD["sssp"]=os.path.join(path_to_benchmark, "pannotia")
benchCMD["sssp"]="graph_app/sssp/sssp_csr ./dataset/sssp/USA-road-d.NY.gr ./graph_app/sssp/kernel/kernel_csr.cl 0"
#
#------------------------------------------------------------------------------


#----------------------------------STREAMMR------------------------------------
benchCD["mr-kmeans"]=os.path.join(path_to_benchmark, "StreamMR/build")
benchCMD["mr-kmeans"]="KMeans -d 2 -p 16777216 -c 128 -g 64"
#
benchCD["mr-matrixmul"]=os.path.join(path_to_benchmark, "StreamMR/build")
benchCMD["mr-matrixmul"]="MatrixMul -c 512 -r 512 -g 64"
#
benchCD["stringmatch"]=os.path.join(path_to_benchmark, "StreamMR/build")
benchCMD["stringmatch"]="StringMatch -f ../test/4300.txt -s 128 -w Troy -g 64"
#
benchCD["wordcount"]=os.path.join(path_to_benchmark, "StreamMR/build")
benchCMD["wordcount"]="WordCount -f ../test/1mil.txt -c 131072 -g 64"
#------------------------------------------------------------------------------


#---------------------------------POLYBENCH------------------------------------
benchCD["correlation"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/datamining/correlation")
benchCMD["correlation"]="correlation.exe"
#
benchCD["covariance"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/datamining/covariance")
benchCMD["covariance"]="covariance.exe"
#
benchCD["2mm"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/kernels/2mm")
benchCMD["2mm"]="2mm.exe"
#
benchCD["3mm"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/kernels/3mm")
benchCMD["3mm"]="3mm.exe"
#
benchCD["atax"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/kernels/atax")
benchCMD["atax"]="atax.exe"
#
benchCD["bicg"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/kernels/bicg")
benchCMD["bicg"]="bicg.exe"
#
benchCD["doitgen"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/kernels/doitgen")
benchCMD["doitgen"]="doitgen.exe"
#
benchCD["poly-gemm"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/kernels/gemm")
benchCMD["poly-gemm"]="gemm.exe"
#
benchCD["gemver"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/kernels/gemver")
benchCMD["gemver"]="gemver.exe"
#
benchCD["gesummv"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/kernels/gesummv")
benchCMD["gesummv"]="gesummv.exe"
#
benchCD["mvt"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/kernels/mvt")
benchCMD["mvt"]="mvt.exe"
#
benchCD["syr2k"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/kernels/syr2k")
benchCMD["syr2k"]="syr2k.exe"
#
benchCD["syrk"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/kernels/syrk")
benchCMD["syrk"]="syrk.exe"
#
benchCD["gramschmidt"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/solvers/gramschmidt")
benchCMD["gramschmidt"]="gramschmidt.exe"
#
benchCD["lu"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/linear-algebra/solvers/lu")
benchCMD["lu"]="lu.exe"
#
benchCD["adi"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/stencils/adi")
benchCMD["adi"]="adi.exe"
#
benchCD["convolution-2d"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/stencils/convolution-2d")
benchCMD["convolution-2d"]="2DConvolution.exe"
#
benchCD["convolution-3d"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/stencils/convolution-3d")
benchCMD["convolution-3d"]="3DConvolution.exe"
#
benchCD["fdtd-2d"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/stencils/fdtd-2d")
benchCMD["fdtd-2d"]="fdtd2d.exe"
#
benchCD["jacobi-1d-imper"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/stencils/jacobi-1d-imper")
benchCMD["jacobi-1d-imper"]="jacobi1D.exe"
#
benchCD["jacobi-2d-imper"]=os.path.join(path_to_benchmark, "PolyBench-ACC/OpenCL/stencils/jacobi-2d-imper")
benchCMD["jacobi-2d-imper"]="jacobi2D.exe"
#------------------------------------------------------------------------------


#-------------------------------FINANCEBENCH-----------------------------------
benchCD["FB-Black-Scholes"]=os.path.join(path_to_benchmark, "FinanceBench/Black-Scholes/OpenCL")
benchCMD["FB-Black-Scholes"]="blackScholesAnalyticEngine.exe"
#
benchCD["FB-Monte-Carlo"]=os.path.join(path_to_benchmark, "FinanceBench/Monte-Carlo/OpenCL")
benchCMD["FB-Monte-Carlo"]="monteCarloEngine.exe"
#------------------------------------------------------------------------------


#---------------------------------GPUSTREAM------------------------------------
benchCD["gpustream"]=os.path.join(path_to_benchmark, "GPU-STREAM")
benchCMD["gpustream"]="gpu-stream-ocl"
#------------------------------------------------------------------------------


#----------------------------------MANTEVO-------------------------------------
benchCD["cloverleaf"]=os.path.join(path_to_benchmark,"mantevo/CloverLeaf_OpenCL")
benchCMD["cloverleaf"]="clover_leaf"
#
benchCD["cloverleaf3d"]=os.path.join(path_to_benchmark,"mantevo/CloverLeaf3D_OpenCL")
benchCMD["cloverleaf3d"]="clover_leaf"
#
benchCD["tealeaf"]=os.path.join(path_to_benchmark,"mantevo/TeaLeaf_OpenCL")
benchCMD["tealeaf"]="tea_leaf"
#
benchCD["tealeaf3d"]=os.path.join(path_to_benchmark,"mantevo/TeaLeaf3D_OpenCL")
benchCMD["tealeaf3d"]="tea_leaf"
#------------------------------------------------------------------------------


#--------------------------------HETERO-MARK-----------------------------------
benchCD["aes_hm"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl12/aes_cl12")
benchCMD["aes_hm"]="aes_cl12 -i ../../../../data/aes/detector_input.txt -k ../../../../data/aes/key.txt"
benchPrefix["aes_hm"]="LD_LIBRARY_PATH=/opt/AMDAPP/lib/x86_64/:$LD_LIBRARY_PATH "
#
benchCD["fir_hm"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl12/fir_cl12")
benchCMD["fir_hm"]="fir_cl12 -d 20000000"
benchPrefix["fir_hm"]="LD_LIBRARY_PATH=/opt/AMDAPP/lib/x86_64/:$LD_LIBRARY_PATH "
#
benchCD["hmm_hm"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl12/hmm_cl12")
benchCMD["hmm_hm"]="hmm_cl12 -s 256"
benchPrefix["hmm_hm"]="LD_LIBRARY_PATH=/opt/AMDAPP/lib/x86_64/:$LD_LIBRARY_PATH "
#
benchCD["iir_hm"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl12/iir_cl12")
benchCMD["iir_hm"]="iir_cl12 -l 4096"
benchPrefix["iir_hm"]="LD_LIBRARY_PATH=/opt/AMDAPP/lib/x86_64/:$LD_LIBRARY_PATH "
#
benchCD["kmeans_hm"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl12/kmeans_cl12")
benchCMD["kmeans_hm"]="kmeans_cl12 -f ../../../../data/kmeans/1500000_34.txt"
benchPrefix["kmeans_hm"]="LD_LIBRARY_PATH=/opt/AMDAPP/lib/x86_64/:$LD_LIBRARY_PATH "
#
benchCD["pagerank_hm"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl12/pagerank_cl12")
benchCMD["pagerank_hm"]="pagerank_cl12 -m ../../../../data/page_rank/csr_9216_10.txt"
benchPrefix["pagerank_hm"]="LD_LIBRARY_PATH=/opt/AMDAPP/lib/x86_64/:$LD_LIBRARY_PATH "
#
benchCD["sw_hm"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl12/sw_cl12")
benchCMD["sw_hm"]="sw_cl12"
benchPrefix["sw_hm"]="LD_LIBRARY_PATH=/opt/AMDAPP/lib/x86_64/:$LD_LIBRARY_PATH "
#------------------------------------------------------------------------------


#---------------------------HETERO-MARK OPENCL 2.0-----------------------------
benchCD["aes_hm_cl2"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl20/aes_cl20")
benchCMD["aes_hm_cl2"]="aes_cl20 -i ../../../../data/aes/detector_small_input.txt -k ../../../../data/aes/key.txt"
#
benchCD["fir_hm_cl2"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl20/fir_cl20")
benchCMD["fir_hm_cl2"]="fir_cl20 -d 20000000"
#
benchCD["hmm_hm_cl2"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl20/hmm_cl20")
benchCMD["hmm_hm_cl2"]="hmm_cl20 -s 256"
#
benchCD["iir_hm_cl2"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl20/iir_cl20")
benchCMD["iir_hm_cl2"]="iir_cl20 -l 4096"
#
benchCD["kmeans_hm_cl2"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl20/kmeans_cl20")
benchCMD["kmeans_hm_cl2"]="kmeans_cl20 -i ../../../../data/kmeans/1500000_34.txt"
#
benchCD["pagerank_hm_cl2"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl20/pagerank_cl20")
benchCMD["pagerank_hm_cl2"]="pagerank_cl20 -m ../../../../data/page_rank/csr_9216_10.txt"
#
benchCD["sw_hm_cl2"]=os.path.join(path_to_benchmark,"Hetero-Mark/build/src/opencl20/sw_cl20")
benchCMD["sw_hm_cl2"]="sw_cl20"
#------------------------------------------------------------------------------


#-----------------------------------LINPACK------------------------------------
benchCD["hpl"]=os.path.join(path_to_benchmark,"LINPACK/hpl-gpu/bin/Generic/")
benchCMD["hpl"]="xhpl"
benchPrefix["hpl"]="LD_LIBRARY_PATH=/opt/AMDAPP/lib/x86_64:"+os.path.join(path_to_benchmark,"LINPACK/lib/")+":"+os.path.join(path_to_benchmark,"LINPACK/acml/gfortran64_mp/lib/")+":$LD_LIBRARY_PATH OMP_NUM_THREADS=2"
#------------------------------------------------------------------------------


#----------------------------------VIENNACL------------------------------------
benchCD["dense_blas-bench-opencl"]=os.path.join(path_to_benchmark, "ViennaCL-1.7.1/build/examples/benchmarks")
benchCMD["dense_blas-bench-opencl"]="dense_blas-bench-opencl"
#
benchCD["amg"]=os.path.join(path_to_benchmark, "ViennaCL-1.7.1/build/examples/tutorial")
benchCMD["amg"]="amg ../testdata/mat65k.mtx"
#
benchCD["bisect"]=os.path.join(path_to_benchmark, "ViennaCL-1.7.1/build/examples/tutorial")
benchCMD["bisect"]="bisect"
#
benchCD["custom-kernels"]=os.path.join(path_to_benchmark, "ViennaCL-1.7.1/build/examples/tutorial")
benchCMD["custom-kernels"]="./custom-kernels"
#
benchCD["matrix-free"]=os.path.join(path_to_benchmark, "ViennaCL-1.7.1/build/examples/tutorial")
benchCMD["matrix-free"]="matrix-free"
#
benchCD["nmf"]=os.path.join(path_to_benchmark, "ViennaCL-1.7.1/build/examples/tutorial")
benchCMD["nmf"]="nmf"
#
benchCD["blas3_solve-test-opencl"]=os.path.join(path_to_benchmark, "ViennaCL-1.7.1/build/tests")
benchCMD["blas3_solve-test-opencl"]="blas3_solve-test-opencl"
#
benchCD["sparse_prod-test-opencl"]=os.path.join(path_to_benchmark, "ViennaCL-1.7.1/build/tests")
benchCMD["sparse_prod-test-opencl"]="sparse_prod-test-opencl"
#------------------------------------------------------------------------------


#----------------------------------SNU_NPB-------------------------------------
benchCD["npb_ocl_bt"]=os.path.join(path_to_benchmark,"SNU_NPB-1.0.3/NPB3.3-OCL/")
benchCMD["npb_ocl_bt"]="./bin/bt.A.x BT"
benchPrefix["npb_ocl_bt"]="OPENCL_DEVICE_TYPE=gpu"
#
benchCD["npb_ocl_cg"]=os.path.join(path_to_benchmark,"SNU_NPB-1.0.3/NPB3.3-OCL/")
benchCMD["npb_ocl_cg"]="./bin/cg.A.x CG"
benchPrefix["npb_ocl_cg"]="OPENCL_DEVICE_TYPE=gpu"
#
benchCD["npb_ocl_ep"]=os.path.join(path_to_benchmark,"SNU_NPB-1.0.3/NPB3.3-OCL/")
benchCMD["npb_ocl_ep"]="./bin/ep.A.x EP"
benchPrefix["npb_ocl_ep"]="OPENCL_DEVICE_TYPE=gpu"
#
benchCD["npb_ocl_ft"]=os.path.join(path_to_benchmark,"SNU_NPB-1.0.3/NPB3.3-OCL/")
benchCMD["npb_ocl_ft"]="./bin/ft.A.x FT"
benchPrefix["npb_ocl_ft"]="OPENCL_DEVICE_TYPE=gpu"
#
benchCD["npb_ocl_is"]=os.path.join(path_to_benchmark,"SNU_NPB-1.0.3/NPB3.3-OCL/")
benchCMD["npb_ocl_is"]="./bin/is.A.x IS"
benchPrefix["npb_ocl_is"]="OPENCL_DEVICE_TYPE=gpu"
#
benchCD["npb_ocl_lu"]=os.path.join(path_to_benchmark,"SNU_NPB-1.0.3/NPB3.3-OCL/")
benchCMD["npb_ocl_lu"]="./bin/lu.S.x LU"
benchPrefix["npb_ocl_lu"]="OPENCL_DEVICE_TYPE=gpu"
#
benchCD["npb_ocl_mg"]=os.path.join(path_to_benchmark,"SNU_NPB-1.0.3/NPB3.3-OCL/")
benchCMD["npb_ocl_mg"]="./bin/mg.A.x MG"
benchPrefix["npb_ocl_mg"]="OPENCL_DEVICE_TYPE=gpu"
#
benchCD["npb_ocl_sp"]=os.path.join(path_to_benchmark,"SNU_NPB-1.0.3/NPB3.3-OCL/")
benchCMD["npb_ocl_sp"]="./bin/sp.A.x SP"
benchPrefix["npb_ocl_sp"]="OPENCL_DEVICE_TYPE=gpu"
#------------------------------------------------------------------------------

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

===============================================================================
Special directions for recreating the CGO 2017 work
===============================================================================

The data in this branch is slightly different than our main branch. It contains
most of our files as they were when we took our data for the CGO 2017 paper
"Dynamic Buffer Overflow Detection for GPGPUS". This branch also includes some
of our helper scripts used to run all of the benchmarks in the paper, as well
as a test microbenchmark that was used fill in some of the paper's data.

Some of the major differences:
 * The major runscript was named run_overflow_detect.py instead of 'clarmor'.
   The majority of what it does is still the same.
 * We did not yet wrap dlsym() calls, so this version will not necessarily
   work for applications that load OpenCL using dlopen()/dlsym().
 * The time_all_benchmarks.sh script can be used to automatically create a
   runtime log for every benchmark. This was used to create Figure 10 in the
   paper, in conjunction with the mean_time_files.py script.
 * bufferUpBench is our microbenchmark that can be used to generate Figures 3
   through 8. To run this for all of these data points, please use the script
   helper_master.sh (which will run helper_clmem.sh, helper_image.sh, and
   helper_svm.sh).


===============================================================================
This is the AMD Research OpenCL Buffer Overflow Detector. This tool wraps
API calls to OpenCL libraries and dynamically checks for memory errors caused
by OpenCL kernels. When an error is detected (e.g. when an OpenCL kernel writes
beyond the end of an OpenCL buffer), error information is printed to the
screen and is optionally logged to a detector output file.

The details of how this tool works can be found in the ./docs/ directory, but
the broad overview is that this is a canary-based overflow detector. OpenCL
API calls that create buffers (such as clCreateBuffer, clCreateImage, and
clSVMAlloc) are intercepted by the tool, and the buffers are expanded to
include 'canary' regions outside of the buffer. These regions are filled with
known values.

Later, OpenCL API calls that launch kernels are also intercepted. After the
kernel has completed, the canary values of all buffers the kernel could
possibly touch are analyzed. If the canaries have changed from their known-good
values, that is an indication that the kernel caused a buffer overflow.
Debugging information about this buffer overflow is then printed to the screen
and, optionally, to a log file. The tool will then either continue running in
order to try to find more problems, or, optionally, will exit with an error.

This tool is only useful for finding buffer overflows in OpenCL kernels and
against buffers created using OpenCL APIs. It does not work for fine-grained
system shared virtual memory, since those regions are allocated with
traditional calls to malloc. It does not attempt to find buffer overflows
from host-side functions that may write into OpenCL buffer regions. For
instance, mapping a buffer and then writing outside of the mapped region
will not necessarily be detected. If it is detected, it may be mis-attributed.

Because this tool runs the application and searches for actual overflows caused
during the program's execution, the ability of this tool to find buffer
overflows depends on the inputs to the program. If the application's input
does not cause a buffer overflow, this tool will not necessarily find nor
report the problem.

In addition, because of the manner in which the canaries are checked (the
checks are performed after the real kernel has completed), this tool offers no
security guarantees. An OpenCL buffer overflow that allows an attacker to take
control of the application may not be observed, since a dedicated attacker
could prevent the canary checker from working.

Nevertheless, this tool has been used to find buffer overflows in real OpenCL
applications, and it is useful as a debugging and development mechanism.
It has been carefully designed to attempt to reduce the runtime overheads it
causes. It will detect overflows in cl_mem buffers, coarse-grained SVM, and
memory buffers for n-dimensional images.


The AMD Research OpenCl Buffer Overflow Detector has been tested on the
following systems:
OpenCL 2.0:
 1) AMD FirePro W9100 GPU running on 64-bit Ubuntu 14.04.4 with the AMD APP
    SDK v3.0 and fglrx 15.30.3 drivers.
 2) AMD Radeon Fury X GPU running on 64-bit Ubuntu 14.04.5 with the AMD APP
    SDK v3.0 and fglrx 15.30.3 drivers.

OpenCL 1.2:
 1) AMD Radeon HD 7950 GPU running on 64-bit CentOS 6.4 with the AMD APP SDK
    v2.8 and fglrx 13.20.6 drivers. This requires installing GCC 4.7.


Currently, this tool does *not* detect the following types of overflows:
 1) Buffer overflows that overflow in a negative direction. In other words,
    writing bytes before the beginning of a buffer is not detected as an error.



-------------------------------------------------------------------------------
Setting up and building the AMD Research OpenCL Buffer Overflow Detector
-------------------------------------------------------------------------------

The AMD Research OpenCL Buffer Overflow Detector requires a number of utilities
in order to properly run. Information about these prerequisites can be found
in the prereqs/ directory. In particular, the file prereqs/README.txt discusses
information about how to set up a system to run this tool.

Currently, this tool is designed to detect buffer overflows on GPU-based OpenCL
kernels. It has been tested using the AMD APP SDK OpenCL implementation on top
of AMD Catalyst drivers. It is designed to run only on Linux. All of these
things should be set up separately from this tool.

This tool requires Python and Python-Argparse. To fully run the test system,
Clang's scan-build tool should be installed, as should Cppcheck.

This tool has been tested using both GCC (v >= 4.7) and clang (v >= 3.3)

To build the AMD Research OpenCL Buffer Overflow Detector using your default
C and C++ compilers, just execute the following from the main directory:
    make

To build the tool using another compiler (e.g. clang), override the CC and
CXX environment variables. For example:
    CC=clang CXX=clang++ make

(Note that the above means that you can also use 'scan-build make' to run
LLVM's static analyzer on this tool.)

To test the tool against its included functional tests, execute:
    make test

To simply build these functional tests without running them, execute:
    make build_test

To run Cppcheck against the detector and all the test codes, run:
    make check

Finally, to clean up any builds and remove any temporary files, run:
    make clean



-------------------------------------------------------------------------------
Running the AMD Research OpenCL Buffer Overflow Detector
-------------------------------------------------------------------------------

The AMD Research OpenCL Buffer Overflow Detector is primarily run using the
script 'bin/run_overflow_detect.py'. Executing this script with the '-h' or
'--help' argument will explain the arguments to this tool.

The simplest description of how to use the buffer overflow detector is to run:
    bin/run_overflow_detect.py -w {working directory} -r "{command line}"

For example, the following will run the FFT benchmark in the AMD APP SDK
through the buffer overflow detector:
    ./bin/run_overflow_detect.py -w /opt/AMDAPP/samples/opencl/bin/x86_64 -r FFT


In the above example command line, the following two parameters were used:
    --working_directory (or -w):
        This parameter allows you to set the working directory of the program
        you wish to test in the overflow detector. {working directory} is the
        the directory where the the program should be run from.
        The run script will temporarily change directories to this working dir,
        and will also temporarily add it to the PATH variable.

    --run (or -r):
        This parameter is the command line used to run the application you wish
        to test. This includes any parameters that you wish to pass into the
        benchmark you wish to test. Because the working directory is part of
        the PATH, this can simply be the name of the binary you wish to run.
        Alternately, it can contain the entire path to the program that you
        want to run.
        This parameter can be put into quotation marks to allow for spaces
        in the application-under-test's command line.

Other important functions that can be controlled when using the runscript:
    --exit_on_overflow (or -e):
        If the buffer overflow detector finds an OpenCL buffer overflow, this
        parameter will cause the tool to halt the program with an error. This
        helps to minimize any damage from buffer overflows and to limit the
        amount of data dumped to the screen.
        This is disabled by default, meaning that the program will try to
        run to completion even in the face of a detected overflow.

    --log (or -l):
        This enables logging to a file, rather than simply printint out
        information about buffer overflows to the screen. This log file will
        be stored in the working directory (set with the '-w' parameter).
        The file will be {working_directory}/buffer_overflow_detector.out
        by default. The following parameter, '--logfile', allows this to be
        overridden.

    --logfile (or -f):
        If using the '--log' parameter described above, this can be used to put
        the logfile in a different directory and with a different filename.
        This should be the entire path to the logfile, including the filename.

    --cutoff_for_device (or -c):
        The buffer overflow detector can perform the canary region checks on
        the device or on the host. The CPU has the benefit of running faster
        when there are fewer buffers to check, since it has more serial
        performance and has very little launch overhead to amortize. The device
        (in particular, if it's a GPU) is better when there are more buffers,
        since the checks would have ample parallel work.
        This sets the cutoff for the number of buffers that a kernel must be
        able to access before it is checked on the device. Setting this to 1
        means that all checks run on the device.
        Setting this to 0 lets the library decide what to do.
        This can also be controlled by setting the environment variable:
            DEVICE_CHECK_CUTOFF

    --gpu_method (or -m):
        An override to decide how to perform device-side canary checks.
        This tool has multiple methods of performing these checks, and this
        helps override the default.
        0 (default): launch a single checker kernel that will check all of the
            canary values for all of the buffers. These canaries are copied
            into a single input buffer that is checked by the kernel.
        1: Launch a single checker kernel for all canary values. For SVM
            buffers, make no copies of the canaries; instead, check them
            in-place by passing pointers to the checker kernel.
        2: Launch a single kernel per buffer to check the canaries in-place.

    --detector_path (or -d):
        This should be the root directory of the AMD Research OpenCL Buffer
        Overflow Detector installation you are using. This should be
        automatically set as a path relative to the location of the run script,
        but if it is incorrect, you can set it with this parameter.

The following parameter can be used to help debug broken applications and
problems in the detector itself:
    --use_gdb:
        This will run the application (and the detector as well) within GDB.
        This will allow users to run the application within the debugger while
        also searching for buffer overflows.

    --use_pdb:
        This will run the run_overflow_detect.py script within PDB, in order to
        help debug any problems in it.

The following parameters are useful for benchmarking the tool. They will
run benchmarks or groups of benchmarks that are described in the
bin/benchmarks.py file. These will override values passed with the -w and
-r parameters. These settings assume you've installed the benchmarks using the
prereqs/download_and_build_benchmarks.sh script.
    --benchmark (or -b):
        Use this to run a single benchmark from the list of names at the top of
        bin/benchmarks.py. For example, 'FFT' will automatically set the
        working directory and command line to run the FFT benchmark.
        Open up bin/benchmarks.py to see the list of benchmarks.

    --group (or -g):
        Run an entire group of benchmarks from one command line call.
        For instance, using 'AMDAPP' will run all of the support benchmarks
        from the AMD APP SDK. The special group ALL_BENCHMARKS will run
        all other groups, running (as the name implies) all benchmarks.

    --perf_stat (or -p):
        A set of boolean flags that can be used to turn on different kinds of
        performance logging. Or the flags together to simultaneously log
        multiple things.
        1 - Monitor how long GPU kernels take, including how long they sit
            in their work queue. This information will be saved into
            debug_enqueue_time.csv
        2 - Monitor how long canary checks take. This information will be
            stored into debug_check_time.csv
        4 - Monitor the memory overhead of the canary regions vs. the total
            size of OpenCL buffers. This information will be stored into
            debug_mem_overhead.csv



-------------------------------------------------------------------------------
Benchmarking the AMD Research OpenCL Buffer Overflow Detector
-------------------------------------------------------------------------------

The AMD Research OpenCL Buffer Overflow Detector is designed to run at very low
runtime overheads. The ideal detector would cause no runtime overheads.
However, this tool must catch OpenCL API calls, expand buffers to include
canary values, and must run checks on these canaries after each kernel
finishes execution. As such, there is some amount of runtime overhead.

In order to verify that the buffer overflow detector works and to help measure
its overheads, the AMD Research OpenCL Buffer Overflow Detector includes a
series of benchmarking mechanisms to allow it to automatically run against
a large number (over 150) OpenCL applications from a variety of benchmark
suites and libraries.

To begin with, you should download the benchmarks. The detector includes a
selection of scripts to help automatically download and build these programs.
The primary means of doing this is to run the script:
    prereqs/download_and_build_benchmarks.sh

This will install a series of benchmarks into your home directory at:
~/benchmarks/

Note that some of the benchmarks this script tries to build cannot be
automatically downloaded from the internet. Parboil, the SNU OpenCL versions
of the NAS Parallel Benchmarks, and the AMD Core Math Libraries require
manual intervention to download. The download-and-build script will not
make forward progress until these files are in the correct locations. It will
spit out a warning and tell you what to download and wheere to put it.


After the benchmarks have finished building, they will be contained in
the ~/benchmarks/ folder. The AMD Research OpenCL Buffer Overflow detector
can then automatically run them through a series of benchmark scripts.

The script 'bin/benchmarks.py' includes a collection of benchmark groups,
benchmark names, and the command lines to run each of these benchmarks.

The groups of benchmarks are declared at the top, and each is a Python string
that contains the names of all of the benchmarks in that group. The largest
group is the ALL_BENCHMARKS group, which contains all the other benchmark
groups.

Each benchmark has a number of parameters:
    1) benchCD["benchmark_name"] contains the working directory of the benchmark
    2) benchCMD["benchmark_name"] contains the command line parameter to run
        This command will be run out of the benchCD directory.
    3) the optional benchPrefix["benchmark_name"]. This can be used to set a
        part of the command line that will appear *before* the benchmark
        command line. For instance, this can be used to set environment
        variables for this benchmark.

The normal runscript, 'bin/run_overflow_detect.py' has two flags that can help
with these benchmark runs:
    --benchmark (or -b):
        This is useful for running a single benchmark. The parameter here is the
        name of the benchmark that is found in bin/benchmarks.py

    --group (or -g):
        This will allow you to run a group of benchmarks with a single command.
        The group used here should be one of the group names from the file
        bin/benchmarks.py.


For example, in the benchmarks.py file, there is a group called 'AMDAPP'. One
of the benchmarks in this group is 'BlackScholes'. To automatically run the
BlackScholes benchmark, use the following example command:
    ./bin/run_overflow_detect.py --benchmark BlackScholes

Alternately, to run all of the benchmarks in the AMD APP SDK group, use:
    ./bin/run_overflow_detect.py --group AMDAPP


-------------------------------------------------------------------------------
Software Architecture
-------------------------------------------------------------------------------


-------------------------------------------------------------------------------
Coding Standards
-------------------------------------------------------------------------------


-------------------------------------------------------------------------------
Testing
-------------------------------------------------------------------------------
The simplest way to run all of the tests for the AMD Research OpenCL Buffer
Overflow Detector is to use the script at:
    tests/automated_test.sh

This script will all of the tests below, back to back, and report the results
to the screen. If any of the tests fail, the script will return a non-zero
value. These tests are split into two categories, which the automated test
script will run, depending on command-line flags.
    1) Build-time tests --- This uses the '-b' option
    2) Tests against a benchmark group --- This uses the '-g {group}' option

The simplest way to run all tests is:
    ./tests/automated_test.sh -b -g ALL_BENCHMARKS


The build-time tests are:
'make check' will run cppcheck, a static code analyzer, over all of the tools
    and tests that are included in the package.

'make pylint' will run pylint over all of the Python files in the project.

'scan-build make' can be used to verify that the Clang static analyzer does
    not find any problems in the tool. Shipping versions should come back
    clean in contemporary versions of scan-build at the time they ship.

'make test' will run a series of programs from the tests/ subdirectory.

    The tests/ subdirectory contains a README.txt file which describes in
    detail what these tests do and how they are written. In a nutshell, tests/
    contains a series of subdirectories that each represent a series of tests.
    For each type of test, there is a good_* test and a bad_* test.

    The 'good' tests do not have buffer overflows, but go through all the
    motions of creating the correct buffer types and launching work. These
    tests are meant to verify that the buffer overflow detector does not find
    any false positives.

    The 'bad' tests perform actions that cause a series of buffer overflows.
    When run using the 'make test' command, they will be run through the buffer
    overflow detector, which will create a log detailing each of these errors.
    Each folder has a configuration parameter that lists the correct number of
    errors that should be found. If the detector finds too few or too many, the
    tests will fail.


With regards to the benchmark tests:
It is also possible to automatically run the buffer overflow detector across
the same benchmarks that are used for performanc testing, as described above.

By default, the benchmark build process should patch all of these benchmarks to
no longer have any buffer overflows. However, running the tool across all of
these benchmarks still helps to verify that there are no bugs in the tool that
cause things to crash or find false positives.

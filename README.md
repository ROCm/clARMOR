clARMOR - A Buffer Overflow Detector for OpenCL Programs
==================
This is clARMOR, the AMD Research Memory Overflow Reporter for OpenCL. This
tool wraps API calls to OpenCL libraries and dynamically checks for memory
errors caused by OpenCL kernels. When an error is detected (e.g. when a kernel
writes beyond the end of a buffer), error information is printed to the screen
and is optionally logged to an output file.

The details of how this tool works can be found in the ./docs/ directory, and
a deeper introduction can be found in the main README.txt file.

Build Status
--------------------------------------------------------------------------------
| Build branch | master | develop |
|-----|-----|-----|
| GCC+Clang, Linux, AMD64 | [![Build Status](https://travis-ci.org/ROCm-Developer-Tools/clARMOR.svg?branch=master)](https://travis-ci.org/GPUOpen-ProfessionalCompute-Tools/clARMOR/branches) | [![Build Status](https://travis-ci.org/ROCm-Developer-Tools/clARMOR.svg?branch=develop)](https://travis-ci.org/GPUOpen-ProfessionalCompute-Tools/clARMOR/branches) |

Setting up and building clARMOR
--------------------------------------------------------------------------------

clARMOR requires a number of utilities in order to properly run. Information
about these prerequisites can be found in the prereqs/ directory. In particular,
the file prereqs/README.txt discusses information about how to set up a system
to run this tool. At this time, clARMOR only runs on Linux.

A simplified list of software required to run clARMOR:

* A working OpenCL 1.1+ installation
* GCC >= 4.7 or LLVM >= 3.3
* Python and Python argparse

Additional software that is useful to have installed when working with clARMOR:

* gdb
* Clang Static Analyzer (scan-build)
* cppcheck
* pylint

Currently, this tool is designed to detect buffer overflows caused by OpenCL
kernels that run on GPUs or CPUs. It has been tested most extensively using the
AMD APP SDK OpenCL runtime implementation and AMD Catalyst GPU drivers.

To build the clARMOR using your default C and C++ compilers, execute the
following from the main directory:

    make

To build the tool using another compiler (e.g. clang), override the CC and
CXX environment variables. For example:

    CC=clang CXX=clang++ make

(Note that the above means that you can also use 'scan-build make' to run
LLVM's static analyzer on this tool.)

To test the tool against its included functional GPU tests, execute:i

    make test

To test the tool against its included functional tests, but to run those tests
(and thus the detection algorithms) on the CPU, execute:

    make cpu_test

To simply build these functional tests without running them, execute:

    make build_test

To run Cppcheck against the detector and all the test codes, run:

    make check

To run pylint against all of the Python files, execute:

    make pylint

clARMOR can also be built in debug mode to help find problems in the
detector itself by running:

    DEBUG=1 make

Finally, to clean up any builds and remove any temporary files, run:

    make clean


Running clARMOR
--------------------------------------------------------------------------------

clARMOR is primarily run using 'bin/clarmor'.
Executing this tool with the '-h' or '--help' will explain its arguments.

The simplest description of how to use the buffer overflow detector is to run:

    bin/clarmor -w {working directory} -r "{command line}"

For example, the following will run the FFT benchmark in the AMD APP SDK
through the buffer overflow detector:

    ./bin/clarmor -w /opt/AMDAPP/samples/opencl/bin/x86_64 -r FFT


In the above example command line, the following two parameters were used:

    --working_directory (or -w):
        This parameter allows you to set the working directory of the program
        you wish to test in the overflow detector. {working directory} is the
        the directory where the the program should be run from.
        The run script will temporarily change directories to this working dir,
        and will also temporarily add it to the PATH environment variable.

    --run (or -r):
        This parameter is the command line used to run the application you wish
        to test. This includes any parameters that you wish to pass into the
        benchmark you wish to test. Because the working directory is part of
        the PATH, this can simply be the name of the binary you wish to run.
        Alternately, it can contain the entire path to the program that you
        want to run.
        This parameter can be put into quotation marks ("") to allow for spaces
        in the application-under-test's command line.

Other important functions that can be controlled when using the runscript:

    --exit_on_overflow (or -e):
        If the buffer overflow detector finds an OpenCL buffer overflow, this
        parameter will cause the tool to halt the program with an error. This
        helps to minimize any damage from buffer overflows and to limit the
        amount of data dumped to the screen.
        This is disabled by default, meaning that the program will try to
        run to completion even in the face of a detected overflow.

    --error_exitcode (or -x):
        Allows the user to specify an exit code for program termination upon
        overflow detection. This flag is only meaningful if --exit_on_overflow
        is set. By default, the error code is set to -1.

    --log (or -l):
        This enables logging to a file, rather than simply printing out
        information about buffer overflows to the screen. By default, this
        log file will be stored in the working directory (set with the '-w'
        parameter). By default, the file will be:
        {working_directory}/buffer_overflow_detector.out
        The following parameter, '--logfile', allows this to be overridden.

    --logfile (or -f):
        If using the '--log' parameter described above, this can be used to put
        the logfile in a different directory and with a different filename.
        This should be the entire path to the logfile, including the filename.

    --prefix
        Allows the user to specify a string to be appended to the front of each
        output line generated by the tool when it finds an overflow.
        This is "clARMOR: " by default.

    --no_api_check
        Normally, clARMOR will check both OpenCL kernels and OpenCL API calls.
        API calls like clEnqueueCopyBuffer() can attempt to move too much data
        into a buffer. clARMOR will check the limits of the data movement
        for these API calls before they run.
        This flag turns off that API checking.

The following parameter can be used to help debug broken applications and
problems in the detector itself:

    --use_gdb:
        This will run the application (and the detector as well) within GDB.
        This will allow users to run the application within the debugger while
        also searching for buffer overflows.

Testing clARMOR
--------------------------------------------------------------------------------
The simplest way to run all of the tests for clARMOR is to use the script at:

    tests/automated_test.sh

This script will all of the tests below, back to back, and report the results
to the screen. If any of the tests fail, the script will return a non-zero
value. These tests are split into three categories, which the automated test
script will run, depending on command-line flags.

1. Build-time tests --- This uses the '-b' option
2. Functional tests --- This uses the '-t' option
3. Tests against a benchmark group --- This uses the '-g {group}' option

The simplest way to run tests is:

    tests/automated_test.sh -b -t

The build-time tests ('-b' option) are:

* `make` will ensure that clARMOR properly builds using the compilers defined
in the CC and CXX environment variables
* `make check` will run cppcheck, a static code analyzer, over all of the tools
and tests that are included in the package.
* `make pylint` will run pylint over all of the Python files in the project.
* `scan-build make` can be used to verify that the Clang Static Analyzer does
not find any problems in the tool. Shipping versions should come back
clean in contemporary versions of scan-build at the time they ship.

The functional tests ('-t' option) are:
* `make test` will run a series of programs from the tests/ subdirectory
(which are further describes in tests/README.txt). If GPU devices are not
supported by the OpenCL runtime, this test does not run.
* `make cpu_test` will run the same tests as the normal make test, but will
ensure that they run on the CPU. This is useful for testing on systems
that do not have a GPU. This requires an OpenCL runtime that allows kernels to
run on the CPU. If CPU-side tests are not supported, this does not run.

clARMOR Limitations
--------------------------------------------------------------------------------
clARMOR is useful for finding write-based buffer overflows in buffers created
using OpenCL APIs and caused by OpenCL APIs or OpenCL kernels. It does not
attempt to find buffer overflows from host-side functions that may write into
OpenCL buffer regions. For instance, mapping a buffer and then writing outside
of the mapped region will not necessarily be detected.
If this is detected, it may be mis-attributed.

In addition, because of the manner in which the canaries are checked (the
checks are performed after the real kernel has completed), this tool offers no
security guarantees. An OpenCL buffer overflow that allows an attacker to take
control of the application may not be observed, since a dedicated attacker
could prevent the canary checker from working.

Nevertheless, this tool has been used to find buffer overflows in real OpenCL
applications, and it is useful as a debugging and development mechanism.
It has been carefully designed to attempt to reduce the runtime overheads it
causes. It will detect overflows in cl\_mem buffers, coarse-grained SVM, and
memory buffers for n-dimensional images.

Currently, this tool does *not* detect the following types of overflows:

1. Buffer overflows that overflow in a negative direction. In other words,
writing bytes before the beginning of a buffer is not detected as an error.
2. Buffer overflows in the \_\_private, \_\_local, or \_\_constant memory spaces.
3. Buffer overflows caused by reads (since these do not disrupt the canary
regions).

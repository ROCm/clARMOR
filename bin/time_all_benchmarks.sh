
FILEOUT=all_bench_times

I=1
END=10
while [ $I -le $END ]
do
    ./run_overflow_detect.py -g ALL_BENCHMARKS -t $FILEOUT''$I'.csv' --gpu_method 1
    I=$(($I+1))
done

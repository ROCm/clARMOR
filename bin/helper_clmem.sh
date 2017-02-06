FILEOUT=$1
OP=$2
TEMP=temp_time.csv
WALLTIME=ubench_clmem_walltime.csv
ITER=10
DEBUG_FILE=debug_*

> $WALLTIME
echo $FILEOUT
> $FILEOUT
rm $DEBUG_FILE

I=2
END=20
while [ $I -le $END ]
do
    ./run_overflow_detect.py -r $PWD'/../bufferUpUbench/bufferUp.exe '$PWD'/../bufferUpUbench/ '$I' '$ITER' Q' -t $TEMP $OP
    cat $DEBUG_FILE >> $FILEOUT
    echo >> $FILEOUT
    cat $TEMP >> $WALLTIME
    I=$(($I+2))
done


rm $TEMP

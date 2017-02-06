#clmem
./helper_clmem.sh ubench_cpu_clmem_enqueue.csv '-c 2 --perf_stat 1'
./combineRuntimes.py ubench_cpu_clmem_enqueue.csv ubench_cpu_clmem_time.csv
#./helper_clmem.sh 2 ubench_cpu_clmem_memory.csv '--perf_stat 4'

./helper_clmem.sh ubench_single_clmem_enqueue.csv '-c 1 --gpu_method 2 --perf_stat 1'
./helper_clmem.sh ubench_single_clmem_check.csv '-c 1 --gpu_method 2 --perf_stat 2'
./combineRuntimes.py ubench_single_clmem_enqueue.csv ubench_single_clmem_check.csv ubench_single_clmem_time.csv
#./helper_clmem.sh 1 ubench_single_clmem_memory.csv '--gpu_method 2 --perf_stat 4'

./helper_clmem.sh ubench_multi_clmem_enqueue.csv '-c 1 --gpu_method 0 --perf_stat 1'
./helper_clmem.sh ubench_multi_clmem_check.csv '-c 1 --gpu_method 0 --perf_stat 2'
./combineRuntimes.py ubench_multi_clmem_enqueue.csv ubench_multi_clmem_check.csv ubench_multi_clmem_time.csv
#./helper_clmem.sh 1 ubench_multi_clmem_memory.csv '--gpu_method 0 --perf_stat 4'


#svm
./helper_svm.sh ubench_cpu_svm_enqueue.csv '-c 2 --perf_stat 1'
./combineRuntimes.py ubench_cpu_svm_enqueue.csv ubench_cpu_svm_time.csv
#./helper_svm.sh 2 ubench_cpu_svm_memory.csv '--perf_stat 4'

./helper_svm.sh ubench_single_svm_enqueue.csv '-c 1 --gpu_method 2 --perf_stat 1'
./helper_svm.sh ubench_single_svm_check.csv '-c 1 --gpu_method 2 --perf_stat 2'
./combineRuntimes.py ubench_single_svm_enqueue.csv ubench_single_svm_check.csv ubench_single_svm_time.csv
#./helper_svm.sh 1 ubench_single_svm_memory.csv '--gpu_method 2 --perf_stat 4'

./helper_svm.sh ubench_multi_svm_enqueue.csv '-c 1 --gpu_method 0 --perf_stat 1'
./helper_svm.sh ubench_multi_svm_check.csv '-c 1 --gpu_method 0 --perf_stat 2'
./combineRuntimes.py ubench_multi_svm_enqueue.csv ubench_multi_svm_check.csv ubench_multi_svm_time.csv
#./helper_svm.sh 1 ubench_multi_svm_memory.csv '--gpu_method 0 --perf_stat 4'

./helper_svm.sh ubench_multi_svmptr_enqueue.csv '-c 1 --gpu_method 1 --perf_stat 1'
./helper_svm.sh ubench_multi_svmptr_check.csv '-c 1 --gpu_method 1 --perf_stat 2'
./combineRuntimes.py ubench_multi_svmptr_enqueue.csv ubench_multi_svmptr_check.csv ubench_multi_svmptr_time.csv
#./helper_svm.sh 1 ubench_multi_svmptr_memory.csv '--gpu_method 1 --perf_stat 4'


#image
IMAGE=image
./helper_image.sh ubench_cpu_${IMAGE}_enqueue.csv '-c 2 --perf_stat 1'
./combineRuntimes.py ubench_cpu_${IMAGE}_enqueue.csv ubench_cpu_${IMAGE}_time.csv
#./helper_image.sh 2 ubench_cpu_${IMAGE}_memory.csv '--perf_stat 4'

./helper_image.sh ubench_single_${IMAGE}_enqueue.csv '-c 1 --gpu_method 2 --perf_stat 1'
./helper_image.sh ubench_single_${IMAGE}_check.csv '-c 1 --gpu_method 2 --perf_stat 2'
./combineRuntimes.py ubench_single_${IMAGE}_enqueue.csv ubench_single_${IMAGE}_check.csv ubench_single_${IMAGE}_time.csv
#./helper_image.sh 1 ubench_single_${IMAGE}_memory.csv '--gpu_method 2 --perf_stat 4'

./helper_image.sh ubench_multi_${IMAGE}_enqueue.csv '-c 1 --gpu_method 0 --perf_stat 1'
./helper_image.sh ubench_multi_${IMAGE}_check.csv '-c 1 --gpu_method 0 --perf_stat 2'
./combineRuntimes.py ubench_multi_${IMAGE}_enqueue.csv ubench_multi_${IMAGE}_check.csv ubench_multi_${IMAGE}_time.csv
#./helper_image.sh 1 ubench_multi_${IMAGE}_memory.csv '--gpu_method 0 --perf_stat 4'

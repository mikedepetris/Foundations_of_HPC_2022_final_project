#var=$(date +"%FORMAT_STRING")
now=$(date +"%m_%d_%Y")
echo "${now}"
now=$(date +"%Y-%m-%d")
echo "${now}"
now=$(date +"%Y-%m-%d_%H-%M-%S")
echo "${now}"
csvname=dgemm_mkl_epyc_$now.csv
echo "${csvname}"

#!/bin/bash
input="/root/input_vectors.txt"
output="/root/output_results.txt"
data="/sys/class/fe_DPRAM_248/fe_DPRAM_248/control_data"
address="/sys/class/fe_DPRAM_248/fe_DPRAM_248/control_address"
wr_en="/sys/class/fe_DPRAM_248/fe_DPRAM_248/control_wr_en"
size=$(stat -c%s "$input")

echo "The size of $input is $size bytes."

echo "This file shows the data values and address locations of the DPRAM component." > "$output"

mapfile -t a < "$input"
for i in $(seq ${#a[*]}); do
	echo "$(($i - 1))" > "$address"
	echo 1 > "$wr_en"
	echo "${a[$(($i - 1))]}" > "$data"
	echo 0 > "$wr_en"
	echo -e "Address Location : $(($i - 1)) \t\t Data value : ${a[$(($i - 1))]}" >> "$output"
done

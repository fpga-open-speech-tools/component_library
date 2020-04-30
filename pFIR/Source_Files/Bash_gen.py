'''
Bash_gen.py
Author : Joshua Harthan
This script generates a bash script that reads in a input file, and writes each value to the corresponding register for the DPRAM component
'''
n=512 # Change this value corresponding to the amount of registers for the DPRAM component

file=open("bash.sh", "+w") # Change "bash.sh" to appropriate name of your choosing
file.write("#!/bin/bash\n")
file.write("input=/root/pFIR/hpf_coefficients.txt\n") # Change the file path location corresponding to the input file to be read in
for i in range(n):
   ''' "fe_DPRAM_248" indicates a DPRAM component with major number 248; the major number "248" will change depending on the Linux kernel being ran. 
   Check /sys/class and change the major number in this script in accordance with the major number associated with the DPRAM component, i.e. fe_DPRAM_xxx
   '''
   file.write("register%d" % i +"=\"/sys/class/fe_DPRAM_248/fe_DPRAM_248/register%d\"\n" % i)
file.write("\n")
file.write("echo \"Programming the FIR filter...\"\n")
file.write("mapfile -t a < \"$input\"\n")
for i in range(n):
   file.write("echo \"$((16#${a[%d]}))\"," % i + "| tee \"$register%d\" > /dev/null;\n" % i ) # '16#' indicates the input values within the input file are in hex; change this accordingly
file.write("\n")
file.write("echo \"FIR filter successfully programmed!\"")
file.close()
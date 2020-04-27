#!/bin/bash
enable="/sys/class/fe_pFIR_247/fe_pFIR_247/enable"

echo "Turning off the FIR filter..."

echo 0 > "$enable";

echo "FIR filter turned off!"

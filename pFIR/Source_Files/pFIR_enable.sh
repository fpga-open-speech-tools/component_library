#!/bin/bash
enable="/sys/class/fe_pFIR_247/fe_pFIR_247/enable"

echo "Turning on the FIR filter..."

echo "((16#$10000000))" > "$enable";

echo "FIR filter turned on!"

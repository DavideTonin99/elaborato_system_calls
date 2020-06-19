#!/bin/bash

args=("$@")

SCRIPT_NAME=$(basename ${BASH_SOURCE[0]})

if [ "$#" != 4 ]; then
    echo -e "\e[31mERROR: $SCRIPT_NAME takes 4 params: n_devices n_righe board_rows board_cols\e[0m"
    return
fi

N_DEVICES=${args[0]}
N_RIGHE=${args[1]}
BOARD_ROWS=${args[2]}
BOARD_COLS=${args[3]}

echo "<$SCRIPT_NAME> Creating position file..."

rm -f input/file_posizioni.txt

for (( row = 0; row < N_RIGHE; row++ ))
do
    pos=""
    for (( d = 0; d < N_DEVICES; d++ ))
    do
        rand_row=$(((RANDOM % ${BOARD_ROWS})))
        rand_col=$(((RANDOM % ${BOARD_COLS})))
        pos+="${rand_row},${rand_col}|"
    done
    pos=${pos:0:${#pos}-1}
    echo "${pos}" >> input/file_posizioni.txt

done
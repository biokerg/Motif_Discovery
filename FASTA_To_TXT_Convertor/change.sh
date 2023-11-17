#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Error: Invalid number of arguments."
    echo "Usage: ./script.sh [input_file] [output_file]"
    exit 1
fi

input_file=$1
output_file=$2

output=""
while IFS= read -r line || [[ -n $line ]]; do
    line=$(echo "$line" | tr -d '\r')
    if [[ "$line" == ">sp|"* ]]; then
        output+="\n"
    else
        output+="$line"
    fi
done < "$input_file"

printf "%b\n" "${output}" > "$output_file"

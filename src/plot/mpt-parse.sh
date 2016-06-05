#!/bin/bash

if [[ -z $1 ]] ; then
	echo "You must inform the PID"
	
	exit 1
fi

echo /dev/null > report-$1.csv
for file in mpt-receiver-$1 ; do 
	cat mpt-receiver-$1* | grep latency | sed 's/\[STAT\]: //' | sed 's/received:/received;/g' >> report-$1.csv
done


plot_input_file=`pwd`/report-$1.csv

gnuplot -e "filename='$plot_input_file';output_filename='report-$1.png'" `dirname $0`/latency.ps
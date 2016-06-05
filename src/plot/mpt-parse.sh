#!/bin/bash

if [[ -z $1 ]] ; then
	echo "You must inform the PID"
	
	exit 1
fi


mkdir -p $1

receiver_latency_report=$1/latency-report.csv
receiver_throughput_report=$1/throughput-report.csv


echo /dev/null > $receiver_latency_report
for file in mpt-receiver-$1 ; do 
	cat mpt-receiver-$1* | grep latency | sed 's/\[STAT\]: //' | sed 's/received:/received;/g' >> $receiver_latency_report
	cat mpt-receiver-$1*.log  | grep STAT | grep rate | sed 's/\[STAT\]: //' >> $receiver_throughput_report
done


receiver_latency_report_in=`pwd`/$receiver_latency_report
receiver_latency_report_out=`pwd`/$1/latency-report.png
receiver_throughput_report_in=`pwd`/$receiver_throughput_report
receiver_throughput_report_out=`pwd`/$1/throughput-report-$1.png


gnuplot -e "filename='$receiver_latency_report_in';output_filename='$receiver_latency_report_out'" `dirname $0`/latency.ps
gnuplot -e "filename='$receiver_throughput_report_in';output_filename='$receiver_throughput_report_out'" `dirname $0`/throughput.ps
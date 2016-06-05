#!/bin/sh

if [[ -z $1 ]] ; then
	echo "You must inform either 'receiver' or 'sender'"
	
	exit 1
fi

if [[ -z $2 ]] ; then
	echo "You must inform the PID"
	
	exit 1
fi

mkdir -p $2


if [[ "$1" == "receiver" ]] ; then
	receiver_latency_report=$2/$1-latency-report.csv
	receiver_throughput_report=$2/$1-throughput-report.csv


	echo /dev/null > $receiver_latency_report
	echo /dev/null > $receiver_throughput_report
	for file in mpt-$1-$2 ; do 
		cat mpt-$1-$2* | grep latency | sed 's/\[STAT\]: //' | sed 's/received:/received;/g' >> $receiver_latency_report
		cat mpt-$1-$2*.log  | grep STAT | grep rate | sed 's/\[STAT\]: //' >> $receiver_throughput_report
	done


	receiver_latency_report_in=`pwd`/$receiver_latency_report
	receiver_latency_report_out=`pwd`/$2/receiver-latency-report.png
	receiver_throughput_report_in=`pwd`/$receiver_throughput_report
	receiver_throughput_report_out=`pwd`/$2/receiver-throughput-report.png


	gnuplot -e "filename='$receiver_latency_report_in';output_filename='$receiver_latency_report_out'" `dirname $0`/latency.ps
	gnuplot -e "filename='$receiver_throughput_report_in';output_filename='$receiver_throughput_report_out'" `dirname $0`/throughput.ps
	
	cp `dirname $0`/receiver-report.html `pwd`/$2/index.html
else
	if [[ $1 == "sender" ]] ; then
		sender_throughput_report=$2/$1-throughput-report.csv


		echo /dev/null > $sender_throughput_report
		for file in mpt-$1-$2 ; do 
			cat mpt-$1-$2*.log  | grep STAT | grep rate | sed 's/\[STAT\]: //' >> $sender_throughput_report
		done

		sender_throughput_report_in=`pwd`/$sender_throughput_report
		sender_throughput_report_out=`pwd`/$2/sender-throughput-report.png

		gnuplot -e "filename='$sender_throughput_report_in';output_filename='$sender_throughput_report_out'" `dirname $0`/throughput.ps
		cp `dirname $0`/sender-report.html `pwd`/$2/index.html
			
	else
		echo "Invalid option $1"
	fi
fi
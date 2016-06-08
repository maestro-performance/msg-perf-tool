#!/bin/bash

share_dir=`dirname $0`/../share/mpt


ARGS=$(getopt -o l:s:r:o:n:s:h -n "$0" -- "$@");
eval set -- "$ARGS";

HELP="USAGE: ./$0 [options]\n
-l 'logdir'	-- log directory\n
-s 'sender pid'	-- sender PID\n
-r 'receiver pid'	-- receiver PID\n
-o 'output directory'	-- output directory for the test report\n
-n 'test name'	-- test name\n
-s 'size'	-- message size used in the test\n
-h									-- this help"

while true; do
	case "$1" in
		-l)
			shift
			export LOG_DIR="$1"
			shift
		;;
		-s)
			shift
			export SENDER_PID="$1"
			shift
		;;
		-r)
			shift
			export RECEIVER_PID="$1"
			shift
		;;
		-o)
			shift
			export OUTPUT_DIR="$1"
			shift
		;;
		-n)
			shift
			export TEST_NAME="$1"
			shift
		;;
		-s)
			shift
			export MESSAGE_SIZE="$1"
			shift
		;;
		-h)
			shift
			echo -e ${HELP}
			exit 0
		;;
		--)
			shift
			break
		;;
	esac
done

export test_name_dir=${TEST_NAME//[[:space:]]/-}/`date +%Y%m%d%H%M`

echo "Creating the report directory at ${OUTPUT_DIR}/${test_name_dir}"
mkdir -p ${OUTPUT_DIR}/${test_name_dir}/

function process_receiver_data() {
	echo "Processing the receiver data"
	receiver_latency_report=${OUTPUT_DIR}/${test_name_dir}/receiver-latency-report.csv
	receiver_throughput_report=${OUTPUT_DIR}/${test_name_dir}/receiver-throughput-report.csv
	receiver_summary_report=${OUTPUT_DIR}/${test_name_dir}/receiver-summary-report.csv

	total_count_receive=0
	num_receivers=0
	file_list=""
	for file in $LOG_DIR/mpt-receiver-${RECEIVER_PID}*.log ; do
		let num_receivers=num_receivers+1
		cat ${file} | grep latency | sed 's/\[STAT\]: //' >> ${receiver_latency_report}_${num_receivers}
		cat ${file}	| grep STAT | grep -v "summary" | grep rate | sed 's/\[STAT\]: //' >> ${receiver_throughput_report}_${num_receivers}
		summary_line=`cat $LOG_DIR/mpt-receiver-${RECEIVER_PID}* | grep summary`
    echo "Processing file ${file}"

    msg_count=`echo $summary_line | awk -F ';' ' { print $3 }'`

		let total_count_receive=total_count_receive+msg_count

	done

 	echo "Processing the summary"
	test_date=`date`

	receiver_latency_report_in=$receiver_latency_report
	receiver_latency_report_out=${OUTPUT_DIR}/${test_name_dir}/receiver-latency-report.png
	receiver_throughput_report_in=$receiver_throughput_report
	receiver_throughput_report_out=${OUTPUT_DIR}/${test_name_dir}/receiver-throughput-report.png

	gnuplot -e "basename='$receiver_latency_report_in';output_filename='$receiver_latency_report_out';count='$num_receivers'" ${share_dir}/latency.ps
	gnuplot -e "basename='$receiver_throughput_report_in';output_filename='$receiver_throughput_report_out';num_connections='$num_receivers'" ${share_dir}/throughput.ps

	export total_count_receive
	export num_receivers

	echo "Compressing receiver reports"
	tar --gzip -cf ${receiver_latency_report}.tar.gz ${receiver_latency_report}_*
	tar --gzip -cf ${receiver_throughput_report}.tar.gz ${receiver_throughput_report}_*
}


function process_sender_data() {
	echo "Processing the sender data"
	sender_throughput_report=${OUTPUT_DIR}/${test_name_dir}/sender-throughput-report.csv

	total_count_sent=0
	num_senders=0

	for file in $LOG_DIR/mpt-sender-${SENDER_PID}*.log ; do
		let num_senders=num_senders+1
		cat ${file}	| grep STAT | grep rate | sed 's/\[STAT\]: //' >> ${sender_throughput_report}_${num_senders}

		summary_line=`cat $LOG_DIR/mpt-sender-${SENDER_PID}* | grep summary`
		msg_count=`echo $summary_line | awk -F ';' ' { print $3 }'`

		let total_count_sent=total_count_sent+msg_count

	done

	test_date=`date`
	sender_throughput_report_in=$sender_throughput_report
	sender_throughput_report_out=${OUTPUT_DIR}/${test_name_dir}//sender-throughput-report.png

	gnuplot -e "basename='$sender_throughput_report_in';output_filename='$sender_throughput_report_out';num_connections='$num_senders'" ${share_dir}/throughput.ps

	export total_count_sent
	export num_senders

	echo "Compressing sender reports"
	tar --gzip -cf ${sender_throughput_report}.tar.gz ${sender_throughput_report}_*
}

function process_template() {
	let count_diff=total_count_sent-total_count_receive

	env_data=`screenfetch -N`
	echo "{}" | jinja2 -D test_date="$test_date" -D environment="$env_data" \
			-D total_count_sent="$total_count_sent" \
			-D total_count_receive="$total_count_receive" \
			-D count_diff="$count_diff" \
			-D num_senders="$num_senders" \
			-D num_receivers="$num_receivers" \
			-D message_size="$MESSAGE_SIZE" \
			${share_dir}/report.html > ${OUTPUT_DIR}/${test_name_dir}/index.html
}

process_sender_data
process_receiver_data
process_template

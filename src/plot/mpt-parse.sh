#!/bin/bash

share_dir=`dirname $0`/../share/mpt


ARGS=$(getopt -o l:s:r:o:n:h -n "$0" -- "$@");
eval set -- "$ARGS";

HELP="USAGE: ./$0 [options]\n
-l 'logdir'  -- log directory\n
-s 'sender pid'  -- sender PID\n
-r 'receiver pid'  -- receiver PID\n
-o 'output directory'  -- output directory for the test report\n
-n 'test name'  -- test name\n
-h                  -- this help"

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

export test_name_dir=${TEST_NAME//[[:space:]]/-}

echo "Creating the report directory at ${OUTPUT_DIR}/${test_name_dir}"
mkdir -p ${OUTPUT_DIR}/${test_name_dir}

function process_receiver_data() {
	echo "Processing the receiver data"
	receiver_latency_report=${OUTPUT_DIR}/${test_name_dir}/receiver-latency-report.csv
	receiver_throughput_report=${OUTPUT_DIR}/${test_name_dir}/receiver-throughput-report.csv
	receiver_summary_report=${OUTPUT_DIR}/${test_name_dir}/receiver-summary-report.csv

	cat /dev/null > $receiver_latency_report
	cat /dev/null > $receiver_throughput_report

	total_count_receive=0;
	for file in $LOG_DIR/mpt-receiver-${RECEIVER_PID} ; do
		cat $LOG_DIR/mpt-receiver-${RECEIVER_PID}* | grep latency | sed 's/\[STAT\]: //' | sed 's/received:/received;/g' >> $receiver_latency_report
		cat $LOG_DIR/mpt-receiver-${RECEIVER_PID}*.log  | grep STAT | grep -v "summary" | grep rate | sed 's/\[STAT\]: //' >> $receiver_throughput_report
		summary_line=`cat $LOG_DIR/mpt-receiver-${RECEIVER_PID}* | grep summary`

		msg_count=`echo $summary_line | awk -F ';' ' { print $3 }'`

		let total_count_receive=total_count_receive+msg_count
	done

 	echo "Processing the summary"
  test_date=`date`

  receiver_latency_report_in=$receiver_latency_report
	receiver_latency_report_out=${OUTPUT_DIR}/${test_name_dir}/receiver-latency-report.png
	receiver_throughput_report_in=$receiver_throughput_report
	receiver_throughput_report_out=${OUTPUT_DIR}/${test_name_dir}/receiver-throughput-report.png

	gnuplot -e "filename='$receiver_latency_report_in';output_filename='$receiver_latency_report_out'" ${share_dir}/latency.ps
	gnuplot -e "filename='$receiver_throughput_report_in';output_filename='$receiver_throughput_report_out'" ${share_dir}/throughput.ps

	export total_count_receive
}


function process_sender_data() {
	echo "Processing the sender data"
	sender_throughput_report=${OUTPUT_DIR}/${test_name_dir}/sender-throughput-report.csv

	total_count_sent=0;
	cat /dev/null > $sender_throughput_report
	for file in $LOG_DIR/mpt-sender-${SENDER_PID} ; do
		cat $LOG_DIR/mpt-sender-${SENDER_PID}*.log  | grep STAT | grep rate | sed 's/\[STAT\]: //' >> $sender_throughput_report

		summary_line=`cat $LOG_DIR/mpt-sender-${SENDER_PID}* | grep summary`
		msg_count=`echo $summary_line | awk -F ';' ' { print $3 }'`

		let total_count_sent=total_count_sent+msg_count
	done

	test_date=`date`
	sender_throughput_report_in=$sender_throughput_report
	sender_throughput_report_out=${OUTPUT_DIR}/${test_name_dir}//sender-throughput-report.png

	gnuplot -e "filename='$sender_throughput_report_in';output_filename='$sender_throughput_report_out'" ${share_dir}/throughput.ps

	export total_count_sent
}

function process_template() {
	env_data=`screenfetch -N`
	echo "{}" | jinja2 -D test_date="$test_date" -D environment="$env_data" \
			-D total_count_sent="$total_count_sent" \
			-D total_count_receive="$total_count_receive" \
			${share_dir}/report.html > ${OUTPUT_DIR}/${test_name_dir}/index.html
}

process_sender_data
process_receiver_data
process_template

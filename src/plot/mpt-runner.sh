#!/bin/bash

# #####
# LOG_DIR=/tmp/log
# BROKER_URL="amqp://<hostname>:5672/test.performance.queue"
# PARALLEL_COUNT=4
# DURATION=1
# MESSAGE_SIZE=1024

app_path=`dirname $0`

[[ ! -d $LOG_DIR ]] && mkdir -p $LOG_DIR

echo "Broker URL: $BROKER_URL"

echo "Lauching the receiver"
pid_receiver=`${app_path}/mpt-receiver -b $BROKER_URL --log-level=STAT --duration=$DURATION -p $PARALLEL_COUNT --logdir=$LOG_DIR --daemon`

echo "Lauching the sender"
pid_sender=`${app_path}/mpt-sender -b $BROKER_URL --log-level=STAT --duration $DURATION -p $PARALLEL_COUNT --logdir=$LOG_DIR -s 1024 --daemon`

# Sleeps for a little longer than the test duration so that it gives some time
# for the program to finish and flush data

echo "Sleeping for ${DURATION}m15s"
sleep 15s "${DURATION}m"

report_path=`mktemp --tmpdir -d mpt.XXXXXXX`

echo "Parsing the receiver data"
${app_path}/mpt-parse.sh receiver $pid_receiver $report_path

echo "Parsing the sender data"
${app_path}/mpt-parse.sh sender $pid_sender $report_path


if [[ ! -z "${UPLOAD_URL}" ]] ; then
    echo "Copying the files"
    scp -r ${report_path}/* ${UPLOAD_URL}
fi


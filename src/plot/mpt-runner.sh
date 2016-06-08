#!/bin/bash

# #####
# LOG_DIR=/tmp/log
# BROKER_URL="amqp://<hostname>:5672/test.performance.queue"
# PARALLEL_COUNT=4
# DURATION=1
export MESSAGE_SIZE=1024

app_path=`dirname $0`

ARGS=$(getopt -o l:b:d:s:p:u:h -n "$0" -- "$@");
eval set -- "$ARGS";

HELP="USAGE: ./$0 [options]\n
-l 'logdir'  -- log directory\n
-b 'broker-url'  -- broker url (ie: amqp://hostname:5672/queue.name)\n
-d 'duration'  -- test duration (in minutes)\n
-s 'size'  -- message size (in bytes [default = 1024])\n
-p 'parallel count'  -- the number of parallel senders and consumers\n
-u 'upload url'  -- (optional) a SCP URL for uploading the test data\n
-h                  -- this help"

while true; do
  case "$1" in
    -l)
      shift
      export LOG_DIR="$1"
      shift
    ;;
    -b)
      shift
      export BROKER_URL="$1"
      shift
    ;;
    -d)
      shift
      export DURATION="$1"
      shift
    ;;
    -s)
      shift
      export MESSAGE_SIZE="$1"
      shift
    ;;
    -p)
      shift
      export PARALLEL_COUNT="$1"
      shift
    ;;
    -u)
      shift
      export UPLOAD_URL="$1"
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

function stop_test() {
  echo "Stopping the test"

  killall -TERM mpt-receiver
  killall -TERM mpt-sender

  exit
}

# Explanation: we trap the exit/kill/termination of the script, so that if the
# job is aborted, we release the allocated nodes
trap stop_test SIGINT SIGTERM


[[ ! -d $LOG_DIR ]] && mkdir -p $LOG_DIR

echo "Broker URL: $BROKER_URL"

echo "Lauching the receiver"
export pid_receiver=`${app_path}/mpt-receiver -b $BROKER_URL --log-level=STAT --duration=$DURATION -p $PARALLEL_COUNT --logdir=$LOG_DIR --daemon`

echo "Lauching the sender"
export pid_sender=`${app_path}/mpt-sender -b $BROKER_URL --log-level=STAT --duration $DURATION -p $PARALLEL_COUNT --logdir=$LOG_DIR -s $MESSAGE_SIZE --daemon`

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

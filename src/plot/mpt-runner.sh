#!/bin/bash

export MESSAGE_SIZE=1024
export SSH_OPTS=""

app_path=`dirname $0`

ARGS=$(getopt -o l:b:d:s:p:u:r:o:n:t:h -n "$0" -- "$@");
eval set -- "$ARGS";

HELP="USAGE: ./$0 [options]\n
-l 'logdir'  -- log directory\n
-b 'broker-url'  -- broker url (ie: amqp://hostname:5672/queue.name)\n
-d 'duration'  -- test duration (in minutes)\n
-s 'size'  -- message size (in bytes [default = 1024])\n
-p 'parallel count'  -- the number of parallel senders and consumers\n
-u 'upload url'  -- (optional) a SCP URL for uploading the test data\n
-r 'remote plot server'  -- (optional) a SSH/SCP URL for plotting the test data in a remote server \n
-o 'output directory'  -- output directory for the test report\n
-n 'test name'  -- test name\n
-t 'throttle'  -- throttle (sends messages in a fixed rate [ msgs per second per connection])\n
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
    -r)
      shift
      export REMOTE_PLOT_SERVER="$1"
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
    -t)
      shift
      export THROTTLE="$1"
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
if [[ -z THROTTLE="$1" ]] ; then
  export THROTTLE="0"
else
  echo "Throttling the sender"
fi

echo "Lauching the receiver"
export pid_receiver=`${app_path}/mpt-receiver -b $BROKER_URL --log-level=STAT --duration=$DURATION -p $PARALLEL_COUNT --logdir=$LOG_DIR -s $MESSAGE_SIZE --daemon`

echo "Lauching the sender"
export pid_sender=`${app_path}/mpt-sender -b $BROKER_URL -t $THROTTLE --log-level=STAT --duration $DURATION -p $PARALLEL_COUNT --logdir=$LOG_DIR -s $MESSAGE_SIZE --daemon`

# Sleeps for a little longer than the test duration so that it gives some time
# for the program to finish and flush data

echo "Sleeping for ${DURATION}m15s"
sleep 15s "${DURATION}m"

if [[ -z $OUTPUT_DIR ]] ; then
  OUTPUT_DIR=`mktemp --tmpdir -d mpt.XXXXXXX`
fi


report_dir=${LOG_DIR}/report/
test_name_dir=${TEST_NAME//[[:space:]]/-}/
if [[ ! -z "${REMOTE_PLOT_SERVER}" ]] ; then
  remote_data=/tmp/mpt
  remote_log_dir=${remote_data}/log
  remote_report_dir=${remote_data}/report

  echo "Creating remote directory ${remote_log_dir}"
  echo ssh ${SSH_OPTS} ${REMOTE_PLOT_SERVER} mkdir -p ${remote_log_dir}
  ssh ${REMOTE_PLOT_SERVER} mkdir -p ${remote_log_dir}

  echo "Copying sender files to ${REMOTE_PLOT_SERVER}:${remote_log_dir}"
  scp -r ${LOG_DIR}/mpt-sender-${pid_sender}* ${REMOTE_PLOT_SERVER}:${remote_log_dir}

  echo "Copying receiver files to ${REMOTE_PLOT_SERVER}:${remote_log_dir}"
  scp -r ${LOG_DIR}/mpt-receiver-${pid_receiver}* ${REMOTE_PLOT_SERVER}:${remote_log_dir}

  echo "Plotting the data remotely"
  ssh ${REMOTE_PLOT_SERVER} "mpt-parse.sh -l ${remote_log_dir} -s $pid_sender -r $pid_receiver -n \"${TEST_NAME}\" -o ${remote_report_dir}"

  echo "Copying generated data"
  mkdir -p ${report_dir}
  scp -r ${REMOTE_PLOT_SERVER}:${remote_report_dir}/${test_name_dir} ${report_dir}
else
  echo "Parsing the receiver data"

  ${app_path}/mpt-parse.sh -l ${LOG_DIR} -s $pid_sender -r $pid_receiver -n "${TEST_NAME}" -o ${report_dir}

fi


if [[ ! -z "${UPLOAD_URL}" ]] ; then
    echo "Copying the files to ${UPLOAD_URL}"
    scp ${SSH_OPTS} -r ${report_dir}/${test_name_dir} ${UPLOAD_URL}/
fi

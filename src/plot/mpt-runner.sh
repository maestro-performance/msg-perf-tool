#!/bin/bash

export MESSAGE_SIZE=1024
export SSH_OPTS=""

app_path=`dirname $0`

ARGS=$(getopt -o l:b:d:c:s:p:u:r:o:n:t:h -n "$0" -- "$@");
eval set -- "$ARGS";

HELP="USAGE: ./$0 [options]\n
-l 'logdir'  -- log directory\n
-b 'broker-url'  -- broker url (ie: amqp://hostname:5672/queue.name)\n
-d 'duration'  -- test duration (in minutes)\n
-c 'count'  -- message count\n
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
    -c)
      shift
      export COUNT="$1"
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

  killall -INT mpt-receiver
  killall -INT mpt-sender

  killall -TERM mpt-receiver || echo ""
  killall -TERM mpt-sender || echo ""

  exit
}

# Explanation: we trap the exit/kill/termination of the script, so that if the
# job is aborted, we release the allocated nodes
trap stop_test SIGINT SIGTERM


if [[ -z "$BROKER_URL" ]] ; then
  echo -e "Broker is a required option (-b)\n"
  echo -e ${HELP}
  exit 1
fi

if [[ -z "$DURATION" ]] ; then
  if [[ -z "$COUNT" ]] ; then
    echo -e "Either the test duration or the message count should be informed (-d or -c)\n"
    echo -e ${HELP}
    exit 1
  fi
fi

if [[ -z "$PARALLEL_COUNT" ]] ; then
  echo -e "Parallel count is a required option (-p)\n"
  echo -e ${HELP}
  exit 1
fi

if [[ -z "$LOG_DIR" ]] ; then
  echo -e "Log dir is a required option (-l)\n"
  echo -e ${HELP}
  exit 1
fi

[[ ! -d $LOG_DIR ]] && mkdir -p $LOG_DIR

echo "Broker URL: $BROKER_URL"
if [[ -z THROTTLE="$1" ]] ; then
  export THROTTLE="0"
else
  echo "Throttling the sender"
fi

function run_by_duration() {
  echo "Lauching the receiver"
  export pid_receiver=`${app_path}/mpt-receiver -b $BROKER_URL --log-level=STAT --duration=$DURATION -p $PARALLEL_COUNT --logdir=$LOG_DIR -s $MESSAGE_SIZE --daemon`

  echo "Lauching the sender"
  export pid_sender=`${app_path}/mpt-sender -b $BROKER_URL -t $THROTTLE --log-level=STAT --duration $DURATION -p $PARALLEL_COUNT --logdir=$LOG_DIR -s $MESSAGE_SIZE --daemon`

  # Sleeps for a little longer than the test duration so that it gives some time
  # for the program to finish and flush data

  echo "Sleeping for ${DURATION}m15s"
  sleep 15s "${DURATION}m"
}


function run_by_count() {
  echo "Lauching the receiver "
  export pid_receiver=`${app_path}/mpt-receiver -b $BROKER_URL --log-level=STAT -p $PARALLEL_COUNT --logdir=$LOG_DIR -s $MESSAGE_SIZE --daemon`
  if [[ -z "${pid_receiver}" ]] ; then
    echo "Invalid PID for the receiver: ${pid_receiver}"
    exit 1
  fi

  echo "Lauching the sender and waiting for it to send ${COUNT} messages"
  export pid_sender=`${app_path}/mpt-sender -b $BROKER_URL -t $THROTTLE --log-level=STAT --count $COUNT -p $PARALLEL_COUNT --logdir=$LOG_DIR -s $MESSAGE_SIZE --daemon`
  if [[ -z "${pid_sender}" ]] ; then
    echo "Invalid PID for the sender: ${pid_sender}"
    exit 1
  fi

  local is_sender_running=$(ps -ef | grep mpt-sender | grep -v grep | wc -l)
  local elapsed=0
  while [[ "${is_sender_running}" -ne 0 ]] ; do
      echo -en "\rWaiting for the sender process to finish: ${elapsed}s elapsed"
      sleep 1s
      is_sender_running=$(ps -ef | grep mpt-sender | grep -v grep | wc -l)
      let elapsed=elapsed+1
  done
  echo -e "\nThe sender has finished sending the messages."

  echo "Waiting 30s for the receiver to catch up"
  sleep 30s

  echo "Stopping the receiver with PID ${pid_receiver}"
  killall -INT mpt-receiver


  local is_receiver_running=$(ps -ef | grep mpt-receiver | grep -v grep | wc -l)
  wait_count=0
  while [[ "${is_receiver_running}" -ne 0 ]] ; do
      echo -en "\rWaiting for the receiver process to finish"
      sleep 10s
      is_receiver_running=$(ps -ef | grep mpt-receiver | grep -v grep | wc -l)
      let wait_count=wait_count+1

      if [[ wait_count -eq 10 ]] ; then
        echo -e "\nTest error: the receiver is taking too long to flush its data"
        killall -TERM mpt-receiver
      fi
  done
  echo ""
}


start_time=$(date '+%Y%-m-%d %H:%M:%S')
echo "Test start time: ${start_time}"
if [[ ! -z "$DURATION" ]] ; then
  run_by_duration
else

  if [[ ! -z "$COUNT" ]] ; then
    run_by_count
  else
    echo "Either the test duration or the message count should be informed (-d or -c)"
    exit 1
  fi
fi
end_time=$(date '+%Y%-m-%d %H:%M:%S')
echo "Test end time: ${end_time}"

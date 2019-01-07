#!/bin/bash

pid=$(pgrep -f mpt-worker-daemon)

if [[ ! -z "${pid}" ]] ; then
    kill -TERM $pid
    if [[ $? -eq 0 ]] ; then
        echo "Terminated PID $pid"
    else
        echo "Unable to terminate PID $pid"
        return 1
    fi
else
    echo "Maestro is not running"
fi
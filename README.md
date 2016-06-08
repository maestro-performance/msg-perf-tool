MPT: messaging performance tool
============


Dependencies:
----
Runtime/Compilation: cmake qpid-proton-c-devel gcc gcc-c++
Parsing and plotting: gnuplot, screenfetch and jinja2-cli


Recommended:
valgrind perf

Broker Settings: ActiveMQ
----


ActiveMQ may need to have the inactivity monitor disabled. It can be done by
adding the following setting in the conf/activemq.xml, in the transport connector
setting:

```
transport.useInactivityMonitor=false
```

For example:

```
<transportConnector name="amqp" uri="amqp://0.0.0.0:5672?maximumConnections=1000&amp;wireFormat.maxFrameSize=104857600&amp;transport.useInactivityMonitor=false"/>
```


Usage:
----

Here's an example of how to run a 10 minute load test, with 4 concurrent senders,
4 concurrent receivers, sending 256 bytes of data per message. After test, the
performance report is uploaded via SCP to the server pointed by <user>@<host>:<path>

mpt-runner.sh -l /tmp/log -b amqp://<amqp server>:5672/<queue name> -d 10 -p 4 -s 256 -u <user>@<host>:<path> -n "sample test"

It's possible to have more complex deployment scenarios by running the sender and receiver separately. In this case, you have to
run the test steps manually:

Run the receiver (the controller will print the PID, please take note of that):
mpt-receiver -b amqp://<amqp server>:5672/<queue name> --log-level=stat -d 10 -p 4 --logdir=/tmp/log --daemon

Run the sender (the controller will print the PID, please take note of that):
mpt-sender -b amqp://<amqp server>:5672/<queue name> --log-level=stat -d 10 -p 4 --logdir=/tmp/log --daemon


After the test is complete:
mpt-parse.sh -l /tmp/log -s <sender PID> -r <receiver PID> -n "<sample test>" -o /tmp/log/report

At the moment, the data for both sender and receiver must be on the same path. If you are running it distributed, please copy the logs before running the parse.

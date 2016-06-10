MPT: messaging performance tool
============


Dependencies:
----
Runtime/Compilation: cmake qpid-proton-c-devel gcc gcc-c++
Parsing and plotting: gnuplot (>= 4.6), screenfetch and jinja2-cli


Recommended:
valgrind perf

Requirements
----

The clients may generate a lot of data depending on how much messages are sent 
per second. On my baseline system (two servers with Quad-Core AMD Opteron 2376 @ 8x 2.3GHz)
on a gigabit network, it generates around 1Gb of data per hour.


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

```
mpt-runner.sh -l /tmp/log -b amqp://<amqp server>:5672/<queue name> -d 10 -p 4 -s 256 -u <user>@<host>:<path> -n "sample test"
```

It's possible to have more complex deployment scenarios by running the sender and receiver separately. In this case, you have to
run the test steps manually:

Run the receiver (the controller will print the PID, please take note of that):
```
mpt-receiver -b amqp://<amqp server>:5672/<queue name> --log-level=stat -d 10 -p 4 --logdir=/tmp/log --daemon
```

Run the sender (the controller will print the PID, please take note of that):

```
mpt-sender -b amqp://<amqp server>:5672/<queue name> --log-level=stat -d 10 -p 4 --logdir=/tmp/log --daemon
```

After the test is complete:
```
mpt-parse.sh -l /tmp/log -s <sender PID> -r <receiver PID> -n "<sample test>" -o /tmp/log/report
```


For some systems combinations, it may be necessary to generate the performance reports in another host (i.e: the system running client 
does not have a newer [>= 4.6] version of gnuplot). In this case, it is possible to request the plotting data to be generated in a different system:

```
mpt-runner.sh -l /tmp/log -t 1000 -b amqp://<broker>:5672/<queue> -d 120 -p 2 -s 256 -u <user>@<host>:<path> -n "development tests" -r <user>@<host>
```

This uses a fixed path (/tmp/mpt) on the remote server, so, please, make sure to sufficient disk space.


At the moment, the data for both sender and receiver must be on the same path. If you are running it distributed, please copy the logs before running the parse.

Tips
----

* Run the clients and the broker in different servers
* Make sure that the time is properly set on both servers
* Run on an dedicated network (or, at least, avoid hours of peak usage)
* Measure the network performance before running (ie.: use iperf)
* Ideally, you should run at a fixed rate instead of flooding the brokers. 
Brokers tend to get slower as the queue size increase.


License
----

The code is licensed under Apache License v2
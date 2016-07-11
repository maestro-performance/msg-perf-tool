#!/usr/bin/env python
__author__ = 'opiske'

import optparse
import sys
import json
import csv
import requests
from requests.auth import HTTPBasicAuth
from StringIO import StringIO
import logging

fmt_console = '[%(asctime)s] [%(levelname)s] %(name)s :: %(message)s'
datefmt_iso = '%Y-%m-%d %H:%M:%S,'

console_formatter = logging.Formatter(fmt_console, datefmt=datefmt_iso)

console_handler = logging.StreamHandler()  # Python 2.6
console_handler.setLevel(logging.ERROR)  # setting console level
console_handler.setFormatter(console_formatter)

logging.getLogger().addHandler(console_handler)
logging.getLogger().setLevel(logging.NOTSET)

logger = logging.getLogger(__name__)


def print_errors(http_response):
    logger.error("Error: %s" % http_response.status_code)
    logger.error("Text: %s" % http_response.content)


def call_service(in_opts, req_url, request_json, force_update=False):
    logger.debug("Connecting to %s" % (req_url))


    headers = {'Content-type': 'application/json'}

    username = in_opts["username"]
    password = in_opts["password"]

    is_update = in_opts["update"]

    if is_update and not force_update:
        answer = requests.put(req_url, headers=headers, data=request_json, verify=False,
                               auth=HTTPBasicAuth(username, password))
    else:
        answer = requests.post(req_url, headers=headers, data = request_json, verify=False,
                               auth=HTTPBasicAuth(username, password))

    if answer.status_code < 200 or answer.status_code >= 205:
        print_errors(answer)
        return 3

    return 0


def call_service_for_check(in_opts, req_url):
    logger.debug("Connecting to %s" % (req_url))


    headers = {'Content-type': 'application/json'}

    username = in_opts["username"]
    password = in_opts["password"]

    answer = requests.get(req_url, headers=headers, verify=False, auth=HTTPBasicAuth(username, password))
    return answer


def register(in_opts):
    base_url = in_opts["url"]
    in_sut_name = in_opts["sut_name"]
    in_sut_key = in_opts["sut_key"]
    in_sut_version = in_opts["sut_version"]

    if base_url is None:
        logger.error("Base URL is required")

        return 1

    if in_sut_name is None or in_sut_name == "":
        logger.error("The SUT name is required")
        return 1

    if in_sut_key is None or in_sut_key == "":
        logger.error("The SUT key is required")
        return 1

    if in_sut_version is None or in_sut_version == "":
        logger.error("The SUT version is required")
        return 1

    request_data = {
        "sut_name": in_sut_name,
        "sut_key": in_sut_key,
        "sut_version": in_sut_version,
    }

    logger.info("Request: %s" % (request_data))

    request_json = StringIO()
    json.dump(request_data, request_json)
    logger.info("JSON: %s" % (request_json.getvalue(),))

    req_url = "%s/sut/messaging" % (base_url)
    call_service(in_opts, req_url, request_json.getvalue())

    return 0

def configure_latency_mapping(in_opts):
    base_url = in_opts["url"]
    in_key = in_opts["sut_key"]

    answer = call_service_for_check(in_opts, ("%s/%s/_mapping" % (base_url, in_key)))
    if answer.status_code == 404:
        req_url = "%s/%s" % (base_url, in_key)
        request_json = '{ "mappings": { "latency": { "properties": { "creation": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss.SSS"} } } } }'
    else:
        req_url = "%s/%s/_mapping/latency" % (base_url, in_key)
        request_json = '{ "properties": { "creation": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss.SSS"} } }'


    ret = call_service(in_opts, req_url, request_json, force_update=True)
    if ret != 0:
        logger.error("Unable to configure the mappings for latency")

def configure_throughput_mapping(in_opts):
    base_url = in_opts["url"]
    in_key = in_opts["sut_key"]

    answer = call_service_for_check(in_opts, ( "%s/%s/_mapping" % (base_url, in_key)))
    if answer.status_code == 404:
        req_url = "%s/%s" % (base_url, in_key)
        request_json = '{ "mappings": { "throughput": { "properties": { "ts": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss"} } } } }'
    else:
        req_url = "%s/%s/_mapping/througput" % (base_url, in_key)
        request_json = '{ "properties": { "ts": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss"} } }'

    ret = call_service(in_opts, req_url, request_json, force_update=True)
    if ret != 0:
        logger.error("Unable to configure the mappings for throughput")



def count_lines(file):
    for i, l in enumerate(file):
      pass

    file.seek(0)
    return i + 1

def validate_parameters(in_opts):
    base_url = in_opts["url"]
    in_file_name = in_opts["filename"]
    in_type = in_opts["test_type"]
    in_sut = in_opts["sut"]
    in_key = in_opts["sut_key"]
    in_version = in_opts["sut_version"]
    in_testrun = in_opts["test_run"]
    in_direction = in_opts["msg_direction"]

    if base_url is None:
        logger.error("Base URL is required")

        return 1

    if in_file_name is None:
        logger.error("Input filename is required")

        return 1

    if in_type is None:
        logger.error("Type is required")

        return 1

    if in_sut is None:
        logger.error("SUT name is required")

        return 1

    if in_key is None:
        logger.error("Key is required")

        return 1

    if in_version is None:
        logger.error("Version is required")

        return 1

    if in_testrun is None:
        logger.error("Test run is required")

        return 1

    if in_direction is None:
        logger.error("Direction is required")

        return 1

    return 0

def load_receiver_latencies(in_opts):
    base_url = in_opts["url"]
    in_file_name = in_opts["filename"]
    in_type = in_opts["test_type"]
    in_sut = in_opts["sut"]
    in_key = in_opts["sut_key"]
    in_version = in_opts["sut_version"]
    in_test_run = in_opts["test_run"]
    in_direction = in_opts["msg_direction"]

    param_check = validate_parameters(in_opts)
    if param_check != 0:
        return param_check

    test_id = ("%s_%s" % (in_key, in_test_run))

    ret = call_service_for_check(in_opts, "%s/test/info/%s" % (base_url, test_id))
    if ret.status_code < 200 or ret.status_code >= 205:
        logger.error("There's no test with the ID %s. Please record that test info before loading data"
                     % test_id)

        return 1

    file = open(in_file_name, 'rb')
    num_lines = count_lines(file)

    configure_latency_mapping(in_opts)

    req_url = "%s/%s/latency" % (base_url, in_key)

    logger.info("There are %d lines to read" % (num_lines))
    csv_data = csv.reader(file, delimiter=';', quotechar='|')
    i = 0;
    for row in csv_data:
        i += 1
        latency = int(row[1])
        creation = row[3]

        request_data = {
            "type": in_type,
            "sut": in_sut,
            "version": in_version,
            "latency": latency,
            "creation": creation,
            "test_id": test_id,
            "direction": in_direction
        }

        request_json = StringIO()
        json.dump(request_data, request_json)
        # logger.info("JSON: %s" % (request_json.getvalue(),))
        sys.stdout.write("Request %d of %d\r" % (i, num_lines))

        # TODO: check if the key exists before calling the service
        call_service(in_opts, req_url, request_json.getvalue())

    print ""

    return 0

def load_receiver_throughput(in_opts):
    base_url = in_opts["url"]
    in_file_name = in_opts["filename"]
    in_type = in_opts["test_type"]
    in_sut = in_opts["sut"]
    in_key = in_opts["sut_key"]
    in_version = in_opts["sut_version"]
    in_test_run = in_opts["test_run"]
    in_direction = in_opts["msg_direction"]

    param_check = validate_parameters(in_opts)
    if param_check != 0:
        return param_check

    file = open(in_file_name, 'rb')
    num_lines = count_lines(file)

    test_id = ("%s_%s" % (in_key, in_test_run))

    ret = call_service_for_check(in_opts, "%s/test/info/%s" % (base_url, test_id))
    if ret.status_code < 200 or ret.status_code >= 205:
        logger.error("There's no test with the ID %s. Please record that test info before loading data"
                     % test_id)

    configure_throughput_mapping(in_opts)

    req_url = "%s/%s/throughput" % (base_url, in_key)

    logger.info("There are %d lines to read" % (num_lines))
    csv_data = csv.reader(file, delimiter=';', quotechar='|')
    i = 0;
    for row in csv_data:
        i += 1

        if row[0] == "summary":
            continue

        ts = row[1]
        count = int(row[3])
        duration = int(row[5])
        rate = float(row[7])

        request_data = {
            "type": in_type,
            "sut": in_sut,
            "version": in_version,
            "ts": ts,
            "count": count,
            "duration": duration,
            "rate": rate,
            "test_id": test_id,
            "direction": in_direction
        }

        request_json = StringIO()
        json.dump(request_data, request_json)
        # logger.info("JSON: %s" % (request_json.getvalue(),))
        sys.stdout.write("Request %d of %d\r" % (i, num_lines))

        # TODO: check if the key exists before calling the service
        call_service(in_opts, req_url, request_json.getvalue())

    print ""

    return 0


def load_test_info(in_opts):
    """
    Loads test information into the database
    :param in_opts: input options from command line
    :return:
    """
    base_url = in_opts["url"]
    in_sut_key = in_opts["sut_key"]

    in_test_type = in_opts["test_type"]
    in_test_run = in_opts["test_run"]
    in_start_time = in_opts["test_start_time"]
    in_test_duration = in_opts["test_duration"]
    in_test_comment = in_opts["test_comment"]
    in_test_result_comment = in_opts["test_result_comment"]

    in_broker_os_name = in_opts["broker_os_name"]
    in_broker_os_type = in_opts["broker_os_type"]
    in_broker_os_version = in_opts["broker_os_version"]
    in_broker_hw_type = in_opts["broker_hw_type"]
    in_brk_sys_info = in_opts["broker_sys_info"]

    in_msg_protocol = in_opts["msg_protocol"]
    in_msg_size = in_opts["msg_size"]
    in_msg_direction = in_opts["msg_direction"]

    in_prod_count = in_opts["producer_count"]
    in_prod_sys_info = in_opts["producer_sys_info"]

    in_con_count = in_opts["consumer_count"]
    in_con_sysinfo = in_opts["consumer_sys_info"]


    if base_url is None:
        logger.error("Base URL is required")

        return 1

    if in_test_run is None:
        logger.error("Test run is required")

        return 1

    if in_start_time is None:
        logger.error("Test start time is required")

        return 1

    if in_sut_key is None:
        logger.error("SUT key is required")

        return 1

    test_id = ("%s_%s" % (in_sut_key, in_test_run))

    ret = call_service_for_check(in_opts, "%s/sut/messaging/_search?q=key:%s" % (base_url, in_sut_key))
    if ret.status_code < 200 or ret.status_code >= 205:
        logger.error("There's no SUT with the ID %s. Please load that key before recording a test info"
                     % test_id)

    req_url = "%s/test/info/%s" % (base_url, test_id)

    request_data = {
        "type": in_test_type,
        "test_id": test_id,
        "test_run": in_test_run,
        "key": in_sut_key,
        "start_time": in_start_time,
        "test_duration": in_test_duration,
        "protocol": in_msg_protocol,
        "msg_size": in_msg_size,
        "msg_direction": in_msg_direction,
        "os_name": in_broker_os_name,
        "os_type": in_broker_os_type,
        "os_version": in_broker_os_version,
        "hw_type": in_broker_hw_type,
        "broker_sysinfo": in_brk_sys_info,
        "producer_count": in_prod_count,
        "producer_sysinfo": in_prod_sys_info,
        "consumer_count": in_con_count,
        "consumer_sysinfo": in_con_sysinfo,
        "comment": in_test_comment,
        "result_comment": in_test_result_comment,
    }

    request_json = StringIO()
    json.dump(request_data, request_json)

    logger.info("JSON: %s" % (request_json.getvalue()))

    call_service(in_opts, req_url, request_json.getvalue())

    return 0

def main(in_opts):
    logger.info("test")
    # config = ConfigParser.RawConfigParser()
    # configfile = in_opts["config"]

    is_register = in_opts["register"]

    if is_register:
        return register(in_opts)
    else:
        is_testinfo = in_opts["testinfo"]

        if is_testinfo:
            return load_test_info(in_opts)

        else:
            in_direction = in_opts["direction"]
            in_type = in_opts["type"]

            if in_direction is None:
                logger.error("Direction is required")

                return 1

            if in_type is None:
                logger.error("Type is required")

                return 1

            if in_direction == "receiver" and in_type == "latency":
                return load_receiver_latencies(in_opts)

            if in_direction == "receiver" and in_type == "throughput":
                return load_receiver_throughput(in_opts)

            if in_direction == "sender" and in_type == "throughput":
                return load_receiver_throughput(in_opts)


    return

# main() call
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    # parameters definition and parsing
    # -------------------------------------------------------------------------
    usage_msg = "usage: %prog [options]";
    op = optparse.OptionParser(usage=usage_msg);

    # General options
    op.add_option("--url", dest="url", type="string",
                  action="store", help="Database server URL", metavar="URL");

    op.add_option("--filename", dest="filename", type="string",
                  action="store", default=None, metavar="FILENAME",
                  help="The input filename (def: %default)");

    op.add_option("--username", dest="username", type="string",
              action="store", default="admin", metavar="USERNAME",
              help="Database user name (def: %default)");

    op.add_option("--password", dest="password", type="string",
              action="store", default=None, metavar="PASSWORD",
              help="Database user password (def: %default)");

    op.add_option("--update", dest="update", action="store_true", default=False,
                  help="Update the record instead of creating a new one");

    # Actions
    op.add_option("--register", dest="register", action="store_true", default=False,
                  help="Register SUTs");

    op.add_option("--load", dest="load", action="store_true", default=False,
                  help="Load test data into the DB");

    op.add_option("--testinfo", dest="testinfo", action="store_true", default=False,
                  help="Load test information data into the DB");


    # SUT stuff
    op.add_option("--sut", dest="sut", type="string",
                  action="store", default=None, metavar="SUT_NAME",
                  help="Software under test (SUT) name (def: %default)");

    op.add_option("--sut-key", dest="sut_key", type="string",
                  action="store", default=None, metavar="KEY",
                  help="Unique key identifier (ie: amq6, artemis, etc) for the SUT (def: %default)");

    op.add_option("--sut-version", dest="sut_version", type="string",
                  action="store", default=None, metavar="VERSION",
                  help="Version of the SUT");

    # Test specific stuff
    op.add_option("--test-type", dest="test_type", type="string",
                  action="store", default=None, metavar="TYPE  ",
                  help="Test handler type (latency, throughput, etc)");

    op.add_option("--test-run", dest="test_run", type="string",
                  action="store", default=None, metavar="TEST_RUN",
                  help="The test execution number (def: %default)");

    op.add_option("--test-duration", dest="test_duration", type="string",
                  action="store", default="0", metavar="DURATION",
                  help="The test duration in number of messages or time in the format " \
                       "<value><t> (ie: 1 day = 1d, 2 hours = 2h, etc)");

    op.add_option("--test-start-time", dest="test_start_time", type="string",
                  action="store", default=None, metavar="YYY-MM-DD HH:mm:ss",
                  help="The start time for the test (def: %default)");

    op.add_option("--test-comment", dest="test_comment", type="string",
                  action="store", default=None, metavar="COMMENT",
                  help="Test comment (def: %default)");

    op.add_option("--test-result-comment", dest="test_result_comment", type="string",
              action="store", default=None, metavar="RESULT_COMMENT",
              help="Test result comment (def: %default)");


    # Message stuff
    op.add_option("--msg-protocol", dest="msg_protocol", type="string",
                  action="store", default="amqp", metavar="PROTOCOL",
                  help="The messaging protocol used on the test [stomp, amqp, mqtt, etc] "\
                          "(def: %default)");

    op.add_option("--msg-size", dest="msg_size", type="string",
                  action="store", default="0", metavar="SIZE",
                  help="The message size used in the tests (def: %default / 0 for random)");

    op.add_option("--msg-direction", dest="msg_direction", type="string",
                  action="store", default=None, metavar="TYPE  ",
                  help="Message direction where it was flowing from/to/through (ie: receiver, sender, router)");

    op.add_option("--msg-endpoint-type", dest="msg_endpoint_type", type="string",
                  action="store", default="queue", metavar="TYPE",
                  help="The endpoint type (queue or topic)");

    # Consumer stuff
    op.add_option("--consumers-count", dest="consumer_count", type="string",
                  action="store", default="1", metavar="NUMBER",
                  help="The number of concurrent consumers (def: %default)");

    op.add_option("--consumer-sys-info", dest="consumer_sys_info", type="string",
                  action="store", default=None, metavar="CON_SYS_INFO",
                  help="The consumer system information (def: %default)");

    # Producer stuff

    op.add_option("--producers-count", dest="producer_count", type="string",
                  action="store", default="1", metavar="NUMBER",
                  help="The number of concurrent producers (def: %default)");


    op.add_option("--producer-sys-info", dest="producer_sys_info", type="string",
                  action="store", default=None, metavar="PROD_SYS_INFO",
                  help="The producer system information (def: %default)");

    # Broker system stuff
    op.add_option("--broker-sys-ostype", dest="broker_os_type", type="string",
                  action="store", default=None, metavar="OS_TYPE",
                  help="The host OS type [linux, windows, etc] (def: %default)");

    op.add_option("--broker-sys-osname", dest="broker_os_name", type="string",
                  action="store", default=None, metavar="OS_NAME",
                  help="The host OS name [rhel, fedora, windows, etc] (def: %default)");

    op.add_option("--broker-sys-osversion", dest="broker_os_version", type="string",
                  action="store", default=None, metavar="OS_VERSION",
                  help="The host OS version [6.6, 6.7, 2012, etc] (def: %default)");

    op.add_option("--broker-sys-hwtype", dest="broker_hw_type", type="string",
                  action="store", default="bare", metavar="HW_TYPE",
                  help="The hardware type [kvm, bare, vmware] (def: %default)");

    op.add_option("--broker-sys-info", dest="broker_sys_info", type="string",
                  action="store", default=None, metavar="BROKER_SYS_INFO",
                  help="The broker system information (def: %default)");



    (opts, args) = op.parse_args();


    int_opts = {};
    int_opts = eval('%s' % opts);

    sys.exit(main(int_opts));

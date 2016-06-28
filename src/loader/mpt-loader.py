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

def call_service(in_opts, req_url, request_json):
    logger.debug("Connecting to %s" % (req_url))


    headers = {'Content-type': 'application/json'}

    username = in_opts["username"]
    password = in_opts["password"]

    is_update = in_opts["update"]

    if is_update:
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
    sut = in_opts["sut"]
    key = in_opts["key"]
    version = in_opts["version"]

    if base_url is None:
        logger.error("Base URL is required")

        return 1

    request_data = {
        "sut": sut,
        "key": key,
        "version": version,
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
    in_key = in_opts["key"]


    request_json = '{ "mappings": { "latency": { "properties": { "creation": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss.SSS"} } } } }'

    req_url = "%s/%s" % (base_url, in_key)
    call_service(in_opts, req_url, request_json)

def configure_throughput_mapping(in_opts):
    base_url = in_opts["url"]
    in_key = in_opts["key"]


    request_json = '{ "mappings": { "throughput": { "properties": { "ts": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss"} } } } }'

    req_url = "%s/%s" % (base_url, in_key)
    call_service(in_opts, req_url, request_json)



def count_lines(file):
    for i, l in enumerate(file):
      pass

    file.seek(0)
    return i + 1

def validate_parameters(in_opts):
    base_url = in_opts["url"]
    in_file_name = in_opts["filename"]
    in_type = in_opts["type"]
    in_sut = in_opts["sut"]
    in_key = in_opts["key"]
    in_version = in_opts["version"]
    in_testid = in_opts["testid"]
    in_direction = in_opts["direction"]

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

    if in_testid is None:
        logger.error("Test ID is required")

        return 1

    if in_direction is None:
        logger.error("Direction is required")

        return 1

    return 0

def load_receiver_latencies(in_opts):
    base_url = in_opts["url"]
    in_file_name = in_opts["filename"]
    in_type = in_opts["type"]
    in_sut = in_opts["sut"]
    in_key = in_opts["key"]
    in_version = in_opts["version"]
    in_testid = in_opts["testid"]
    in_direction = in_opts["direction"]

    param_check = validate_parameters(in_opts)
    if param_check != 0:
        return param_check

    ret = call_service_for_check(in_opts, "%s/test/info/%s" % (base_url, in_testid))
    if ret.status_code < 200 or ret.status_code >= 205:
        logger.error("There's no test with the ID %s. Please record that test info before loading data"
                     % in_testid)

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
            "test_id": in_testid,
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
    in_type = in_opts["type"]
    in_sut = in_opts["sut"]
    in_key = in_opts["key"]
    in_version = in_opts["version"]
    in_testid = in_opts["testid"]
    in_direction = in_opts["direction"]

    param_check = validate_parameters(in_opts)
    if param_check != 0:
        return param_check

    file = open(in_file_name, 'rb')
    num_lines = count_lines(file)

    ret = call_service_for_check(in_opts, "%s/test/info/%s" % (base_url, in_testid))
    if ret.status_code < 200 or ret.status_code >= 205:
        logger.error("There's no test with the ID %s. Please record that test info before loading data"
                     % in_testid)

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
            "test_id": in_testid,
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
    base_url = in_opts["url"]
    in_type = in_opts["type"]
    in_key = in_opts["key"]
    in_testid = in_opts["testid"]
    in_os_name = in_opts["os_name"]
    in_os_type = in_opts["os_type"]
    in_os_version = in_opts["os_version"]
    in_hw_type = in_opts["hw_type"]
    in_protocol = in_opts["protocol"]
    in_prod_sysinfo = in_opts["prodsysinfo"]
    in_con_sysinfo = in_opts["consysinfo"]
    in_brk_sysinfo = in_opts["brksysinfo"]
    in_start_time = in_opts["start_time"]

    in_comment = in_opts["comment"]
    in_res_comment = in_opts["rescomment"]

    if base_url is None:
        logger.error("Base URL is required")

        return 1

    if in_testid is None:
        logger.error("Test ID is required")

        return 1

    if in_start_time is None:
        logger.error("Test start time is required")

        return 1

    if in_key is None:
        logger.error("SUT key is required")

        return 1

    ret = call_service_for_check(in_opts, "%s/sut/messaging/_search?q=key:%s" % (base_url, in_key))
    if ret.status_code < 200 or ret.status_code >= 205:
        logger.error("There's no SUT with the ID %s. Please load that key before recording a test info"
                     % in_testid)

    req_url = "%s/test/info/%s" % (base_url, in_testid)

    request_data = {
        "type": in_type,
        "test_id": in_testid,
        "key": in_key,
        "start_time": in_start_time,
        "os_name": in_os_name,
        "os_type": in_os_type,
        "os_version": in_os_version,
        "hw_type": in_hw_type,
        "protocol": in_protocol,
        "producer_sysinfo": in_prod_sysinfo,
        "consumer_sysinfo": in_con_sysinfo,
        "broker_sysinfo": in_brk_sysinfo,
        "comment": in_comment,
        "result_comment": in_res_comment,
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

    # define parameters
    # -------------------------------------------------------------------------
    op.add_option("--url", dest="url", type="string",
                  action="store", help="Database server URL", metavar="URL");

    op.add_option("--update", dest="update", action="store_true", default=False,
                  help="Update the record instead of creating a new one");

    op.add_option("--register", dest="register", action="store_true", default=False,
                  help="Register SUTs");

    op.add_option("--load", dest="load", action="store_true", default=False,
                  help="Load test data into the DB");

    op.add_option("--testinfo", dest="testinfo", action="store_true", default=False,
                  help="Load test information data into the DB");

    op.add_option("--sut", dest="sut", type="string",
                  action="store", default=None, metavar="SUT_NAME",
                  help="Software under test (SUT) name (def: %default)");

    op.add_option("--key", dest="key", type="string",
                  action="store", default=None, metavar="KEY",
                  help="Unique key identifier (ie: amq6, artemis, etc) for the SUT (def: %default)");

    op.add_option("--version", dest="version", type="string",
                  action="store", default=None, metavar="VERSION",
                  help="Version of the SUT");

    op.add_option("--type", dest="type", type="string",
                  action="store", default=None, metavar="TYPE  ",
                  help="Test handler type (latency, throughput, etc)");

    op.add_option("--direction", dest="direction", type="string",
                  action="store", default=None, metavar="TYPE  ",
                  help="Message direction where it was flowing from/to/through (ie: receiver, sender, router)");

    op.add_option("--testid", dest="testid", type="string",
                  action="store", default=None, metavar="TEST_ID",
                  help="The test ID (def: %default)");

    op.add_option("--filename", dest="filename", type="string",
                  action="store", default=None, metavar="FILENAME",
                  help="The input filename (def: %default)");

    op.add_option("--username", dest="username", type="string",
                  action="store", default="admin", metavar="USERNAME",
                  help="Database user name (def: %default)");

    op.add_option("--password", dest="password", type="string",
              action="store", default=None, metavar="PASSWORD",
              help="Database user password (def: %default)");

    op.add_option("--ostype", dest="os_type", type="string",
                  action="store", default=None, metavar="OS_TYPE",
                  help="The host OS type [linux, windows, etc] (def: %default)");

    op.add_option("--osname", dest="os_name", type="string",
              action="store", default=None, metavar="OS_NAME",
              help="The host OS name [rhel, fedora, windows, etc] (def: %default)");

    op.add_option("--osversion", dest="os_version", type="string",
              action="store", default=None, metavar="OS_VERSION",
              help="The host OS version [6.6, 6.7, 2012, etc] (def: %default)");

    op.add_option("--hwtype", dest="hw_type", type="string",
                  action="store", default="bare", metavar="HW_TYPE",
                  help="The hardware type [kvm, bare, vmware] (def: %default)");

    op.add_option("--protocol", dest="protocol", type="string",
                  action="store", default="amqp", metavar="PROTOCOL",
                  help="The messaging protocol used on the test [stomp, amqp, mqtt, etc] (def: %default)");

    op.add_option("--producer-sys-info", dest="prodsysinfo", type="string",
                  action="store", default=None, metavar="PROD_SYS_INFO",
                  help="The producer system information (def: %default)");

    op.add_option("--consumer-sys-info", dest="consysinfo", type="string",
                  action="store", default=None, metavar="CON_SYS_INFO",
                  help="The consumer system information (def: %default)");

    op.add_option("--start-time", dest="start_time", type="string",
                  action="store", default=None, metavar="YYY-MM-DD HH:mm:ss",
                  help="The start time for the test (def: %default)");

    op.add_option("--broker-sys-info", dest="brksysinfo", type="string",
                  action="store", default=None, metavar="BROKER_SYS_INFO",
                  help="The broker system information (def: %default)");

    op.add_option("--comment", dest="comment", type="string",
                  action="store", default=None, metavar="COMMENT",
                  help="Test comment (def: %default)");

    op.add_option("--result-comment", dest="rescomment", type="string",
                  action="store", default=None, metavar="RESULT_COMMENT",
                  help="Test result comment (def: %default)");


    (opts, args) = op.parse_args();


    int_opts = {};
    int_opts = eval('%s' % opts);

    sys.exit(main(int_opts));

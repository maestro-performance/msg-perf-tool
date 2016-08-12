import logging
import requests

from requests.auth import HTTPBasicAuth
from mpt.settings import *

logger = logging.getLogger(__name__)

def print_errors(http_response):
    logger.error("Error: %s" % http_response.status_code)
    logger.error("Text: %s" % http_response.content)


def call_service(req_url, request_json, force_update=False, session=None, is_update=False):
    logger.debug("Connecting to %s" % (req_url))

    headers = {'Content-type': 'application/json'}

    username = read_param("database", "username")
    if username is not None:
        password = read_param("database", "password")

    if session is None:
        session = requests.session()

    logger.debug("Req URL: %s" % req_url)
    logger.debug("Data: %s" % request_json)

    if is_update and not force_update:
        answer = session.put(req_url, headers=headers, data=request_json, verify=False,
                               auth=HTTPBasicAuth(username, password))
    else:
        answer = session.post(req_url, headers=headers, data = request_json, verify=False,
                               auth=HTTPBasicAuth(username, password))

    if answer.status_code < 200 or answer.status_code >= 205:
        print_errors(answer)
        return 3

    return 0


def call_service_for_check(req_url, session=None):
    logger.debug("Connecting to %s" % (req_url))

    headers = {'Content-type': 'application/json'}

    username = read_param("database", "username")
    if username is not None:
        password = read_param("database", "password")

    if session is None:
        session = requests.session()

    answer = session.get(req_url, headers=headers, verify=False, auth=HTTPBasicAuth(username, password))
    return answer

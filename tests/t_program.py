#!/usr/bin/python3
# Copyright (C) 2017 - GSS-Proxy contributors; see COPYING for the license.

from testlib import *

from t_acquire import run as run_acquire_test

import os
import testlib

GSSPROXY_PROGRAM = '''
[gssproxy]
  debug_level = 3

[service/t_acquire]
  mechs = krb5
  cred_store = keytab:${GSSPROXY_KEYTAB}
  cred_store = client_keytab:${GSSPROXY_CLIENT_KEYTAB}
  trusted = yes
  euid = ${UIDNUMBER}
  allow_client_ccache_sync = yes
  program = ${PROGDIR}/t_acquire
'''

def run(testdir, env, conf):
    conf['prefix'] = str(testlib.cmd_index)
    prefix = conf["prefix"]
    retval = 0

    print("Testing positive program name matching...", file=sys.stderr)
    sys.stderr.write("  ")
    conf["prefix"] = prefix + "_1"
    update_gssproxy_conf(testdir, conf["keysenv"], GSSPROXY_PROGRAM)
    gssproxy_reload(testdir, conf['gpid'])
    retval |= run_acquire_test(testdir, env, conf)

    print("Testing negative program name matching...", file=sys.stderr)
    sys.stderr.write("  ")
    conf["prefix"] = prefix + "_2"
    bad_progdir = GSSPROXY_PROGRAM.replace("${PROGDIR}", "//bad/path")
    update_gssproxy_conf(testdir, conf["keysenv"], bad_progdir)
    gssproxy_reload(testdir, conf['gpid'])
    retval |= run_acquire_test(testdir, env, conf, expected_failure=True)

    # be a good citizen and clean up after ourselves
    update_gssproxy_conf(testdir, conf["keysenv"], GSSPROXY_CONF_TEMPLATE)
    gssproxy_reload(testdir, conf['gpid'])

    print_return(retval, -1, "(%d) Program" % testlib.cmd_index, False)
    testlib.cmd_index += 1
    return retval

if __name__ == "__main__":
    from runtests import runtests_main
    runtests_main(["t_program.py"])

import pytest
import os
import re
import copy

from daqconf.utils import find_free_port
import integrationtest.log_file_checks as log_file_checks
import integrationtest.data_classes as data_classes
import integrationtest.utility_functions as utility_functions
from integrationtest.verbosity_helper import IntegtestVerbosityLevels

import functools
print = functools.partial(print, flush=True)  # always flush print() output

pytest_plugins = "integrationtest.integrationtest_drunc"

# Values that help determine the running conditions
run_duration = 20  # seconds

# Default values for validation parameters
check_for_logfile_errors = True
hostname = os.uname().nodename

ignored_logfile_problems = {
    "local-connection-server": [
        "errorlog: -",
    ],    
}

common_config_obj = data_classes.integtest_params_for_predefined_dunedaq_config()
# 22-Jan-2026, KAB: added the use of the DAQSYSTEMTEST_SHARE env var as part of
# specifying the location of the example-configs.data.xml file.  This is more
# reliable than using a relative path to a parallel directory with the daqsystemtest
# code in it.  Of course, this new model has the feature that users will need to
# rebuild their software area if they make changes to the configuration entities
# in a local copy of the daqsystemtest repo before this code will see those changes.
common_config_obj.predefined_config_db = (
    os.environ.get("DAQSYSTEMTEST_SHARE") + "/config/daqsystemtest/example-configs.data.xml"
)

onebyone_local_crt_bern_conf = copy.deepcopy(common_config_obj)
onebyone_local_crt_bern_conf.config_session_name = "local-socket-1x1-config"

new_local_port = find_free_port()
new_remote_port = find_free_port()
onebyone_local_crt_bern_conf.config_substitutions.append(
    data_classes.attribute_substitution(
        obj_class="SocketDataSender",
        obj_id="socket_sender_crt",
        updates={"local_port": new_local_port, "remote_port": new_remote_port},
    )
)
new_local_port = find_free_port()
new_remote_port = find_free_port()
onebyone_local_crt_bern_conf.config_substitutions.append(
    data_classes.attribute_substitution(
        obj_class="SocketDataSender",
        obj_id="socket_sender_crt_2",
        updates={"local_port": new_local_port, "remote_port": new_remote_port},
    )
)

onebyone_local_crt_grenoble_conf = copy.deepcopy(common_config_obj)
onebyone_local_crt_grenoble_conf.config_session_name = "local-socket-1x1-config"

new_local_port = find_free_port()
new_remote_port = find_free_port()
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.attribute_substitution(
        obj_class="SocketDataSender",
        obj_id="socket_sender_crt",
        updates={"local_port": new_local_port, "remote_port": new_remote_port},
    )
)
new_local_port = find_free_port()
new_remote_port = find_free_port()
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.attribute_substitution(
        obj_class="SocketDataSender",
        obj_id="socket_sender_crt_2",
        updates={"local_port": new_local_port, "remote_port": new_remote_port},
    )
)
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.relationship_substitution(
        obj_class="CRTFrameBuilderApplication",
        obj_id="crt-data-source-01",
        rel_name="detector_frame_builder",
        replacement_object_class="CRTGrenobleFrameBuilderConf",
        replacement_object_id="def-crt-grenoble-frame-builder-conf"
    )
)
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.list_element_substitution(
        obj_class="CRTFrameBuilderApplication",
        obj_id="crt-data-source-01",
        rel_name="queue_rules",
        list_index=0,
        replacement_object_class="QueueConnectionRule",
        replacement_object_id="crt-grenoble-sender-input-queue-rule"
    )
)
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.list_element_substitution(
        obj_class="ActionPlan",
        obj_id="crt-readout-start",
        rel_name="steps",
        list_index=1,
        replacement_object_class="DaqModulesGroupByType",
        replacement_object_id="crt-grenoble-frame-builder-data-source-step"
    )
)
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.list_element_substitution(
        obj_class="ActionPlan",
        obj_id="crt-readout-stop",
        rel_name="steps",
        list_index=0,
        replacement_object_class="DaqModulesGroupByType",
        replacement_object_id="crt-grenoble-frame-builder-data-source-step"
    )
)
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.relationship_substitution(
        obj_class="DetectorStream",
        obj_id="stream_1007",
        rel_name="geo_id",
        replacement_object_class="GeoId",
        replacement_object_id="g_13_1_1_0"
    )
)
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.relationship_substitution(
        obj_class="DetectorStream",
        obj_id="stream_1008",
        rel_name="geo_id",
        replacement_object_class="GeoId",
        replacement_object_id="g_13_1_1_1"
    )
)
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.relationship_substitution(
        obj_class="DetectorStream",
        obj_id="stream_1009",
        rel_name="geo_id",
        replacement_object_class="GeoId",
        replacement_object_id="g_13_2_1_0"
    )
)
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.relationship_substitution(
        obj_class="DetectorStream",
        obj_id="stream_1010",
        rel_name="geo_id",
        replacement_object_class="GeoId",
        replacement_object_id="g_13_2_1_1"
    )
)

onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.relationship_substitution(
        obj_class="ReadoutApplication",
        obj_id="socket-ru-01",
        rel_name="link_handler",
        replacement_object_class="DataHandlerConf",
        replacement_object_id="def-crt-grenoble-link-handler"
    )
)
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.relationship_substitution(
        obj_class="ReadoutApplication",
        obj_id="socket-ru-01",
        rel_name="callback_desc",
        replacement_object_class="DataMoveCallbackDescriptor",
        replacement_object_id="crt-grenoble-raw-input"
    )
)

confgen_arguments = {
    "Local CRT Bern 1x1 Conf": onebyone_local_crt_bern_conf,
    "Local CRT Grenoble 1x1 Conf": onebyone_local_crt_grenoble_conf,
}

dunerc_command_list = "boot conf".split()
dunerc_command_list += (
    "start --run-number 101 wait 5 enable-triggers wait ".split()
    + [str(run_duration)]
    + "disable-triggers wait 1 drain-dataflow wait 2 stop-trigger-sources wait 1 stop wait 2".split()
)
dunerc_command_list += "scrap terminate".split()

def test_dunerc_success(run_dunerc, caplog):
    # checks for run control success, problems during pytest setup, etc.
    utility_functions.basic_checks(run_dunerc, caplog, print_test_name=True)

def test_log_files(run_dunerc):
    if check_for_logfile_errors:
        # Check that there are no warnings or errors in the log files
        assert log_file_checks.logs_are_error_free(
            run_dunerc.log_files, True, True, ignored_logfile_problems,
            verbosity_helper=run_dunerc.verbosity_helper
        )

import pytest
import os
import re
import copy

from daqconf.utils import find_free_port
import integrationtest.log_file_checks as log_file_checks
import integrationtest.data_classes as data_classes

pytest_plugins = "integrationtest.integrationtest_drunc"

# Values that help determine the running conditions
number_of_data_producers = 1
number_of_readout_apps = 1
run_duration = 20  # seconds

# Default values for validation parameters
check_for_logfile_errors = True
hostname = os.uname().nodename

ignored_logfile_problems = {
    "local-connection-server": [
        "errorlog: -",
    ],    
}

common_config_obj = data_classes.drunc_config()
common_config_obj.dro_map_config.n_streams = number_of_data_producers
common_config_obj.dro_map_config.n_apps = number_of_readout_apps
# 22-Jan-2026, KAB: added the use of the DAQSYSTEMTEST_SHARE env var as part of
# specifying the location of the example-configs.data.xml file.  This is more
# reliable than using a relative path to a parallel directory with the daqsystemtest
# code in it.  Of course, this new model has the feature that users will need to
# rebuild their software area if they make changes to the configuration entities
# in a local copy of the daqsystemtest repo before this code will see those changes.
common_config_obj.config_db = (
    os.environ.get("DAQSYSTEMTEST_SHARE") + "/config/daqsystemtest/example-configs.data.xml"
)

onebyone_local_crt_bern_conf = copy.deepcopy(common_config_obj)
onebyone_local_crt_bern_conf.session = "local-socket-1x1-config"

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
onebyone_local_crt_bern_conf.config_substitutions.append(
    data_classes.relationship_substitution(
        obj_class="CRTReaderApplication",
        obj_id="crt-data-source-01",
        rel_name="data_reader",
        replacement_object_class="CRTBernReaderConf",
        replacement_object_id="def-crt-bern-receiver-conf"
    )
)
onebyone_local_crt_bern_conf.config_substitutions.append(
    data_classes.list_element_substitution(
        obj_class="ActionPlan",
        obj_id="crt-readout-start",
        rel_name="steps",
        list_index=1,
        replacement_object_class="DaqModulesGroupByType",
        replacement_object_id="crt-bern-reader-data-source-step"
    )
)
onebyone_local_crt_bern_conf.config_substitutions.append(
    data_classes.list_element_substitution(
        obj_class="ActionPlan",
        obj_id="crt-readout-stop",
        rel_name="steps",
        list_index=0,
        replacement_object_class="DaqModulesGroupByType",
        replacement_object_id="crt-bern-reader-data-source-step"
    )
)

onebyone_local_crt_grenoble_conf = copy.deepcopy(common_config_obj)
onebyone_local_crt_grenoble_conf.session = "local-socket-1x1-config"

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
        obj_class="CRTReaderApplication",
        obj_id="crt-data-source-01",
        rel_name="data_reader",
        replacement_object_class="CRTGrenobleReaderConf",
        replacement_object_id="def-crt-grenoble-receiver-conf"
    )
)
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.relationship_substitution(
        obj_class="CRTReaderApplication",
        obj_id="crt-data-source-01",
        rel_name="callback_desc",
        replacement_object_class="DataMoveCallbackDescriptor",
        replacement_object_id="crt-grenoble-raw-input"
    )
)
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.list_element_substitution(
        obj_class="ActionPlan",
        obj_id="crt-readout-start",
        rel_name="steps",
        list_index=1,
        replacement_object_class="DaqModulesGroupByType",
        replacement_object_id="crt-grenoble-reader-data-source-step"
    )
)
onebyone_local_crt_grenoble_conf.config_substitutions.append(
    data_classes.list_element_substitution(
        obj_class="ActionPlan",
        obj_id="crt-readout-stop",
        rel_name="steps",
        list_index=0,
        replacement_object_class="DaqModulesGroupByType",
        replacement_object_id="crt-grenoble-reader-data-source-step"
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

nanorc_command_list = "boot conf".split()
nanorc_command_list += (
    "start --run-number 101 wait 5 enable-triggers wait ".split()
    + [str(run_duration)]
    + "disable-triggers wait 1 drain-dataflow wait 2 stop-trigger-sources wait 1 stop wait 2".split()
)
nanorc_command_list += "scrap terminate".split()

def test_nanorc_success(run_nanorc):
    # print the name of the current test
    current_test = os.environ.get("PYTEST_CURRENT_TEST")
    match_obj = re.search(r".*\[(.+)-run_.*rc.*\d].*", current_test)
    if match_obj:
        current_test = match_obj.group(1)
    banner_line = re.sub(".", "=", current_test)
    print(banner_line)
    print(current_test)
    print(banner_line)    

    # Check that nanorc completed correctly
    assert run_nanorc.completed_process.returncode == 0 

def test_log_files(run_nanorc):
    if check_for_logfile_errors:
        # Check that there are no warnings or errors in the log files
        assert log_file_checks.logs_are_error_free(
            run_nanorc.log_files, True, True, ignored_logfile_problems
        )

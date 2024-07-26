# This module facilitates the generation of crtmodules DAQModules within crtmodules apps


# Set moo schema search path                                                                              
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

# Load configuration types                                                                                
import moo.otypes
moo.otypes.load_types("crtmodules/crtcontrollermodule.jsonnet")

import dunedaq.crtmodules.crtcontrollermodule as crtcontrollermodule

from daqconf.core.app import App, ModuleGraph
from daqconf.core.daqmodule import DAQModule
#from daqconf.core.conf_utils import Endpoint, Direction

def get_crtmodules_app(nickname, num_crtcontrollermodules, board_confs, host="localhost"):
    """
    Here the configuration for an entire daq_application instance using DAQModules from crtmodules is generated.
    """

    modules = []

    for i in range(num_crtcontrollermodules):
        modules += [DAQModule(name = f"nickname{i}", 
                              plugin = "CRTControllerModule", 
                              conf = crtcontrollermodule.Conf(BoardConfs = board_confs
                                )
                    )]

    mgraph = ModuleGraph(modules)
    crtmodules_app = App(modulegraph = mgraph, host = host, name = nickname)

    return crtmodules_app

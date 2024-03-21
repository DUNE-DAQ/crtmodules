local moo = import "moo.jsonnet";
local ns = "dunedaq.crtmodules.crtcontrollermodule";
local s = moo.oschema.schema(ns);

local types = {

    int4 :    s.number(  "int4",    "i4",          doc="A signed integer of 4 bytes"),
    uint4 :   s.number(  "uint4",   "u4",          doc="An unsigned integer of 4 bytes"),
    int8 :    s.number(  "int8",    "i8",          doc="A signed integer of 8 bytes"),
    uint8 :   s.number(  "uint8",   "u8",          doc="An unsigned integer of 8 bytes"),
    float4 :  s.number(  "float4",  "f4",          doc="A float of 4 bytes"),
    double8 : s.number(  "double8", "f8",          doc="A double of 8 bytes"),
    boolean:  s.boolean( "Boolean",                doc="A boolean"),
    string:   s.string(  "String",   		   doc="A string"),   

    // TO crtmodules DEVELOPERS: PLEASE DELETE THIS FOLLOWING COMMENT AFTER READING IT
    // The following code is an example of a configuration record
    // written in jsonnet. In the real world it would be written so as
    // to allow the relevant members of CRTControllerModule to be configured by
    // Run Control

    gainlist: s.sequence("gains",self.uint4,doc="List of 64 gain values"),

    board_conf: s.record("BoardConf", [
                           s.field("usb_serial", self.uint4, doc="USB number from the CRT USB board"),
			   s.field("pmt_board", self.uint4, doc="PMT board number"),
			   s.field("hv_setting", self.uint8, 765, doc="HV setting for the PMT in V, maybe not used, def. 765"),
			   s.field("dac_threshold", self.uint8, doc="DAC threshold in xxx units"),
			   s.field("use_maroc2gain", self.boolean, doc="Flag to set gain on all channel (if false, use defaults)"),
			   s.field("gate", self.string, "off", doc="Type of gate to use ('off' means do not use one)"),
			   s.field("pipedelay", self.uint8, 5, doc="pipe delay in clock cycles (default: 5)"),
			   s.field("trigger_mode", self.uint4, doc="What trigger mode to use (bitmask)"),
			   s.field("force_trigger", self.uint4, 0, doc="two-bit flag on how often to force a trigger (default 0, do not force trigger)"),
			   s.field("gain", self.gainlist,
			     [ 16, 16, 16, 16, 16, 16, 16, 16,
			       16, 16, 16, 16, 16, 16, 16, 16,
			       16, 16, 16, 16, 16, 16, 16, 16,
			       16, 16, 16, 16, 16, 16, 16, 16,
			       16, 16, 16, 16, 16, 16, 16, 16,
			       16, 16, 16, 16, 16, 16, 16, 16,
			       16, 16, 16, 16, 16, 16, 16, 16,
			       16, 16, 16, 16, 16, 16, 16, 16],
			     doc="list of 64 gain values, defaults to 16 for all")
                           ],
                   doc="Configuration for a single board"),
		   
    board_confs: s.sequence("board_confs",self.board_conf,doc="list of configs for each board"),

    conf: s.record("Conf", [
        s.field( "BoardConfs", self.board_confs, doc="A list of settings to  configure the CRTControllerModule DAQModule instance")
    ])
};

moo.oschema.sort_select(types, ns)

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.crtmodules.crtreaderinfo");

local info = {

    int4 :    s.number(  "int4",    "i4",          doc="A signed integer of 4 bytes"),
    uint4 :   s.number(  "uint4",   "u4",          doc="An unsigned integer of 4 bytes"),
    int8 :    s.number(  "int8",    "i8",          doc="A signed integer of 8 bytes"),
    uint8 :   s.number(  "uint8",   "u8",          doc="An unsigned integer of 8 bytes"),
    float4 :  s.number(  "float4",  "f4",          doc="A float of 4 bytes"),
    double8 : s.number(  "double8", "f8",          doc="A double of 8 bytes"),
    boolean:  s.boolean( "Boolean",                doc="A boolean"),
    string:   s.string(  "String",                 doc="A string"),   

    info: s.record("Info", [
       s.field("frames_built",      self.int8, doc="Number of CRT frames built"),
       s.field("bytes_from_file",   self.int8, doc="Total bytes returned by FillBuffer from backend files"),
       s.field("frames_dropped",    self.int4, doc="Number of CRT frames dropped"),
       s.field("syncs_missed",      self.int4, doc="Number of 7s sync signals not received by CRT boards")
    ], doc="Information about the CRT reader"),

};

moo.oschema.sort_select(info)

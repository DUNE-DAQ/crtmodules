local moo = import "moo.jsonnet";
local ns = "dunedaq.crtmodules.crtreader";
local s = moo.oschema.schema(ns);

local types = {

    usb: s.number("USB", "u8", doc="usb stream number"),
    data_directory: s.string("DataDirectory", doc="directory"),

    conf: s.record("Conf", [
                           s.field("data_directory",
                                   self.data_directory,
                                   doc="directory where CRT binary data is written"
                                  ),
                           s.field("usb",
                                   self.usb,
                                   doc="USB stream"
                                  ),
                           ],
                   doc="Configuration for CRT card reader module (CRTReader)"),

};

moo.oschema.sort_select(types, ns)

// This is the application info schema used by the data link handler module.
// It describes the information object structure passed by the application 
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.lbrulibs.pacmancardreaderinfo");

local info = {
    uint8  : s.number("uint8", "u8",
                     doc="An unsigned of 8 bytes"),
    float8 : s.number("float8", "f8",
                      doc="A float of 8 bytes"),
    choice : s.boolean("Choice"),
    string : s.string("String", moo.re.ident,
                          doc="A string field"),

   info: s.record("ZMQLinkInfo", [
        s.field("num_packets_received",                  self.uint8,     0, doc="Number of packets received"),
        s.field("info_type",                  self.string,     "", doc="Information Type"),

   ], doc="ZMQ Link Information"),
};

moo.oschema.sort_select(info) 
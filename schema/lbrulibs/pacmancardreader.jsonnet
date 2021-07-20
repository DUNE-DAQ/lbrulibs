// The schema used by classes in the appfwk code tests.
//
// It is an example of the lowest layer schema below that of the "cmd"
// and "app" and which defines the final command object structure as
// consumed by instances of specific DAQModule implementations (ie,
// the test/Pacman* modules).

local moo = import "moo.jsonnet";

// A schema builder in the given path (namespace)
local ns = "dunedaq.lbrulibs.pacmancardreader";
local s = moo.oschema.schema(ns);

// Object structure used by the test/fake producer module
local pacmancardreader = {
    count  : s.number("Count", "u4",
                      doc="Count of things"),

    id : s.number("Identifier", "i4",
                  doc="An ID of a thingy"),

    region_id : s.number("region_id", "u2"),
    element_id : s.number("element_id", "u4"),
    system_type : s.string("system_type"),

    geoid : s.record("GeoID", [s.field("region", self.region_id, doc="" ),
        s.field("element", self.element_id, doc="" ),
        s.field("system", self.system_type, doc="" )],
        doc="GeoID"),

    link_conf : s.record("LinkConfiguration", [
        s.field("geoid", self.geoid, doc="GeoID of the link")
        ], doc="Configuration for one link"),

    link_conf_list : s.sequence("link_conf_list", self.link_conf, doc="Link configuration list"),


    conf: s.record("Conf", [

        s.field("link_confs", self.link_conf_list,
                doc="Link configurations"),

        s.field("card_id", self.id, 0,
                doc="FE card identifier"),

        s.field("zmq_receiver_timeout", self.id, 0,
                doc="ZMQ Receive Timeout value"),

    ], doc="Upstream Pacman CardReader DAQ Module Configuration"),

};

moo.oschema.sort_select(pacmancardreader, ns)

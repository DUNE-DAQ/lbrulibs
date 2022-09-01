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
                      doc="Item count"),

    id : s.number("Identifier", "i4",
                  doc="Generic ID variable"),
    
    sourceid: s.number("sourceid", "u4", doc="Source ID for Incoming Data"),
    
    link_conf : s.record("LinkConfiguration", [
        s.field("Source_ID", self.sourceid, 0, doc="Source ID for Link")
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

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

    conf: s.record("Conf", [
        s.field("card_id", self.id, 0,
                doc="Physical card identifier (in the same host)"),

        s.field("logical_unit", self.count, 0,
                doc="Superlogic region of selected card"),

    ], doc="Upstream Pacman CardReader DAQ Module Configuration"),

};

moo.oschema.sort_select(pacmancardreader, ns)

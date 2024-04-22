local moo = import "moo.jsonnet";

local ns = "dunedaq.lbrulibs.patcardreader"
local s = moo.oschema.schema(ns);

local patcardreader = {
      count : s.number("Count", "u4", doc="Item count"),
      id : s.number("sourceid", "u4", doc="Source ID for Incoming Data"),
      link_conf : s.record("LinkConfiguration", [s.field("Source_ID", self.sourceid, 0, doc="Source ID for Link")], doc="Configuration for one link"),
      link_conf_list : s.sequence("link_conf_list", self.link_conf, doc="Link configuration list"),

      conf: s.record("Conf", [
      	    s.field("link_confs", self.link_conf_list, doc="Link configurations"),
	    s.field("card_id" , self.id, 0, doc="FE card identifier"),
	    s.field("board_name", self.board_name, "board.xml", doc="Board definition for XML"),
	    s.field("dev_name", self.dev_name, "aggr", doc="Device node"),
	    s.field("mhal_receiver_timeout", self.id, 0, doc="mHAL timeout value", ], doc="Upstream PAT card DAQ module configuration"),
};
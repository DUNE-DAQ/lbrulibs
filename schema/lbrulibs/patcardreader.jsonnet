local moo = import "moo.jsonnet";

local ns = "dunedaq.lbrulibs.patcardreader";
local s  = moo.oschema.schema(ns);

local patcardreader = {
      count          : s.number("Count"            , "u4"                                                              , doc="Item count"                 ),
      id             : s.number("Identifier"       , "i4"                                                              , doc="Generic ID variable"        ),
      sourceid       : s.number("sourceid"         , "u4"                                                              , doc="Source ID for Incoming Data"),
      link_conf      : s.record("LinkConfiguration", [s.field("Source_ID", self.sourceid, 0, doc="Source ID for Link")], doc="Configuration for one link" ),
      link_conf_list : s.sequence("link_conf_list" , self.link_conf                                                    , doc="Link configuration list"    ),
      name           : s.string("name"             , moo.re.ident                                                      , doc="Board name string"          ),

      conf: s.record("Conf", [
      	    s.field("link_confs"           , self.link_conf_list, doc="Link configurations"),
	    s.field("card_id"              , self.id            , 0      , doc="FE card identifier"),
	    s.field("board_name"           , self.name          , "board", doc="Board definition for XML"),
	    s.field("dev_name"             , self.name          , "aggr" , doc="Device node"),
	    s.field("mhal_receiver_timeout", self.id            , 0      , doc="mHAL timeout value"),
	    ], doc="Upstream PAT card DAQ module configuration"),
};

moo.oschema.sort_select(patcardreader, ns)
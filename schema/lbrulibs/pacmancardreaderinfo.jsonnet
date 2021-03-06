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
             s.field("num_packets_received",                  self.uint8,      0, doc="Number of packets received"),
	     s.field("last_packet_size",                      self.float8,     0, doc="Message size of last packet"),
	     s.field("bandwidth",                  	      self.float8,     0, doc="Bandwidth during last Monitoring Period"),
	     s.field("last_message_timestamp",                self.float8,     0, doc="Timestamp of last received message from header"),
	     s.field("subscriber_connected",                  self.choice,     0, doc="ZMQ Subscriber connection status"),
	     s.field("run_marker",                  	      self.float8,     0, doc="Active run status"),
	     s.field("sink_name",                  	      self.string,     0, doc="Sink queue name, e.g. PACMAN"),
	     s.field("subscriber_num_zero_packets",           self.float8,     0, doc="Times zero data received"),
	     s.field("sink_is_set",                  	      self.choice,     0, doc="Sink queue creation status"),
	     s.field("card_id",                  	      self.float8,     0, doc="Card ID"),
	     s.field("link_tag",                  	      self.float8,     0, doc="Link Tag"),
	     s.field("source_link_string",                    self.string,     0, doc="Source link ID string"),
   ], doc="ZMQ Link Information"),
};

moo.oschema.sort_select(info) 
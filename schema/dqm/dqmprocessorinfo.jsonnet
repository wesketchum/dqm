// This is the application info schema used by the ta link handler module.
// It describes the information object structure passed by the application 
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.dqm.dqmprocessorinfo");

local info = {
    uint8  : s.number("uint8", "u8",
                     doc="An unsigned of 8 bytes"),

   info: s.record("Info", [
       s.field("requests",              self.uint8, 0, doc="Number of requests"), 
       s.field("total_requests",        self.uint8, 0, doc="Total number of requests"), 
       s.field("data_deliveries",       self.uint8, 0, doc="Number of requests"), 
       s.field("total_data_deliveries", self.uint8, 0, doc="Number of requests"), 

       s.field("raw_times_run",       self.uint8, 0, doc="Time taken to run the raw data algorithm"), 
       s.field("raw_time_taken",      self.uint8, 0, doc="Number of times the raw data algorithm has run"), 

       s.field("std_times_run",       self.uint8, 0, doc="Time taken to run the STD"), 
       s.field("std_time_taken",      self.uint8, 0, doc="Number of times STD has run"), 

       s.field("rms_times_run",       self.uint8, 0, doc="Time taken to run the RMS"), 
       s.field("rms_time_taken",      self.uint8, 0, doc="Number of times RMS has run"), 

       s.field("fourier_channel_times_run",       self.uint8, 0, doc="Time taken to run the fourier transform for each channel"), 
       s.field("fourier_channel_time_taken",      self.uint8, 0, doc="Number of times the fourier transform for each channel has run"), 

       s.field("fourier_plane_times_run",       self.uint8, 0, doc="Time taken to run the fourier transform for each plane"), 
       s.field("fourier_plane_time_taken",      self.uint8, 0, doc="Number of times the fourier transform for each plane has run"), 

       s.field("channel_map_total_channels",    self.uint8, 0, doc="Time taken to run the fourier transform for each plane"), 
       s.field("channel_map_total_planes",      self.uint8, 0, doc="Number of times the fourier transform for each plane has run"), 

   ], doc="DQM information")
};

moo.oschema.sort_select(info) 

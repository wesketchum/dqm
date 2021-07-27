// The schema used by classes in the appfwk code tests.
//
// It is an example of the lowest layer schema below that of the "cmd"
// and "app" and which defines the final command object structure as
// consumed by instances of specific DAQModule implementations (ie,
// the test/Fake* modules).

local moo = import "moo.jsonnet";

// A schema builder in the given path (namespace)
local ns = "dunedaq.dqm.dqmprocessor";
local s = moo.oschema.schema(ns);

// Object structure used by the test/fake producer module
local dqmprocessor = {

    string : s.string("RunningMode", moo.re.ident,
                            doc="A string field"),

    time : s.number("Time", "f4", doc="A time"),

    count : s.number("Count", "i4",
                     doc="A count of not too many things"),

    standard_dqm: s.record("StandardDQM", [
        s.field("histogram_how_often", self.time, 1,
                doc="Histograms are run every x seconds"),
        s.field("histogram_unavailable_time", self.time, 1,
                doc="When it's time to run the histograms but it's already running wait this time"),
        s.field("histogram_frames", self.count, 1,
                doc="How many frames we send in each instance of the histogram")
    ], doc="Standard DQM analysis"),

    conf: s.record("Conf", [
        s.field("mode", self.string,
                doc="'debug' if in debug mode"),
        s.field("sdqm", self.standard_dqm,
                doc="Test"),
        s.field("kafka_address", self.string,
                doc="Address used for sending to the kafka broker")
    ], doc="Generic DQM configuration"),
};

moo.oschema.sort_select(dqmprocessor, ns)
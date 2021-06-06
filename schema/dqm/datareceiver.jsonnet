// The schema used by classes in the appfwk code tests.
//
// It is an example of the lowest layer schema below that of the "cmd"
// and "app" and which defines the final command object structure as
// consumed by instances of specific DAQModule implementations (ie,
// the test/Fake* modules).

local moo = import "moo.jsonnet";

// A schema builder in the given path (namespace)
local ns = "dunedaq.dqm.datareceiver";
local s = moo.oschema.schema(ns);

// Object structure used by the test/fake producer module
local datareceiver = {
    conf: s.record("Conf", [
        s.field("mode", self.string,
                 doc="'debug' if in debug mode")
    ], doc="SNBWriter configuration"),

};

moo.oschema.sort_select(datareceiver, ns)
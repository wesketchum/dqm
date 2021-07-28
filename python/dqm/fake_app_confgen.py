# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

# Load configuration types
import moo.otypes

moo.otypes.load_types('rcif/cmd.jsonnet')
moo.otypes.load_types('appfwk/cmd.jsonnet')
moo.otypes.load_types('appfwk/app.jsonnet')

moo.otypes.load_types('dfmodules/triggerrecordbuilder.jsonnet')
moo.otypes.load_types('readout/fakecardreader.jsonnet')
moo.otypes.load_types('readout/datalinkhandler.jsonnet')

moo.otypes.load_types('dqm/dqmprocessor.jsonnet')

# Import new types
import dunedaq.cmdlib.cmd as basecmd # AddressedCmd, 
import dunedaq.rcif.cmd as rccmd # AddressedCmd, 
import dunedaq.appfwk.cmd as cmd # AddressedCmd, 
import dunedaq.appfwk.app as app # AddressedCmd, 
import dunedaq.dfmodules.triggerrecordbuilder as trb
import dunedaq.readout.fakecardreader as fcr
import dunedaq.readout.datalinkhandler as dlh
import dunedaq.dqm.dqmprocessor as dqmprocessor

from appfwk.utils import mcmd, mrccmd, mspec

import json
import math
# Time to wait on pop()
QUEUE_POP_WAIT_MS=100;
# local clock speed Hz
CLOCK_SPEED_HZ = 50000000;

def generate(
        FRONTEND_TYPE='wib',
        NUMBER_OF_DATA_PRODUCERS=1,
        DATA_RATE_SLOWDOWN_FACTOR=10,
        RUN_NUMBER=333,
        TRIGGER_RATE_HZ=1.0,
        DATA_FILE="./frames.bin",
        OUTPUT_PATH=".",
        DISABLE_OUTPUT=False,
        TOKEN_COUNT=10
    ):
    
    trigger_interval_ticks = math.floor((1/TRIGGER_RATE_HZ) *
                                        CLOCK_SPEED_HZ/DATA_RATE_SLOWDOWN_FACTOR)

    # Define modules and queues
    queue_bare_specs = [
            app.QueueSpec(inst="time_sync_dqm_q", kind='FollyMPMCQueue', capacity=100),
            app.QueueSpec(inst="trigger_decision_q_dqm", kind='FollySPSCQueue', capacity=20),
            app.QueueSpec(inst="trigger_record_q_dqm", kind='FollySPSCQueue', capacity=20),
            app.QueueSpec(inst="data_fragments_q", kind='FollyMPMCQueue', capacity=20*NUMBER_OF_DATA_PRODUCERS),
            app.QueueSpec(inst="data_fragments_q_dqm", kind='FollyMPMCQueue', capacity=20*NUMBER_OF_DATA_PRODUCERS),
        ] + [
            app.QueueSpec(inst=f"wib_fake_link_{idx}", kind='FollySPSCQueue', capacity=100000)
                for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ] + [

            app.QueueSpec(inst=f"data_requests_{idx}", kind='FollySPSCQueue', capacity=1000)
                for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ] + [
            app.QueueSpec(inst=f"{FRONTEND_TYPE}_link_{idx}", kind='FollySPSCQueue', capacity=100000)
                for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ] + [
            app.QueueSpec(inst=f"{FRONTEND_TYPE}_recording_link_{idx}", kind='FollySPSCQueue', capacity=100000)
                for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ]
    

    # Only needed to reproduce the same order as when using jsonnet
    queue_specs = app.QueueSpecs(sorted(queue_bare_specs, key=lambda x: x.inst))


    mod_specs = [
        mspec("fake_source", "FakeCardReader", [

                        app.QueueInfo(name=f"output_{idx}", inst=f"{FRONTEND_TYPE}_link_{idx}", dir="output")
                            for idx in range(NUMBER_OF_DATA_PRODUCERS)
                        ]),
        ] + [
        mspec(f"datahandler_{idx}", "DataLinkHandler", [
                        app.QueueInfo(name="raw_input", inst=f"{FRONTEND_TYPE}_link_{idx}", dir="input"),
                        app.QueueInfo(name="timesync", inst="time_sync_dqm_q", dir="output"),
                        # app.QueueInfo(name="requests", inst=f"data_requests_{idx}", dir="input"),
                        # app.QueueInfo(name="fragments_dqm", inst="data_fragments_q_dqm", dir="output"),
                        app.QueueInfo(name="data_requests_0", inst=f"data_requests_{idx}", dir="input"),
                        app.QueueInfo(name="data_response_0", inst="data_fragments_q_dqm", dir="output"),
                        app.QueueInfo(name="raw_recording", inst=f"{FRONTEND_TYPE}_recording_link_{idx}", dir="output")
                        ]) for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ] + [
        mspec("trb_dqm", "TriggerRecordBuilder", [
                        app.QueueInfo(name="trigger_decision_input_queue", inst="trigger_decision_q_dqm", dir="input"),
                        app.QueueInfo(name="trigger_record_output_queue", inst="trigger_record_q_dqm", dir="output"),
                        app.QueueInfo(name="data_fragment_input_queue", inst="data_fragments_q_dqm", dir="input")
                    ] + [
                        app.QueueInfo(name=f"data_request_{idx}_output_queue", inst=f"data_requests_{idx}", dir="output")
                            for idx in range(NUMBER_OF_DATA_PRODUCERS)
                    ]),
        ] + [
        mspec("dqmprocessor", "DQMProcessor", [
                        app.QueueInfo(name="trigger_record_dqm_processor", inst="trigger_record_q_dqm", dir="input"),
                        app.QueueInfo(name="trigger_decision_dqm_processor", inst="trigger_decision_q_dqm", dir="output"),
                        app.QueueInfo(name="timesync_dqm_processor", inst="time_sync_dqm_q", dir="input"),
                    ]),

        ]

    init_specs = app.Init(queues=queue_specs, modules=mod_specs)

    jstr = json.dumps(init_specs.pod(), indent=4, sort_keys=True)
    print(jstr)

    initcmd = rccmd.RCCommand(
        id=basecmd.CmdId("init"),
        entry_state="NONE",
        exit_state="INITIAL",
        data=init_specs
    )

    if TOKEN_COUNT > 0:
        df_token_count = 0
        trigemu_token_count = TOKEN_COUNT
    else:
        df_token_count = -1 * TOKEN_COUNT
        trigemu_token_count = 0

    confcmd = mrccmd("conf", "INITIAL", "CONFIGURED",[
                ("fake_source",fcr.Conf(
                            link_confs=[fcr.LinkConfiguration(
                                geoid=fcr.GeoID(system="TPC", region=0, element=idx),
                                slowdown=DATA_RATE_SLOWDOWN_FACTOR,
                                queue_name=f"output_{idx}"
                            ) for idx in range(NUMBER_OF_DATA_PRODUCERS)],
                            # input_limit=10485100, # default
                            queue_timeout_ms=QUEUE_POP_WAIT_MS,
			                set_t0_to=0
                        )),
            ] + [
                (f"datahandler_{idx}", dlh.Conf(
                        source_queue_timeout_ms=QUEUE_POP_WAIT_MS,
                        fake_trigger_flag=0,
                        latency_buffer_size=3*CLOCK_SPEED_HZ/(25*12*DATA_RATE_SLOWDOWN_FACTOR),
                        pop_limit_pct=0.8,
                        pop_size_pct=0.1,
                        apa_number=0,
                        link_number=idx
                        )) for idx in range(NUMBER_OF_DATA_PRODUCERS)
            ] + [
                ("trb_dqm", trb.ConfParams(
                        general_queue_timeout=QUEUE_POP_WAIT_MS,
                        map=trb.mapgeoidqueue([
                                trb.geoidinst(region=0, element=idx, system="TPC", queueinstance=f"data_requests_{idx}") for idx in range(NUMBER_OF_DATA_PRODUCERS)
                            ]),
                        )),
            ] + [
                ('dqmprocessor', dqmprocessor.Conf(
                        mode='normal', # normal or debug
                        sdqm=[1, 1, 1],
                        ))
            ]
                     )
    
    jstr = json.dumps(confcmd.pod(), indent=4, sort_keys=True)
    print(jstr)

    startpars = rccmd.StartParams(run=RUN_NUMBER, trigger_interval_ticks=trigger_interval_ticks, disable_data_storage=DISABLE_OUTPUT)
    startcmd = mrccmd("start", "CONFIGURED", "RUNNING", [
            ("fake_source", startpars),
            ("datahandler_.*", startpars),
            ("trb_dqm", startpars),
            ("dqmprocessor", startpars),
        ])

    jstr = json.dumps(startcmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nStart\n\n", jstr)


    stopcmd = mrccmd("stop", "RUNNING", "CONFIGURED", [
            ("fake_source", None),
            ("datahandler_.*", None),
            ("trb_dqm", None),
            ("dqmprocessor", None),
        ])

    jstr = json.dumps(stopcmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nStop\n\n", jstr)

    pausecmd = mrccmd("pause", "RUNNING", "RUNNING", [
            ("", None)
        ])

    jstr = json.dumps(pausecmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nPause\n\n", jstr)

    resumecmd = mrccmd("resume", "RUNNING", "RUNNING", [
        ])

    jstr = json.dumps(resumecmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nResume\n\n", jstr)

    scrapcmd = mrccmd("scrap", "CONFIGURED", "INITIAL", [
            ("", None)
        ])

    jstr = json.dumps(scrapcmd.pod(), indent=4, sort_keys=True)
    print("="*80+"\nScrap\n\n", jstr)

    # Create a list of commands
    cmd_seq = [initcmd, confcmd, startcmd, stopcmd, pausecmd, resumecmd, scrapcmd]

    # Print them as json (to be improved/moved out)
    jstr = json.dumps([c.pod() for c in cmd_seq], indent=4, sort_keys=True)
    return jstr
        
if __name__ == '__main__':
    # Add -h as default help option
    CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'])

    import click

    @click.command(context_settings=CONTEXT_SETTINGS)
    @click.option('-n', '--number-of-data-producers', default=1)
    @click.option('-e', '--emulator-mode', is_flag=True)
    @click.option('-s', '--data-rate-slowdown-factor', default=10)
    @click.option('-r', '--run-number', default=333)
    @click.option('-t', '--trigger-rate-hz', default=1.0)
    @click.option('-d', '--data-file', type=click.Path(), default='./frames.bin')
    @click.option('-o', '--output-path', type=click.Path(), default='.')
    @click.option('--disable-data-storage', is_flag=True)
    @click.option('-c', '--token-count', default=10)
    @click.argument('json_file', type=click.Path(), default='dqm.json')
    def cli(number_of_data_producers, emulator_mode, data_rate_slowdown_factor,
            run_number, trigger_rate_hz, data_file, output_path, disable_data_storage,
            token_count, json_file):
        """
          JSON_FILE: Input raw data file.
          JSON_FILE: Output json configuration file.
        """

        with open(json_file, 'w') as f:
            f.write(generate(
                    NUMBER_OF_DATA_PRODUCERS=number_of_data_producers,
                    DATA_RATE_SLOWDOWN_FACTOR=data_rate_slowdown_factor,
                    RUN_NUMBER=run_number,
                    TRIGGER_RATE_HZ=trigger_rate_hz,
                    DATA_FILE=data_file,
                    OUTPUT_PATH=output_path,
                    DISABLE_OUTPUT=disable_data_storage,
                    TOKEN_COUNT=token_count
                ))

        print(f"'{json_file}' generation completed.")

    cli()

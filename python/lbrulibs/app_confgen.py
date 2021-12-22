# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io

moo.io.default_load_path = get_moo_model_path()

# Load configuration types
import moo.otypes

moo.otypes.load_types("cmdlib/cmd.jsonnet")
moo.otypes.load_types("rcif/cmd.jsonnet")
moo.otypes.load_types("appfwk/cmd.jsonnet")
moo.otypes.load_types("appfwk/app.jsonnet")
moo.otypes.load_types("readoutlibs/readoutconfig.jsonnet")
moo.otypes.load_types("readoutlibs/recorderconfig.jsonnet")
moo.otypes.load_types("nwqueueadapters/queuetonetwork.jsonnet")
moo.otypes.load_types("nwqueueadapters/networkobjectsender.jsonnet")
moo.otypes.load_types('networkmanager/nwmgr.jsonnet')
moo.otypes.load_types('lbrulibs/pacmancardreader.jsonnet')

# Import new types
import dunedaq.cmdlib.cmd as basecmd  # AddressedCmd,
import dunedaq.rcif.cmd as rccmd  # AddressedCmd,
import dunedaq.appfwk.app as app  # AddressedCmd,
import dunedaq.appfwk.cmd as cmd  # AddressedCmd,
import dunedaq.readoutlibs.readoutconfig as rconf
import dunedaq.readoutlibs.recorderconfig as bfs
import dunedaq.nwqueueadapters.queuetonetwork as qton
import dunedaq.nwqueueadapters.networkobjectsender as nos
import dunedaq.networkmanager.nwmgr as nwmgr
import dunedaq.lbrulibs.pacmancardreader as pcr

from appfwk.utils import mcmd, mrccmd, mspec

import json
import math

# Time to waait on pop()
QUEUE_POP_WAIT_MS = 100
# local clock speed Hz
CLOCK_SPEED_HZ = 50000000


def generate(
    FRONTEND_TYPE="pacman",
    NUMBER_OF_DATA_PRODUCERS=1,
    NUMBER_OF_TP_PRODUCERS=1,
    DATA_RATE_SLOWDOWN_FACTOR=1,
    ENABLE_SOFTWARE_TPG=False,
    RUN_NUMBER=333,
    DATA_FILE="./frames.bin",
    TP_DATA_FILE="./tp_frames.bin",
):

    # Define modules and queues
    queue_bare_specs = (
        [
            app.QueueSpec(inst="time_sync_q", kind="FollyMPMCQueue", capacity=100),
            app.QueueSpec(inst="data_fragments_q", kind="FollyMPMCQueue", capacity=100),
            app.QueueSpec(
                inst="errored_frames_q", kind="FollyMPMCQueue", capacity=10000
            ),
        ]
        + [
            app.QueueSpec(
                inst=f"data_requests_{idx}", kind="FollySPSCQueue", capacity=1000
            )
            for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ]
        + [
            app.QueueSpec(
                inst=f"{FRONTEND_TYPE}_link_{idx}",
                kind="FollySPSCQueue",
                capacity=100000,
            )
            for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ]
        + [
            app.QueueSpec(
                inst=f"raw_tp_link_{idx}", kind="FollySPSCQueue", capacity=100000
            )
            for idx in range(
                NUMBER_OF_DATA_PRODUCERS,
                NUMBER_OF_DATA_PRODUCERS + NUMBER_OF_TP_PRODUCERS,
            )
        ]
        + [
            app.QueueSpec(
                inst=f"sw_tp_queue_{idx}", kind="FollySPSCQueue", capacity=100000
            )
            for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ]
        + [
            app.QueueSpec(
                inst=f"tp_data_requests", kind="FollySPSCQueue", capacity=1000
            )
        ]
        + [
            app.QueueSpec(
                inst=f"tpset_link_{idx}", kind="FollySPSCQueue", capacity=10000
            )
            for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ]
    )

    # Only needed to reproduce the same order as when using jsonnet
    queue_specs = app.QueueSpecs(sorted(queue_bare_specs, key=lambda x: x.inst))

    mod_specs = (
        [
            mspec(
                "fake_source",
                "PacmanCardReader",
                [
                    app.QueueInfo(
                        name=f"output_{idx}",
                        inst=f"{FRONTEND_TYPE}_link_{idx}",
                        dir="output",
                    )
                    for idx in range(NUMBER_OF_DATA_PRODUCERS)
                ]
            ),
        ]
        + [
            mspec(
                f"datahandler_{idx}",
                "DataLinkHandler",
                [
                    app.QueueInfo(
                        name="raw_input",
                        inst=f"{FRONTEND_TYPE}_link_{idx}",
                        dir="input",
                    ),
                    app.QueueInfo(name="timesync", inst="time_sync_q", dir="output"),
                    app.QueueInfo(
                        name="data_requests_0", inst=f"data_requests_{idx}", dir="input"
                    ),
                    app.QueueInfo(
                        name="fragment_queue", inst="data_fragments_q", dir="output"
                    ),
                    app.QueueInfo(
                        name="tp_out", inst=f"sw_tp_queue_{idx}", dir="output"
                    ),
                    app.QueueInfo(
                        name="tpset_out", inst=f"tpset_link_{idx}", dir="output"
                    ),
                    app.QueueInfo(
                        name="errored_frames", inst="errored_frames_q", dir="output"
                    ),
                ],
            )
            for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ]
        + [
            mspec(
                f"timesync_consumer",
                "TimeSyncConsumer",
                [app.QueueInfo(name="input_queue", inst=f"time_sync_q", dir="input")],
            )
        ]
        + [
            mspec(
                f"fragment_consumer",
                "FragmentConsumer",
                [
                    app.QueueInfo(
                        name="input_queue", inst=f"data_fragments_q", dir="input"
                    )
                ],
            )
        ]
        + [
            mspec(
                f"sw_tp_handler_{idx}",
                "DataLinkHandler",
                [
                    app.QueueInfo(
                        name="raw_input", inst=f"sw_tp_queue_{idx}", dir="input"
                    ),
                    app.QueueInfo(name="timesync", inst="time_sync_q", dir="output"),
                    app.QueueInfo(
                        name="requests", inst="tp_data_requests", dir="input"
                    ),
                    app.QueueInfo(
                        name="fragment_queue", inst="data_fragments_q", dir="output"
                    ),
                ],
            )
            for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ]
        + [
            mspec(
                f"tpset_publisher_{idx}",
                "QueueToNetwork",
                [app.QueueInfo(name="input", inst=f"tpset_link_{idx}", dir="input")],
            )
            for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ]
        + [
            mspec(
                f"raw_tp_handler_{idx}",
                "DataLinkHandler",
                [
                    app.QueueInfo(
                        name="raw_input", inst=f"raw_tp_link_{idx}", dir="input"
                    ),
                    app.QueueInfo(name="timesync", inst="time_sync_q", dir="output"),
                ],
            )
            for idx in range(
                NUMBER_OF_DATA_PRODUCERS,
                NUMBER_OF_DATA_PRODUCERS + NUMBER_OF_TP_PRODUCERS,
            )
        ]
        + [
            mspec(
                "errored_frame_consumer",
                "ErroredFrameConsumer",
                [
                    app.QueueInfo(
                        name="input_queue", inst="errored_frames_q", dir="input"
                    )
                ],
            )
        ]
    )

    nw_specs = [nwmgr.Connection(name=f"tpsets_{idx}",topics=["foo"],  address="tcp://127.0.0.1:" + str(5000 + idx)) for idx in range(NUMBER_OF_DATA_PRODUCERS)]
    nw_specs.append(nwmgr.Connection(name="timesync", topics=["Timesync"], address="tcp://127.0.0.1:6000"))

    init_specs = app.Init(queues=queue_specs, modules=mod_specs, nwconnections=nw_specs)

    jstr = json.dumps(init_specs.pod(), indent=4, sort_keys=True)
    print(jstr)

    initcmd = rccmd.RCCommand(
        id=basecmd.CmdId("init"),
        entry_state="NONE",
        exit_state="INITIAL",
        data=init_specs,
    )

    confcmd = mrccmd(
        "conf",
        "INITIAL",
        "CONFIGURED",
        [
            (
                "fake_source",
                pcr.Conf(
                    link_confs=[
                        pcr.LinkConfiguration(
                            geoid=pcr.GeoID(system="kNDLarTPC", region=0, element=idx),
                        )
                        for idx in range(NUMBER_OF_DATA_PRODUCERS)
                    ]
                    + [
                        pcr.LinkConfiguration(
                            geoid=sec.GeoID(system="TPC", region=0, element=idx),
                        )
                        for idx in range(
                            NUMBER_OF_DATA_PRODUCERS,
                            NUMBER_OF_DATA_PRODUCERS + NUMBER_OF_TP_PRODUCERS,
                        )
                    ],
                    # input_limit=10485100, # default
                ),
            ),
        ]
        + [
            (
                f"datahandler_{idx}",
                rconf.Conf(
                    readoutmodelconf=rconf.ReadoutModelConf(
                        source_queue_timeout_ms=QUEUE_POP_WAIT_MS,
                        fake_trigger_flag=1,
                        region_id=0,
                        element_id=idx,
                        timesync_connection_name = f"timesync",
                        timesync_topic_name = "Timesync",

                    ),
                    latencybufferconf=rconf.LatencyBufferConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        region_id=0,
                        element_id=idx,
                    ),
                    rawdataprocessorconf=rconf.RawDataProcessorConf(
                        region_id=0,
                        element_id=idx,
                        enable_software_tpg=ENABLE_SOFTWARE_TPG,
                        error_counter_threshold=100,
                        error_reset_freq=10000,
                    ),
                    requesthandlerconf=rconf.RequestHandlerConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        pop_limit_pct=0.8,
                        pop_size_pct=0.1,
                        region_id=0,
                        element_id=idx,
                        output_file=f"output_{idx}.out",
                        stream_buffer_size=8388608,
                        enable_raw_recording=True,
                    ),
                ),
            )
            for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ]
        + [
            (
                f"sw_tp_handler_{idx}",
                rconf.Conf(
                    readoutmodelconf=rconf.ReadoutModelConf(
                        source_queue_timeout_ms=QUEUE_POP_WAIT_MS,
                        fake_trigger_flag=1,
                        region_id=0,
                        element_id=idx,
                    ),
                    latencybufferconf=rconf.LatencyBufferConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        region_id=0,
                        element_id=idx,
                    ),
                    rawdataprocessorconf=rconf.RawDataProcessorConf(
                        region_id=0,
                        element_id=idx,
                        enable_software_tpg=ENABLE_SOFTWARE_TPG,
                    ),
                    requesthandlerconf=rconf.RequestHandlerConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        pop_limit_pct=0.8,
                        pop_size_pct=0.1,
                        region_id=0,
                        element_id=idx,
                        output_file=f"output_{idx}.out",
                        stream_buffer_size=8388608,
                        enable_raw_recording=False,
                    ),
                ),
            )
            for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ]
        + [
            (
                f"raw_tp_handler_{idx}",
                rconf.Conf(
                    readoutmodelconf=rconf.ReadoutModelConf(
                        source_queue_timeout_ms=QUEUE_POP_WAIT_MS,
                        fake_trigger_flag=1,
                        region_id=0,
                        element_id=idx,
                    ),
                    latencybufferconf=rconf.LatencyBufferConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        region_id=0,
                        element_id=idx,
                    ),
                    rawdataprocessorconf=rconf.RawDataProcessorConf(
                        region_id=0,
                        element_id=idx,
                        enable_software_tpg=ENABLE_SOFTWARE_TPG,
                    ),
                    requesthandlerconf=rconf.RequestHandlerConf(
                        latency_buffer_size=3
                        * CLOCK_SPEED_HZ
                        / (25 * 12 * DATA_RATE_SLOWDOWN_FACTOR),
                        pop_limit_pct=0.8,
                        pop_size_pct=0.1,
                        region_id=0,
                        element_id=idx,
                        output_file=f"output_{idx}.out",
                        stream_buffer_size=8388608,
                        enable_raw_recording=False,
                    ),
                ),
            )
            for idx in range(
                NUMBER_OF_DATA_PRODUCERS,
                NUMBER_OF_DATA_PRODUCERS + NUMBER_OF_TP_PRODUCERS,
            )
        ]
        + [
            (
                f"tpset_publisher_{idx}",
                qton.Conf(
                    msg_type="dunedaq::trigger::TPSet",
                    msg_module_name="TPSetNQ",
                    sender_config=nos.Conf(
                        name=f"tpsets_{idx}", 
                        topic="foo",
                        stype="msgpack",
                    ),
                ),
            )
            for idx in range(NUMBER_OF_DATA_PRODUCERS)
        ],
    )

    jstr = json.dumps(confcmd.pod(), indent=4, sort_keys=True)
    print(jstr)

    startpars = rccmd.StartParams(run=RUN_NUMBER)
    startcmd = mrccmd(
        "start",
        "CONFIGURED",
        "RUNNING",
        [
            ("datahandler_.*", startpars),
            ("fake_source", startpars),
            ("data_recorder_.*", startpars),
            ("timesync_consumer", startpars),
            ("fragment_consumer", startpars),
            ("sw_tp_handler_.*", startpars),
            ("raw_tp_handler_.*", startpars),
            ("tpset_publisher_.*", startpars),
            ("errored_frame_consumer", startpars),
        ],
    )

    jstr = json.dumps(startcmd.pod(), indent=4, sort_keys=True)
    print("=" * 80 + "\nStart\n\n", jstr)

    stopcmd = mrccmd(
        "stop",
        "RUNNING",
        "CONFIGURED",
        [
            ("fake_source", None),
            ("datahandler_.*", None),
            ("data_recorder_.*", None),
            ("timesync_consumer", None),
            ("fragment_consumer", None),
            ("sw_tp_handler_.*", None),
            ("raw_tp_handler_.*", None),
            ("tpset_publisher_.*", None),
            ("errored_frame_consumer", None),
        ],
    )

    jstr = json.dumps(stopcmd.pod(), indent=4, sort_keys=True)
    print("=" * 80 + "\nStop\n\n", jstr)

    scrapcmd = mrccmd("scrap", "CONFIGURED", "INITIAL", [("", None)])

    jstr = json.dumps(scrapcmd.pod(), indent=4, sort_keys=True)
    print("=" * 80 + "\nScrap\n\n", jstr)

    # Create a list of commands
    cmd_seq = [initcmd, confcmd, startcmd, stopcmd, scrapcmd]

    record_cmd = mrccmd(
        "record",
        "RUNNING",
        "RUNNING",
        [("datahandler_.*", rconf.RecordingParams(duration=10))],
    )

    jstr = json.dumps(record_cmd.pod(), indent=4, sort_keys=True)
    print("=" * 80 + "\nRecord\n\n", jstr)

    cmd_seq.append(record_cmd)

    # Print them as json (to be improved/moved out)
    jstr = json.dumps([c.pod() for c in cmd_seq], indent=4, sort_keys=True)
    return jstr


if __name__ == "__main__":
    # Add -h as default help option
    CONTEXT_SETTINGS = dict(help_option_names=["-h", "--help"])

    import click

    @click.command(context_settings=CONTEXT_SETTINGS)
    @click.option(
        "-f",
        "--frontend-type",
        type=click.Choice(
            ["wib", "wib2", "pds_queue", "pds_list", "pacman"], case_sensitive=True
        ),
        default="pacman",
    )
    @click.option("-n", "--number-of-data-producers", default=1)
    @click.option("-t", "--number-of-tp-producers", default=0)
    @click.option("-s", "--data-rate-slowdown-factor", default=10)
    @click.option("-g", "--enable-software-tpg", is_flag=True)
    @click.option("-r", "--run-number", default=333)
    @click.option("-d", "--data-file", type=click.Path(), default="./frames.bin")
    @click.option("--tp-data-file", type=click.Path(), default="./tp_frames.bin")
    @click.argument("json_file", type=click.Path(), default="fake_NDreadout.json")
    def cli(
        frontend_type,
        number_of_data_producers,
        number_of_tp_producers,
        data_rate_slowdown_factor,
        enable_software_tpg,
        run_number,
        data_file,
        tp_data_file,
        json_file,
    ):
        """
        JSON_FILE: Input raw data file.
        JSON_FILE: Output json configuration file.
        """

        with open(json_file, "w") as f:
            f.write(
                generate(
                    FRONTEND_TYPE=frontend_type,
                    NUMBER_OF_DATA_PRODUCERS=number_of_data_producers,
                    NUMBER_OF_TP_PRODUCERS=number_of_tp_producers,
                    DATA_RATE_SLOWDOWN_FACTOR=data_rate_slowdown_factor,
                    ENABLE_SOFTWARE_TPG=enable_software_tpg,
                    RUN_NUMBER=run_number,
                    DATA_FILE=data_file,
                    TP_DATA_FILE=tp_data_file,
                )
            )

        print(f"'{json_file}' generation completed.")

    cli()

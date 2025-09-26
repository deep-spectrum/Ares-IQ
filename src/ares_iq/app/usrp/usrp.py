import numpy as np
from ares_iq.print_utils import print_error, CaptureProgress
from ares_iq.configurations import load_config_section, save_config_section
from ares_iq.iq_data import IQData
from abc import ABC, abstractmethod
import math
import typer
from typing_extensions import Annotated

try:
    import uhd
except ImportError:
    from ares_iq.uhd_installation import install_uhd
    import sys
    import os

    install_uhd()
    os.execv(sys.executable, [sys.executable] + sys.argv)


class UsrpDevice(ABC):
    _usrp: uhd.usrp.MultiUSRP
    _center: float
    _bw: float
    _rx_streamer: uhd.usrp.RXStreamer
    _rx_meta: uhd.types.RXMetadata
    _iq_data: list[IQData] = []
    _quantized_data: list[None] = []
    app = typer.Typer()

    def _find_usrp(self):
        try:
            self._usrp = uhd.usrp.MultiUSRP(f"type={self.type}")
        except RuntimeError as err:
            print_error(str(err))

    def _configure_usrp(self):
        self._usrp.set_rx_freq(self._center)
        self._usrp.set_rx_bandwidth(self._bw)

        configs = load_config_section("usrp")

        if "spp" in configs:
            spp = configs["spp"]
        else:
            spp = 200

        stream_args = uhd.usrp.StreamArgs("fc32", "sc16")
        stream_args.args = f"spp={spp}"
        self._rx_streamer = self._usrp.get_rx_stream(stream_args)
        self._rx_meta = uhd.types.RXMetadata()

    def _start_stream(self):
        stream_cmd = uhd.types.StreamCMD(uhd.types.StreamMode.start_cont)
        stream_cmd.stream_now = True
        self._rx_streamer.issue_stream_cmd(stream_cmd)

    def _stop_stream(self):
        stream_cmd = uhd.types.StreamCMD(uhd.types.StreamMode.stop_cont)
        self._rx_streamer.issue_stream_cmd(stream_cmd)


    def capture_iq(self, center: float, bw: float, file_size_gb: float):
        self._center = center
        self._bw = bw
        self._find_usrp()
        self._configure_usrp()

        file_size = file_size_gb * 1e9
        samples_per_capture = self._rx_streamer.get_max_num_samps()
        bytes_per_capture = (samples_per_capture * 8) + 8
        captures = math.ceil(file_size / bytes_per_capture)
        self._iq_data = [IQData() for _ in range(captures)]

        try:
            with CaptureProgress(captures, samples_per_capture) as progress:
                self._start_stream()
                for iq in self._iq_data:
                    recv_buffer = np.zeros(samples_per_capture, dtype=np.complex64)
                    samples = self._rx_streamer.recv(recv_buffer, self._rx_meta)
                    if samples != samples_per_capture:
                        continue  # TODO: Figure out how to deal with underflows
                    iq.iq = recv_buffer
                    iq.ts_sec = self._rx_meta.time_spec.get_full_secs()
                    iq.ts_nsec = int(self._rx_meta.time_spec.get_frac_secs() * 1e9)
                    progress.update()
                progress.update()
        except KeyboardInterrupt:
            pass

        self._stop_stream()

        self._quantize()

    @property
    def iq_data(self) -> list[IQData]:
        return self._iq_data

    @property
    def quantized_data(self) -> list[None]:
        return self._quantized_data

    @abstractmethod
    def _quantize(self):
        pass

    @property
    @abstractmethod
    def type(self):
        pass

    @staticmethod
    @app.command(name='usrp-config', help="Set configurations for the USRP platform")
    def config(samples: Annotated[int | None, typer.Option("--spp", "-s", help="Samples per packet")] = None):
        # TODO: add other configs
        configs = load_config_section("usrp")
        if samples is not None:
            configs["spp"] = str(samples)
        save_config_section("usrp", configs)

import numpy as np
from ares_iq.print_utils import print_error
from ares_iq.configurations import load_config_section
from ares_iq.iq_data import IQData
from abc import ABC, abstractmethod

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

    def _find_usrp(self):
        try:
            self._usrp = uhd.usrp.MultiUSRP(f"type={self.type}")
        except RuntimeError as err:
            print_error(str(err))

    def _configure_usrp(self):
        self._usrp.set_rx_freq(self._center)
        self._usrp.set_rx_bandwidth(self._bw)

        # TODO: Stream arguments
        stream_args = uhd.usrp.StreamArgs("fc32", "sc16")
        stream_args.args = "spp=200"
        self._rx_streamer = self._usrp.get_rx_stream(stream_args)
        self._rx_meta = uhd.types.RXMetadata()

    def _start_stream(self):
        stream_cmd = uhd.types.StreamCMD(uhd.types.StreamMode.start_cont)
        stream_cmd.stream_now = True
        self._rx_streamer.issue_stream_cmd(stream_cmd)

    def _stop_stream(self):
        stream_cmd = uhd.types.StreamCMD(uhd.types.StreamMode.stop_cont)
        self._rx_streamer.issue_stream_cmd(stream_cmd)


    def capture_iq(self, center: float, bw: float, file_size: float):
        self._center = center
        self._bw = bw
        self._find_usrp()
        self._configure_usrp()

        # TODO: calculate size

        self._start_stream()
        while True:
            recv_buffer = np.zeros(self._rx_streamer.get_max_num_samps(), dtype=np.complex64)
            samples = self._rx_streamer.recv(recv_buffer, self._rx_meta)
            # TODO: Add data to list
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

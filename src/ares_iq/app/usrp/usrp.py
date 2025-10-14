from ares_iq_ext.usrp import _USRP
from ares_iq.iq_data import IQData
from decimal import Decimal
import typer
from typing_extensions import Annotated
from ares_iq.configurations import load_config_section, save_config_section


class USRP(_USRP):
    _iq_data: list[IQData]
    _quantized_data: list[None]
    app = typer.Typer()

    def _stream_args(self):
        configs = load_config_section("usrp-stream-configs")

        if "spp" not in configs:
            spp = 200
        else:
            spp = int(configs["spp"])

        self._set_stream_args(spp)

    def capture_iq(self, center: float, bw: float, file_size: float):
        self._stream_args()
        iq_data, timestamps = super().capture_iq(center, bw, file_size)

        self._iq_data = [IQData() for _ in range(len(timestamps))]
        for data, ts, iq in zip(iq_data, timestamps, self._iq_data):
            iq.iq = data
            iq.ts_sec = int(ts)
            iq.ts_nsec = int((Decimal(ts) - iq.ts_sec) * Decimal('1e9'))

        # TODO quantize

    @property
    def iq_data(self) -> list[IQData]:
        return self._iq_data

    @property
    def quantized_data(self) -> list[None]:
        return self._quantized_data

    @staticmethod
    @app.command('usrp-stream-args', help='Set USRP platform stream arguments')
    def stream_args(spp: Annotated[int | None, typer.Option(help="Samples per packet")] = None):
        configs = load_config_section("usrp-stream-configs")
        if spp is not None:
            configs["spp"] = str(spp)

        save_config_section("usrp-stream-configs", configs)

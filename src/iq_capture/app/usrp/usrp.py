import uhd
import numpy as np
from iq_capture.print_utils import print_error


def _find_usrp(platform_type: str = 'x300') -> uhd.usrp.MultiUSRP:
    try:
        return uhd.usrp.MultiUSRP(f"type={platform_type}")
    except RuntimeError as e:
        print_error(str(e))
        raise


def collect_usrp_iq_data():
    usrp = _find_usrp()

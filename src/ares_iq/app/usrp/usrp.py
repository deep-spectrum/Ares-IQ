# import uhd
# import numpy as np
# from ares_iq.print_utils import print_error
# from ares_iq.configurations import load_config_section
#
#
# def _find_usrp(platform_type: str = 'x300') -> uhd.usrp.MultiUSRP:
#     try:
#         return uhd.usrp.MultiUSRP(f"type={platform_type}")
#     except RuntimeError as e:
#         print_error(str(e))
#         raise
#
#
# def collect_usrp_iq_data():
#     configs = load_config_section("platform")
#     usrp = _find_usrp(configs["hw"])

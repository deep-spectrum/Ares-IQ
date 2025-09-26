from .usrp import UsrpDevice


class X410Device(UsrpDevice):
    def _quantize(self):
        pass

    @property
    def type(self):
        return "x400"

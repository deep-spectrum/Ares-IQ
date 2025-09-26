from .usrp import UsrpDevice


class X310Device(UsrpDevice):
    def _quantize(self):
        pass

    @property
    def type(self):
        return "x300"

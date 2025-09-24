from .usrp import UsrpDevice


class X310Device(UsrpDevice):
    def _quantize(self):
        pass

    def type(self):
        return "x300"

from attrs import Attribute


def clamp_bounds(value, field: Attribute):
    """
    Clamp the set value to the min or max. This must be used in a attrs.Converter with the `take_field` attribute set to `True`.
    In order for this to work properly, a metadata dictionary must be defined with the keys `"min"` and/or `"max"` defined.
    If "min" is not present, there is no minimum value, and if "max" is not defined, there is no maximum value.
    :param value: The new value
    :param field: The attribute field data
    :return: The clamped value if any clamping occurred.
    """
    if field.metadata is None:
        raise AttributeError(f"metadata for {field.name} must be defined in order to use {__name__}")
    if "min" in field.metadata:
        if value < field.metadata["min"]:
            return field.metadata["min"]
    if "max" in field.metadata:
        if value > field.metadata["max"]:
            return field.metadata["max"]
    return value

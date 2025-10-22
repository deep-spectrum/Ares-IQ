from attrs import Attribute


def clamp_bounds(value, field: Attribute):
    """Clamp a field to bounds if value is out of range.

    Clamp the set value to the min or max. This must be used in an
    `attrs.Converter` with the `take_field` parameter set to
    `True`. In order for this to work properly, a metadata dictionary must be
    defined with the keys `"min"` and/or `"max"` defined. If "min" is not
    present, there is no minimum value, and if "max" is not defined, there is
    no maximum value. The bounds are inclusive.

    Typical usage example:

        ``` {.py}
        from attrs import define, field, Converter
        from ares_iq.validators import clamp_bounds
        @define
        class Foo:
            bar: int = field(default=5,
                             metadata={"min": 1, "max: 2},
                             converter=Converter(clamp_bounds, takes_field=True))
        ```

    Args:
        value: The new set value.
        field: The attribute field data.

    Returns:
        The original set value if in range.
        The "max" value if the value is greater than "max".
        The "min" value if the value is less than "min".

    Raises:
        AttributeError: metadata dictionary is not defined.
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

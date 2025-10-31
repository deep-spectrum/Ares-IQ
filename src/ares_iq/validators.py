from attrs import Attribute
from typing import Any


def clamp_bounds(value: float, field: Attribute):
    """Clamp a field to bounds if value is out of range.

    Clamp the set value to the min or max. This must be used in an
    `attrs.Converter` with the `take_field` parameter set to
    `True`. In order for this to work properly, a metadata dictionary must be
    defined with the keys `"min"` and/or `"max"` defined. If "min" is not
    present, there is no minimum value, and if "max" is not defined, there is
    no maximum value. The bounds are inclusive.

    Typical usage example:

    ```py
    from attrs import define, field, Converter
    from ares_iq.validators import clamp_bounds
    @define
    class Foo:
        bar: int = field(default=5,
                         metadata={"min": 1, "max": 10},
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
        ValueError: Unable to cast value to proper type.
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


def power_of_two(_instance: Any, attribute: Attribute, value: int):
    """Check if value is a power of 2.

    Checks if the input value is a power of 2.

    Typical usage example:

    ```py
    from attrs import define, field
    from ares_iq.validators import power_of_two
    @define
    class Foo:
        bar: int = field(default=1, validator=power_of_two)
    ```

    Args:
        _instance: Unused
        attribute: The attribute being set
        value: The value to validate

    Raises:
        ValueError: Value is not a power of 2.
    """
    if not ((value & (value - 1) == 0) and value > 0):
        raise ValueError(f"{attribute.name} must be a power of 2")


def is_positive(_instance: Any, attribute: Attribute, value: float):
    """Validates if the input is positive.

    Validates if the input value is valid. If `0` is considered valid, then
    that must be specified in the metadata with the `"zero_valid"` key. If
    `0` is considered to be invalid, then specifying metadata is not necessary.

    Typical usage example:

    ```py
    from attrs import define, field
    from ares_iq.validators import is_positive
    @define
    class Foo:
        bar: int field(default=1,
                       metadata={"zero_valid": False},  # Not necessary if `0` is considered invalid.
                       validator=is_positive)
    ```

    Args:
        _instance: Unused
        attribute: The attribute being set
        value: The value to validate

    Raises:
        ValueError: Value is not a positive number.
    """
    zero_valid = False
    if attribute.metadata is not None:
        if "zero_valid" in attribute.metadata:
            zero_valid = bool(attribute.metadata["zero_valid"])

    if value < 0 or (value == 0 and not zero_valid):
        raise ValueError(
            f"{attribute.name} must be greater than 0" if not zero_valid else f"{attribute.name} must be greater or equal to 0")

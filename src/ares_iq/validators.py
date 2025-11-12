from attrs import Attribute
from typing import Any


def clamp_bounds(value: float, field: Attribute) -> float:
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


def validate_bounds(_instance: Any, attribute: Attribute, value: float):
    """Validate if the input is within bounds.

    Validates if the input value is within bounds. In order for this to work, a min
    and/or a max must be specified in the metadata. If no min is specified, then there
    is no lower bound. If no max is specified, then there is no upper bound. Specifying
    no bounds acts as a pass through. By default, the bounds are inclusive, however,
    one or both of the bounds can be marked as exclusive through the metadata. In order
    to mark a bound as exclusive, min_exclusive and/or max_exclusive must be specified
    in the metadata with the value being a boolean.

    Typical usage example:

    ```py
    from attrs import define, field
    from ares_iq.validators import validate_bounds
    @define
    class Foo:
        # bar has the following bound validation: 0 < bar <= 10
        bar: int = field(default=1,
                         metadata=metadata={"min": 0,
                                            "max": 10,
                                            "min_exclusive": True,
                                            "max_exclusive": False},
                         validator=validate_bounds)
    ```

    Args:
        _instance: Unused.
        attribute: The attribute being set.
        value: The value to validate

    Raises:
        AttributeError: metadata dictionary missing.
        ValueError: Value is not within bounds.
    """
    if attribute.metadata is None:
        raise AttributeError(f"metadata for {attribute.name} must be defined in order to use {__name__}")

    min_exclusive = False
    max_exclusive = False
    lower_bound = " <= "
    upper_bound = " <= "
    if "min_exclusive" in attribute.metadata:
        min_exclusive = bool(attribute.metadata["min_exclusive"])
        lower_bound = " < " if min_exclusive else " <= "
    if "max_exclusive" in attribute.metadata:
        max_exclusive = bool(attribute.metadata["max_exclusive"])
        upper_bound = " < " if max_exclusive else " <= "

    if "min" in attribute.metadata:
        if value < attribute.metadata["min"] or (min_exclusive and value <= attribute.metadata["min"]):
            raise ValueError(
                f"{attribute.metadata['min']}{lower_bound}{attribute.name}{upper_bound}{attribute.metadata['max']}. "
                f"Got {value}" if "max" in attribute.metadata else
                f"{attribute.metadata['min']}{lower_bound}{attribute.name}. Got {value}.")
    if "max" in attribute.metadata:
        if value > attribute.metadata["max"] or (max_exclusive and value >= attribute.metadata["max"]):
            raise ValueError(
                f"{attribute.metadata['min']}{lower_bound}{attribute.name}{upper_bound}{attribute.metadata['max']}. "
                f"Got {value}" if "min" in attribute.metadata else
                f"{attribute.name}{upper_bound}{attribute.metadata['max']}. Got {value}.")


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

import warnings


def deprecated(reason: str = ""):
    """Mark a function as deprecated.

    Args:
        reason: The reason why its being deprecated and what to use instead.

    Returns:
        The decrated function.
    """
    def decorator(func):
        def wrapper(*args, **kwargs):
            warnings.warn(
                f"{func.__name__}(): {reason}",
                category=DeprecationWarning,
                stacklevel=2
            )
            return func(*args, **kwargs)
        return wrapper
    return decorator

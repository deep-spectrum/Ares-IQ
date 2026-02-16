import yaml
from pathlib import Path
from attrs import asdict


class ConfigBase:
    @staticmethod
    def serialize(inst, field, value):
        """Serializes the value for saving to yaml file.

        Serialize value for YAML file. By default, no serialization takes place,
        however, this should be implemented by the subclass if non-builtin types
        are used in the subclass.

        Args:
            inst: The class instance.
            field: The field being serialized.
            value: The value of the field being serialized.

        Returns:
            The serialized value.
        """
        return value

    def to_yaml(self, config_file: Path | str) -> None:
        """Save configurations to YAML file.

        Save the configurations to the given YAML file. Other YAML contents
        will not be overwritten.

        Args:
            config_file: The file to save the configurations into.

        Notes:
            Any private attributes (starts with the "_" character) will not be
            saved to the YAML file.
        """
        if Path(config_file).exists():
            with open(config_file, 'r') as f:
                config = yaml.safe_load(f)
        else:
            config = {}
        config[self.__class__.__name__] = asdict(
            self, filter=lambda attr, value: not attr.name.startswith("_"), value_serializer=self.serialize)  # type: ignore[arg-type]
        with open(config_file, 'w') as f:
            yaml.safe_dump(config, f)

    @classmethod
    def from_yaml(cls, config_file: Path | str):
        """Load configurations from YAML file.

        Loads the configurations from the given YAML file. If the
        configurations are undefined, the default values are used.

        Args:
            config_file: The file to load the configurations from.

        Returns: A configs class instance.
        """
        if Path(config_file).exists():
            with open(config_file, 'r') as f:
                config = yaml.safe_load(f)
        else:
            config = {}
        if cls.__name__ in config:
            return cls(**config[cls.__name__])
        return cls()

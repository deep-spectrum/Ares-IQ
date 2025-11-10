import yaml
from pathlib import Path
from attrs import asdict


class ConfigBase:
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
            self, filter=lambda attr, value: not attr.name.startswith("_"))  # type: ignore[arg-type]
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

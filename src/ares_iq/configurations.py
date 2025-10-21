from configparser import ConfigParser, SectionProxy
from pathlib import Path
from .typing import PathLike
from .warnings import deprecated


CONFIG_DIR = Path.home() / ".ares_iq"
CONFIG_FILE = CONFIG_DIR / "config.ini"


@deprecated("Migrate to PyYAML")
def load_configs(config_file: PathLike) -> ConfigParser:
    config = ConfigParser()
    config.read(config_file)
    return config


@deprecated("Migrate to PyYAML")
def load_config_section(section: str, config_file: PathLike | None = None) -> SectionProxy:
    if config_file is None:
        config_file = CONFIG_FILE
    config = load_configs(config_file)
    if section in config.sections():
        return config[section]
    config[section] = {}
    return config[section]


@deprecated("Migrate to PyYAML")
def save_configs(config_file: PathLike, config: ConfigParser) -> None:
    with open(config_file, 'w') as f:
        config.write(f)


@deprecated("Migrate to PyYAML")
def save_config_section(section: str, section_configs: dict | SectionProxy, config_file: PathLike | None = None) -> None:
    if config_file is None:
        config_file = CONFIG_FILE
    config = load_configs(config_file)
    config[section] = section_configs
    save_configs(config_file, config)

from configparser import ConfigParser, SectionProxy
from pathlib import Path


CONFIG_DIR = Path.home() / ".iq_capture"
CONFIG_FILE = CONFIG_DIR / "config.ini"


def load_configs(config_file: str) -> ConfigParser:
    config = ConfigParser()
    config.read(config_file)
    return config


def load_config_section(section: str, config_file: str | None = None) -> SectionProxy:
    if config_file is None:
        config_file = CONFIG_FILE
    config = load_configs(config_file)
    if section in config.sections():
        return config[section]
    config[section] = {}
    return config[section]


def save_configs(config_file: str, config: ConfigParser) -> None:
    with open(config_file, 'w') as f:
        config.write(f)


def save_config_section(section: str, section_configs: dict | SectionProxy, config_file: str | None = None) -> None:
    if config_file is None:
        config_file = CONFIG_FILE
    config = load_configs(config_file)
    config[section] = section_configs
    save_configs(config_file, config)

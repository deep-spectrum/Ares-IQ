import distro
from pathlib import Path


def _fetch_centos_path() -> Path:
    if int(distro.major_version()) < 7:
        raise OSError(f"{distro.name(True)} is not supported")
    return Path("CentOS 7")


def _fetch_redhat_path() -> Path:
    if int(distro.major_version()) < 7:
        raise OSError(f"{distro.name(True)} is not supported")
    version = 7
    if int(distro.major_version()) > 7:
        version = 8
    return Path(f"Red Hat {version}")


def _fetch_ubuntu_path() -> Path:
    if int(distro.major_version()) < 14:
        raise OSError(f"{distro.name(True)} is not supported")
    version = "14.04"
    if int(distro.major_version()) >= 18:
        version = "18.04"
    return Path(f"Ubuntu {version}")


def lib_path():
    match distro.id():
        case "ubuntu": path = _fetch_ubuntu_path()
        case "centos": path = _fetch_centos_path()
        case "rhel": path = _fetch_redhat_path()
        case _: raise OSError(f"{distro.name(True)} is not supported")

    return "lib" / path

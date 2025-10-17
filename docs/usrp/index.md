# USRP Platform

The **Universal Software Radio Peripheral (USRP)** is a family of hardware devices 
developed by [Ettus Research](https://www.ettus.com/) for high-performance 
software-defined radio (SDR) applications. USRPs provide a flexible interface to 
RF hardware, enabling transmission and reception across a wide range of frequencies 
and bandwidths. These devices are commonly used in research, prototyping, and deployment 
of wireless systems.

---

## Hardware Overview

USRPs consist of:

- **Motherboard**: Hosts ADCs, DACs, FPGA, and host interface (e.g. USB, Ethernet, PCIe).
- **Daughterboards**: Modular front-end boards that determine supported frequency ranges and 
RF capabilities.
- **Clocking and Synchronization**: External and internal clocking options for precise timing.

---

## Device Families

| Family       | Description                                                    |
|--------------|----------------------------------------------------------------|
| **B-Series** | Cost-effective, compact devices for basic SDR applications.    |
| **X-Series** | High-performance devices with wide bandwidth and FPGA support. |
| **E-Series** | Embedded USRPs with onboard ARM processors for standalone use. |
| **N-Series** | Networked devices for distributed or remote deployments.       |

---

## Supported Devices

The following USRP models are supported by this software package:

| Model     | Family   | Frequency Range          | Notes                |
|-----------|----------|--------------------------|----------------------|
| USRP X310 | X-Series | Depends on daughterboard | Dual 10 GigE or PCIe |

!!! note "FPGA and Firmware Images"

    Device compatibility may depend on both the firmware version and the FPGA image version.
    Ensure the installed images are compatible with UHD 4.9.0.0.

---

## Usage Notes

- Each supported device must be properly configured with the correct frequency range, 
sample rate, and gain values.
- Ensure that the appropriate antenna is connected for the target frequency band.
- Clock synchronization (e.g., GPSDO or external 10 MHz reference) may be required 
for time-sensitive applications.

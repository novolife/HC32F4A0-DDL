# HC32F4A0 DDL

This repository is a mirror of the Device Driver Library (DDL) for
HC32F4A0 series MCUs, originally released by
Xiaohua Semiconductor (XHSC) (https://www.xhsc.com.cn/product/1220.html).

## License

### SDK Code
All SDK source code in the following directories is licensed under the
BSD 3-Clause License, as provided by the original vendor:

- drivers/
- midwares/
- projects/

See the LICENSE file for details.

### Third-Party Software

This repository includes the following third-party component:

- **SRecord**
  - Location: `utils/`
  - License: GPL-3.0
  - Source code: https://github.com/sierrafoxtrot/srecord

The SRecord binary is included unmodified.
The corresponding source code is publicly available at the URL above,
in compliance with GPL-3.0 requirements.

## Disclaimer

This is an **unofficial mirror**.
This repository is **not affiliated with, endorsed by, or maintained by**
Xiaohua Semiconductor.

All trademarks and product names remain the property of their respective owners.

## Toolchain and IDE Files

This repository includes project and debug configuration files for
various development environments, such as:

- IAR Embedded Workbench (EWARM)
- Keil MDK
- PyOCD / J-Link

These files only contain project structure and configuration settings.
They do **not** include any proprietary compiler, debugger, or IDE
software.

Users must obtain the corresponding tools from their respective vendors.

## Documentation

The `documents/hc32f4a0_ddl_Rev2.4.0.chm` file is the original documentation provided
with the HC32F4A0 DDL SDK by Xiaohua Semiconductor.

It is included here unmodified for reference purposes.

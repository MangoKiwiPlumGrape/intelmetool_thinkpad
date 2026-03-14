# intelmetool — CNL / ICL / CML Platform Support Fix

Fork of [coreboot's intelmetool](https://github.com/coreboot/coreboot/tree/main/util/intelmetool) with missing PCI device ID support for 8th–10th gen Intel platforms (Cannon Lake, Ice Lake, Comet Lake).

---

## The Problem

Upstream intelmetool works by scanning PCI to identify the platform before querying ME status. This scan happens in two stages:

1. **`pci_platform_scan()`** — scans for the PCH/chipset device ID to determine if ME is present and whether it can be disabled. If the device ID isn't in any of the known lists, it prints "ME is not present on your board or unknown" and **returns early** — ME is never queried.

2. **`pci_me_interface_scan()`** — scans for the MEI (HECI) interface device ID to actually communicate with ME. If the MEI device ID isn't in `PCI_DEV_HAS_SUPPORTED_ME`, intelmetool can't talk to ME at all.

Upstream `intelmetool.h` stops at Union Point (Kaby Lake refresh, 7th gen, 2017). Every platform from 8th gen onwards falls through both checks silently. The result on a ThinkPad X1 Carbon (8th/9th gen) or X13 Gen1 (10th gen) is:

```
ME is not present on your board or unknown
```

Even though ME is very much present and running.

---

## Root Cause

The `PCI_DEV_HAS_ME_DIFFICULT` macro — which covers platforms where ME is present but hard to remove — ends at `UNIONPOINT_X299` (Kaby Lake-X, 2017). The following platforms are completely absent:

- **Cannon Point** (CNL, 8th/9th gen LP, 300-series PCH)
- **Ice Lake** (ICL, 10th gen LP, 400-series PCH)
- **Comet Lake** (CML, 10th gen LP/H/S, 400-series PCH)

Because none of these PCH device IDs appear in `PCI_DEV_HAS_ME_DIFFICULT` (or any other macro), `pci_platform_scan()` exits before ME is ever interrogated.

Additionally, the MEI/HECI interface device IDs for these platforms are largely missing from `PCI_DEV_HAS_SUPPORTED_ME`, so even if the platform scan passed, intelmetool couldn't communicate with ME anyway.

---

## The Fix

All changes are in `intelmetool.h`. No other files were modified.

### PCH device IDs added to `PCI_DEV_HAS_ME_DIFFICULT`

| Platform | Generation | PCH IDs Added |
|----------|-----------|---------------|
| Cannon Point LP | 8th/9th gen | `0x9d83`, `0x9d84`, `0x9da8` |
| Ice Lake LP | 10th gen | `0x3482`, `0x3484`, `0x3488` |
| Comet Lake U/H/S | 10th gen | `0x02e8`, `0x06e0`, `0x06e8`, `0xa3b0`, `0xa3ba` |

### MEI/HECI device IDs added to `PCI_DEV_HAS_SUPPORTED_ME`

| Platform | Device ID | Description |
|----------|-----------|-------------|
| Cannon Point-LP | `0x9de8` | CNL MEI2 (upstream only had MEI1 `0x9de0`) |
| Ice Lake-LP | `0x34e0` | ICL MEI1 — completely absent from upstream |
| Ice Lake-LP | `0x34e8` | ICL MEI2 — completely absent from upstream |
| Comet Lake-U | `0x02e8` | CML-U MEI2 (upstream only had `0x02e0`) |
| Comet Lake-H | `0x06e0` | CML-H MEI1 — absent from upstream |
| Comet Lake-H | `0x06e8` | CML-H MEI2 — absent from upstream |
| Comet Lake-S | `0xa3b0` | CML-S MEI1 — absent from upstream |
| Comet Lake-S | `0xa3ba` | CML-S MEI2 — absent from upstream |

### Version bump

`INTELMETOOL_VERSION` bumped from `1.1` to `1.2`.

---

## Building

intelmetool has the same header dependencies as ifdtool. From inside the intelmetool directory:

```bash
mkdir -p commonlib/bsd
curl -o commonlib/helpers.h \
  https://raw.githubusercontent.com/coreboot/coreboot/main/src/commonlib/include/commonlib/helpers.h
curl -o commonlib/bsd/helpers.h \
  https://raw.githubusercontent.com/coreboot/coreboot/main/src/commonlib/bsd/include/commonlib/bsd/helpers.h
curl -o commonlib/bsd/compiler.h \
  https://raw.githubusercontent.com/coreboot/coreboot/main/src/commonlib/bsd/include/commonlib/bsd/compiler.h

make CFLAGS="-I."
```

You will also need `libpci` installed:

```bash
# Debian/Ubuntu
sudo apt install libpci-dev pciutils

# Arch
sudo pacman -S pciutils
```

---

## Usage

intelmetool requires root as it needs direct hardware access:

```bash
# Dump ME status and firmware info
sudo ./intelmetool -m

# Dump Boot Guard status
sudo ./intelmetool -b

# Both at once
sudo ./intelmetool -m -b
```

### Expected output on a machine with HAP set (ME disabled via firmware patch)

```
MEI found: [8086:02e0] Comet Point-LP MEI Controller

ME Status   : 0x...
ME Status 2 : 0x...

ME: Current Working State   : Normal
ME: Current Operation Mode  : Soft Temporary Disable
ME: Error Code              : No Error
...
```

### Before this fix — upstream output on 8th–10th gen

```
ME is not present on your board or unknown
```

---

## Verified Platforms

| Device | Platform | Result |
|--------|----------|--------|
| ThinkPad X1 Carbon 6th–9th gen | Cannon Lake LP (8th/9th gen) | ✅ ME status now reported correctly |
| ThinkPad X13 Gen1 | Comet Lake LP (10th gen) | ✅ ME status now reported correctly |

---

## Relationship to Other Forks

This fork is part of a set of three patched coreboot utilities for 8th–10th gen ThinkPad ME disable:

- **[me_cleaner_thinkpad](https://https://github.com/MangoKiwiPlumGrape/me_cleaner_thinkpad)** — fixes HAP bit offset for CNL/ICL/CML firmware images (hardware confirmed)
- **[ifdtool_thinkpad](https://https://github.com/MangoKiwiPlumGrape/ifdtool_thinkpad)** — fixes HAP bit read/write for CNL/ICL/CML in ifdtool
- **intelmetool** (this repo) — fixes platform detection so ME status can actually be queried on these machines

All three tools were broken for 8th–10th gen platforms due to the same underlying cause: the upstream coreboot utilities stopped being updated for new platforms after ~2017. and does not take pull requests on Github.

---

## License

GPL-2.0-only — same as upstream intelmetool.

Copyright (C) 2015 Damien Zammit (original)
Copyright (C) 2017 Philipp Deppenwiese (original)
Copyright (C) 2017 Patrick Rudolph (original)
Modifications copyright (C) 2026 MangoKiwiPlumGrape

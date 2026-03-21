# intelmetool-thinkpad — Platform Support Fix (8th–18th gen Intel)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/MangoKiwiPlumGrape/intelmetool_thinkpad)

Fork of [coreboot's intelmetool](https://github.com/coreboot/coreboot/tree/main/util/intelmetool) with missing PCI device ID support for 8th gen through Panther Lake Intel platforms.

---

## The Problem

Upstream intelmetool works by scanning PCI to identify the platform before querying ME status. This scan happens in two stages:

1. **`pci_platform_scan()`** — scans for the PCH/chipset device ID to determine if ME is present. If the device ID isn't in any known list, it prints `"ME is not present on your board or unknown"` and returns early — ME is never queried.

2. **`pci_me_interface_scan()`** — scans for the MEI (HECI) interface device ID to communicate with ME. If the MEI device ID isn't in `PCI_DEV_HAS_SUPPORTED_ME`, intelmetool can't talk to ME at all.

Upstream `intelmetool.h` stops at Union Point (Kaby Lake refresh, 7th gen, 2017). Every platform from 8th gen onwards falls through both checks silently:

```
ME is not present on your board or unknown
```

Even though ME is very much present and running.

---

## The Fix

All changes are in `intelmetool.h`. Version bumped from `1.1` → `1.3`.

### MEI/HECI device IDs added to `PCI_DEV_HAS_SUPPORTED_ME`

| Platform | Generation | Device IDs Added | Source |
|----------|-----------|------------------|--------|
| Cannon Point-LP | 8th/9th gen | `0x9de8` (MEI2) | kernel mei/hw-me-regs.h |
| Ice Lake-LP | 10th gen | `0x34e0`, `0x34e8` (MEI1/2) | kernel — completely absent upstream |
| Comet Lake-U | 10th gen | `0x02e8` (MEI2) | kernel |
| Comet Lake-H | 10th gen | `0x06e0`, `0x06e8` (MEI1/2) | kernel — absent upstream |
| Comet Lake-S | 10th gen | `0xa3b0`, `0xa3ba` (MEI1/2) | kernel — absent upstream |
| Tiger Lake-LP | 11th gen | `0xa0e0`, `0xa0e8` (MEI1/2) | kernel |
| Tiger Lake-H | 11th gen | `0x43e0`, `0x43e8` (MEI1/2) | kernel |
| Elkhart Lake | Atom x6000 | `0x4b28` (MEI1) | kernel |
| Alder Lake-S | 12th gen | `0x7ae8`, `0x7ae9` (MEI1/2) | Doc 648364 — corrected from wrong `0x7aea` |
| Alder Lake-P | 12th gen | `0x51e0`, `0x51e8` (MEI1/2) | Doc 648364 |
| Alder Lake-N | 12th gen | `0x54e0`, `0x54e8` (MEI1/2) | kernel |
| Raptor Lake-S | 13th gen | `0x7a68`, `0x7a69` (MEI1/2) | Doc 743835 — corrected from wrong `0x7a60` |
| Meteor Lake-P | 14th gen | `0x7e70`, `0x7e71`, `0x7e74` (MEI1/2/3) | Doc 792044 |
| Arrow Lake H/U | 15th gen | `0x7770`–`0x7775` (MEI1–4), `0x7758`–`0x775a` (H-tile) | Doc 842704 |
| Arrow Lake-S | 15th gen | `0xae70`, `0xae71` (MEI1/2) | coreboot |
| Lunar Lake | 16th gen | `0xa870` (MEI1) | coreboot |
| Panther Lake-U | Series 3 | `0xe362`, `0xe363`, `0xe364` (MEI1/2/3) | Doc 872188 |
| Panther Lake-H | Series 3 | `0xe462`, `0xe463`, `0xe464` (MEI1/2/3) | Doc 872188 |

### ID corrections

Two upstream IDs were simply wrong — the device IDs existed but pointed to different functions:

| Platform | Wrong ID | Correct ID | Issue |
|----------|----------|------------|-------|
| ADL-S | `0x7aea` | `0x7ae8` / `0x7ae9` | `0x7aea` is IDE-R, not HECI1. Doc 648364. |
| RPL-S | `0x7a60` | `0x7a68` / `0x7a69` | `0x7a60` is wrong SKU variant. Doc 743835. |

### PCH eSPI device IDs added to `PCI_DEV_HAS_ME_DIFFICULT`

Tiger Lake, Alder Lake, Raptor Lake, and Meteor Lake PCH eSPI controller IDs added so `pci_platform_scan()` no longer exits early on these platforms.

---

## Building

No coreboot tree required. All dependencies are vendored in this repo.

**Prerequisites** (one-time):
```bash
# Debian/Ubuntu
sudo apt install libpci-dev zlib1g-dev

# Fedora/RHEL
sudo dnf install pciutils-devel zlib-devel
```

**Build:**
```bash
git clone https://github.com/MangoKiwiPlumGrape/intelmetool_thinkpad
cd intelmetool_thinkpad
make
```

---

## Usage

intelmetool requires root as it needs direct hardware access:

```bash
# ME status and firmware info
sudo ./intelmetool -m

# Boot Guard status
sudo ./intelmetool -b

# Both
sudo ./intelmetool -m -b
```

### Expected output with HAP set (ME disabled)

```
MEI found: [8086:02e0] Comet Point-LP MEI Controller

ME Status   : 0x...
ME Status 2 : 0x...

ME: Current Working State   : Normal
ME: Current Operation Mode  : Soft Temporary Disable
ME: Error Code              : No Error
```

### Before this fix — upstream output on 8th–15th gen

```
ME is not present on your board or unknown
```

---

## Verified Platforms

| Device | Platform | Result |
|--------|----------|--------|
| ThinkPad X1 Carbon 6th–9th gen | Cannon Lake LP (8th/9th gen) | ✅ ME status reported correctly |
| ThinkPad X13  | Comet Lake LP (10th gen) | ✅ ME status reported correctly |

---

## Platform Notes

**MTL (Meteor Lake, 14th gen):** Detection works. HAP bit write path in ifdtool/me_cleaner is unconfirmed — MTL has no discrete PCH and the descriptor layout changed completely. Do not flash an MTL image without verifying the HAP offset from a known-good dump first.

**ARL/LNL/PTL (15th–Series 3):** Detection IDs added from Intel datasheets (Doc 842704, Doc 872188) and coreboot. HAP bit write path unknown for all three — no register maps analysed yet.

---

## Relationship to Other Forks

This is one of three patched coreboot utilities for Intel ME disable:

- **[me_cleaner_thinkpad](https://github.com/MangoKiwiPlumGrape/me_cleaner_thinkpad)** — fixes HAP bit offset for CNL/ICL/CML/TGL/ADL/RPL firmware images
- **[ifdtool_thinkpad](https://github.com/MangoKiwiPlumGrape/ifdtool_thinkpad)** — fixes HAP bit read/write for CNL through RPL in ifdtool
- **intelmetool_thinkpad** (this repo) — fixes platform detection so ME status can be queried on 8th gen through Panther Lake

All three were broken for post-7th-gen platforms because upstream coreboot utilities stopped being updated for new platforms after ~2017.

---

## License

GPL-2.0-only — same as upstream intelmetool.

Copyright (C) 2015 Damien Zammit (original)  
Copyright (C) 2017 Philipp Deppenwiese (original)  
Copyright (C) 2017 Patrick Rudolph (original)  
Modifications copyright (C) 2026 MangoKiwiPlumGrape

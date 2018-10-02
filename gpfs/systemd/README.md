# Setup GPFS/MMFS/Scale Systemd unit files

## Why?
GPFS comes with a systemd unit file (/usr/lpp/mmfs/lib/systemd/gpfs.service) that comes with the RPMs by default. Older version of gpfs <5 come with a SysV Init file. Both of these have been found to be fragile with larger machines and don't allow control of each of the startup steps.

# Installation
1. Disable the builtin gpfs init 
```bash
systemctl disable gpfs
```
2. Copy new unit files into place
```bash

```


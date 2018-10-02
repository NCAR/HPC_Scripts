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
cp -vf ncar-gpfs-startup.service ncar-gpfs-mount.service ncar-gpfsmond.service /etc/systemd/system/
cp -vf stop-gpfs-mounts.ncar stop-gpfs.ncar check_mmgetstate.ncar check_gpfsmond.ncar /usr/sbin/ 
chmod 0755 /usr/sbin/check_gpfsmond.ncar /usr/sbin/check_mmgetstate.ncar /usr/sbin/stop-gpfs.ncar /usr/sbin/check_mmgetstate.ncar /usr/sbin/stop-gpfs.ncar /usr/sbin/stop-gpfs-mounts.ncar 
```
3. Test new unit files
```bash     
  systemctl sttart ncar-gpfs.target
```
4. Activate new unit files
```bash     
  systemctl enable ncar-gpfs.target
```




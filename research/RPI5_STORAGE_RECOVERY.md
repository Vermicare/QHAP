# Raspberry Pi 5 storage incident and recovery runbook — 2026-10-13

**Status:** Recovery in progress. Do not infer hardware failure or successful repair from the current evidence. This runbook is operational documentation, not a record of a completed fix.

## Purpose and scope

The Raspberry Pi 5 (8 GB) is intended to supervise QHAP experiments, ingest logs, and run isolated test services; it must **not** become a single point of failure for the autonomous ESP32 reference miner. Preserve validated mining firmware and experiment records independently of the Pi.

**Never publish:** SSH private keys, passwords, GPG passphrases, Wi-Fi credentials, pool secrets, wallet keys, full home-directory archives, or raw sensitive logs. The encrypted backup belongs in private cloud storage, **not** in Git.

## Incident evidence (Ubuntu live session on separate laptop, 2026-10-12/13)

- Raspberry Pi OS microSD was detected in a USB card reader as a ~117.2 GiB removable device (at the time: `/dev/sdc`), with ~512 MiB FAT `bootfs` (then `/dev/sdc1`) and ~116.7 GiB EXT4 `rootfs` (then `/dev/sdc2`).
- The laptop also had a ~15.2 GiB live Ubuntu USB installer (then `/dev/sdb`) and a ~476.9 GiB Windows NVMe SSD (then `/dev/nvme0n1`) containing BitLocker volumes. **Do not overwrite or repair either. Device letters can change on reconnect.**
- Repeated EXT4 checks reported very extensive `Block bitmap differences`, group bitmap checksum mismatches, and free-block accounting inconsistencies. Earlier `e2fsck -f -n` exited **4** (errors left uncorrected); `e2fsck -f -p` and prior interactive attempts did not establish a verified clean result. A later interactive `e2fsck -f` was **canceled at `Fix<y>?`** because the scope of proposed changes was large.
- `dumpe2fs -h` reported EXT4 `metadata_csum`, checksum `crc32c`, and filesystem state `clean`. The clean flag **does not override failed structural verification**. An earlier warning said the superblock last-write time was in the future; later timestamps were plausible. Root cause is **unknown**.
- Filtered `dmesg` showed ordinary USB mass-storage enumeration, an EXT4 `orphan cleanup on readonly fs` message, a `ro without journal` mount, and subsequent unmount; no explicit I/O errors or USB resets in that filtered output. This **does not prove the SD card or reader is healthy**.
- Read-only recovery mount used `mount -t ext4 -o ro,noload`; `/home/qhap/qhap` contained `.git`, `PROJECT_MEMORY.md`, source, tests, tools, and research. `du -sh` reported ~576 KiB for the repo and ~644 KiB for the full `/home/qhap` directory. File readability is **not** a complete integrity audit.
- An AES-256 GPG symmetric archive of `/home/qhap` was created in live Ubuntu: `qhap-home-backup-2026-10-13.tar.gz.gpg`, ~192 KiB. A local GPG-decrypt + `tar -tzf` check printed `BACKUP VERIFIED`. A browser screenshot showed that filename in **My Drive**, owned by the user. The connected Drive API did not independently locate it; **cloud download/hash round-trip verification is still pending**.
- The archive covers **only `/home/qhap`**, including hidden files and the repository; it is **not** a whole-card or whole-OS backup. Other paths such as `/etc`, `/var`, services, installed packages, and boot configuration might contain unique state. The Pi's complete installed OS and SSH service state were not validated before this incident.
- At last observed check, both SD partitions were **unmounted**. No subsequent repair or successful Pi boot has been confirmed.

## Recovery decision gates

1. **Preserve evidence / identify the device.** Do not assume `/dev/sdc` persists across replug or reboot. Use `lsblk -e7 -o NAME,SIZE,MODEL,FSTYPE,LABEL,MOUNTPOINTS`; check serial/by-id where available. Distinguish the live USB and BitLocker NVMe SSD. Never run filesystem repairs on mounted partitions.
2. **Secure a second copy.** Keep the encrypted archive in private cloud storage and retain its passphrase separately. Ideally download the cloud copy to a safe destination and compare SHA-256 with the original live-session archive. Do not publish archive or passphrase. Do not restart the live session until the remote backup is independently confirmed. Inspect `/etc`, `/boot/firmware`, `/var/lib`, crontabs/systemd services, and any other unique Pi configuration **read-only** and back up as necessary before riskier steps.
3. **Choose preservation level deliberately.** A file-level backup of ~644 KiB does not protect the entire installed OS. If a complete recovery point is required, image the whole microSD to a **different** device with sufficient space (about 128 GB uncompressed; use `ddrescue` if read errors occur). A full image is preferable before risky repair if capacity is available. Never image the card onto itself, onto the Ubuntu installer USB, or onto an unverified Windows partition.
4. **Check reliability without destructive writes.** Review complete kernel logs for storage errors, resets and disconnects; consider a different known-good USB card reader. If preserving an image, capture it before repeated repair attempts. Recurrent corruption, read failures, or device instability warrants replacing the microSD rather than trusting it for continuous experiments.
5. **Repair only with informed consent.** On an **unmounted, correctly re-identified** EXT4 root partition, `e2fsck -f` can propose changes to block/inode bitmaps and directories. `-p` only fixes safe cases automatically; exit code 4 means errors remain uncorrected. Do not run `-y` reflexively or approve massive bitmap changes without understanding the preservation tradeoff. The user must explicitly accept possible loss of damaged/unbacked-up OS files before an interactive repair.
6. **Verify after repair.** Re-run an offline structural check and interpret exit codes; do not rely on the superblock's `clean` flag. Review `lost+found`, inspect restored files, check bootfs separately if indicated, then test boot, SSH and QHAP lab behavior. If corruption returns or repair cannot converge, prefer a clean Raspberry Pi OS reflash **only after user approval and data preservation**.
7. **Rebuild safely if needed.** Use Raspberry Pi Imager, verify target by capacity/model, configure SSH and trusted LAN access, restore only needed data from the encrypted archive, install dependencies reproducibly, and verify the repository state and QHAP experiment scripts. Do not deploy experimental firmware to the reference ESP32 during Pi recovery.

## Prevention and operational policy

- Use a Raspberry Pi 5-compatible, stable power supply (official 27 W USB-C PSU recommended where appropriate) and avoid brownouts or unplugging while the filesystem is active.
- Use `sudo shutdown -h now` and wait for shutdown before removing power or microSD; do not hot-remove the card.
- Keep source in Git, push verified commits, and back up **uncommitted work**, local configs, run data, SQLite databases and logs separately. GitHub alone is not an OS backup.
- Maintain encrypted, off-device, **restorable** backups; verify restore/decryption periodically. For long-running logging, use bounded log retention/rotation, write-rate controls, and consider higher-endurance storage or an SSD if reliability measurements justify it.
- Record Pi OS image version, setup steps, installed packages, service definitions, storage health, power supply and recovery procedure without exposing secrets.
- Add a Pi health gate before any unattended evolution experiments: clean offline filesystem validation after recovery, stable boots, monitored power, bounded storage use, and backup/restore drill.
- Keep the production ESP32 autonomous and isolated from Pi recovery or experiment failure.

## Current next safe action

The last command was a filtered `dmesg` inspection; it did **not** establish hardware failure. Before choosing repair vs reimage, confirm whether the user needs to preserve unique system-level Pi configuration outside `/home/qhap` and whether a complete disk image destination is available. Continue **one command per physical step**, checking screenshots. No automatic format, `fsck -y`, or write test.

## References

- `man e2fsck` (exit codes, interactive repair), `man dumpe2fs`, `man mount`, `man lsblk`
- GNU ddrescue manual: https://www.gnu.org/software/ddrescue/manual/ddrescue_manual.html
- Raspberry Pi power supplies: https://www.raspberrypi.com/documentation/computers/power-supplies.html
- [QHAP project memory](../PROJECT_MEMORY.md)
- [Pre-ASIC lab plan](PRE_ASIC_LAB_PLAN.md)

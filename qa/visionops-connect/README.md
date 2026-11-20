# VisionOps Connect · v0.13.4 Windows camera onboarding

## Product goal
Install → authenticate → click **Kameraları Bul** → pick a discovered camera/NVR → provide a
camera-authorized account when needed → select a video profile/channel → connect and verify a
**real decoded live frame**. Credential prompting is mandatory where the device requires it.
"Internet connected" never means discoverable: ONVIF is a local network discovery standard,
RTSP is a stream protocol, and cloud/P2P-only products may not expose either.

## Safety contract
- Network enumeration runs on the local Windows PC after an authenticated workspace request
  and existing CSRF guard. It is never cloud-side or continuously scanning.
- Default scan: UDP 239.255.255.250:3702 WS-Discovery on detected private IPv4 interfaces,
  matched one-time MessageID, same-IP host validation, XML size/entity rejection, 64 entries max.
- **Opt-in only** expanded scan: no credential guessing, up to two locally derived /24
  networks, TCP/RTSP port 554 only, at most 508 addresses and 32 findings. Enterprise admins
  should opt out where network scanning is prohibited.
- Untrusted ONVIF services cannot redirect or point clients at a different IP, WAN, loopback
  or metadata address. Only the discovered local IPv4 host may supply HTTP(S) and RTSP URLs.
- Supports ONVIF Media1 and Media2 profiles (Profile T), preserves manually entered RTSP
  and model-specific NVR paths, HTTPS/MJPEG and USB/OBS compatibility. No guarantee for
  proprietary P2P/cloud-only cameras or every ONVIF firmware implementation.
- Reachable TCP port, positive RTSP OPTIONS and successful ONVIF GetProfiles are **not** a
  decoded frame. UI checks existing camera state and waits for a real live frame; when it
  cannot confirm one, reports failure/unknown instead of success.
- Camera passwords are requested explicitly and transiently in the browser, wiped when
  finished, never placed in a URL in the frontend, log, cloud, scan response or localStorage.
  If the user chooses Remember, the existing Windows DPAPI account-scoped camera vault
  persists the connection information. Device-specific RTSP authentication still happens
  on the customer's LAN; network segregation and device security matter.
- Never change camera password, ONVIF switch, network firewall, or device configuration
  without administrator permission. No default-password dictionary or WAN scanning.
- Native network decoding is isolated in a child process with bounded frames, timeouts and
  restart, so blocking drivers cannot freeze the entire UI. USB/OBS remains native.
- Existing anonymous privacy/blur logic stays unchanged. No new employee unblur exemption.

## Physical compatibility gate (NOT yet satisfied)
Required before marking commercially ready:
- [ ] Mercusys MC200 ONVIF discovery and 30+ reconnect cycles; 1080p/SD profile selection
- [ ] EZVIZ C6N: RTSP with actual account permissions and any unavailable ONVIF reported clearly
- [ ] Hikvision or HiLook IP camera or NVR: Media1 and channel selection
- [ ] Dahua IP camera or NVR: Media1/Media2 and multi-channel RTSP
- [ ] 4 concurrent real camera streams with independent failure/recovery and CPU/GPU reports
- [ ] Two Windows network adapters, VPN, firewall, VLAN split, and Wi-Fi isolation
- [ ] Wrong password, disabled ONVIF, wrong device clock, disconnected switch, changed DHCP IP
- [ ] H.264/H.265 main versus substream and unsupported codec error guidance
- [ ] Seven-day test with outage/restart, no credential or raw image leakage to cloud/logs
- [ ] Windows interactive installation, login, reconnect across app restart and saved vault restore
- [ ] Per-device operator consent, access controls, and compatibility documentation

## Reproduce offline CI
Run `python qa/visionops-connect/build.py` followed by
`python qa/visionops-connect/test_connect.py`, and on Windows after installing
`numpy` and `opencv-python`, `python qa/visionops-connect/test_decoder.py`.
The build first reproduces the v0.13.2 + v0.13.3 tested baseline with hash checks.
Its output is `connect-qa-build/`. No production release or automatic update occurs here.

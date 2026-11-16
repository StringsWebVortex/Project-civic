# Project Civic — secure cloud firmware update design

## Goal

Allow the ESP32 in the Civic to retrieve signed firmware releases over an outbound HTTPS connection. The workstation publishes a release; the vehicle does not need an inbound port, public IP or port-forwarding rule. This supports mobile-hotspot and home-router connectivity, including double NAT.

## Proposed flow

1. Workstation builds ESP32 and STM32 images and generates SHA-256 digests.
2. Release tooling signs a manifest with an offline-held signing key.
3. Private object storage holds firmware binaries; an authenticated API/Worker returns a device-authorized manifest and short-lived download URLs.
4. ESP32 connects outbound over TLS, authenticates using a per-device credential, downloads the manifest and verifies its signature.
5. ESP32 validates device family, hardware revision, image size, version/rollback policy and SHA-256.
6. For STM32, ESP32 verifies the entire image before entering the STM32 ROM bootloader, then programs over USART3 and performs read-back verification.
7. For ESP32, stream to the inactive OTA partition, calculate the digest during transfer, verify the signature before finalizing/booting the new slot, and retain rollback/recovery behaviour.
8. ESP32 reports result/version to the service when network connectivity is available.

## Security requirements

- HTTPS with certificate validation; never use insecure TLS mode.
- Unique per-device credentials stored in NVS, not a shared fleet-wide token.
- Signed manifest (recommended ECDSA P-256 or Ed25519) with a public verification key embedded in firmware.
- Manifest signature covers target, version, hardware compatibility, image size, SHA-256 and artifact identifier/URL.
- SHA-256 is for integrity; the digital signature establishes authenticity.
- Reject unsigned images, mismatched hardware, oversized images, downgrade attempts and malformed manifests.
- Short-lived download URLs or an authenticated streaming endpoint; no inbound connection to the vehicle.
- Keep the ROM bootloader inaccessible from the CAN bus; only ESP32-controlled GPIO/UART can enter it.
- Preserve a recovery path if a download or STM32 flash operation fails.

## Existing project implementation / gap

The project already has:
- ESP32 local HTTP upload endpoint for ESP32 OTA (/update).
- ESP32 local HTTP upload endpoint for STM32 ROM-bootloader programming (/update-stm32).
- STM32 ROM bootloader target uses USART3 PB10/PB11 (same GPIO16/17 wires as the 2 Mbaud application UART), at 57,600 baud 8E1, with erase/write/read-back verification.
- ESP32 controls BOOT0 (GPIO26) and NRST (GPIO25, open-drain); PB2/BOOT1 is not wired, so MB997 SB19 must be fitted ON to hold it low.

However, the current local HTTP OTA path is not ready to expose to the internet:
- requireLogin() currently returns true unconditionally.
- Upload handlers set their OTA-authenticated flags true without a credential check.
- The STA interface joins a LAN, so update routes may be reachable by other clients on that network.
- There is no cloud manifest, per-device cloud authentication or signed firmware verification yet.

**Do not port-forward the existing /update or /update-stm32 routes, and do not expose them to an untrusted network.**

## Implementation sequence

1. Introduce a device identity and unique provisioned API credential in NVS.
2. Add HTTPS manifest retrieval with certificate validation.
3. Add signature verification using a public key compiled into firmware.
4. Add signed SHA-256/version/target validation.
5. Add ESP32 OTA with verification and rollback.
6. Add STM32 download-to-buffer, signature/hash verification, then ROM bootloader programming.
7. Add progress/result reporting to dashboard and cloud.
8. Test failure cases on bench: bad signature, truncated download, wrong target, downgrade, power interruption, STM32 NACK and read-back mismatch.

Cloud account credentials, production signing keys and deployment URLs are deliberately not hard-coded into source. They must be provisioned after the cloud service is selected.

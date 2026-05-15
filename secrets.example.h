#pragma once

// ---------------------------------------------------------------------------
// CyberDash · secrets template
//
// 1. Copy this file to `secrets.h` (which is gitignored).
// 2. Fill in the real values below.
// 3. Never commit `secrets.h`.
//
// CI builds use this template directly with placeholder values so that
// published release binaries never contain real credentials.
// ---------------------------------------------------------------------------

#define WIFI_SSID     "your-ssid-here"
#define WIFI_PASS     "your-wifi-password"
#define HOSTNAME      "cyberdash"
#define OTA_PASSWORD  "change-me-after-flash"

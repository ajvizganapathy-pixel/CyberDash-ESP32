#pragma once

// ---------------------------------------------------------------------------
// CyberDash · firmware version
//
// Update CYBERDASH_VERSION on every release. Tag the commit `vX.Y.Z` and
// the release workflow will pick it up automatically.
//
// CYBERDASH_BUILD_TAG / CYBERDASH_BUILD_SHA are injected by CI via
// build_flags (see .github/workflows/release.yml). When building locally
// they fall back to the values below.
// ---------------------------------------------------------------------------

#define CYBERDASH_VERSION       "1.0.0"

#ifndef CYBERDASH_BUILD_TAG
  #define CYBERDASH_BUILD_TAG   "dev"
#endif

#ifndef CYBERDASH_BUILD_SHA
  #define CYBERDASH_BUILD_SHA   "local"
#endif

#define CYBERDASH_USER_AGENT    "CyberDash/" CYBERDASH_VERSION " (" CYBERDASH_BUILD_TAG ")"

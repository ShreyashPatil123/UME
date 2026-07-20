#pragma once

/// @file ume.h
/// @brief Master include header for the Unified Memory Engine.
///
/// Including this header provides access to the complete UME Core API.
/// Prefer including only the specific headers you need for faster compilation.
///
/// @code
///     #include <ume/ume.h>
///     
///     int main() {
///         auto config = ume::Config::default_test_config();
///         // ... engine lifecycle
///     }
/// @endcode

// Version
#include "ume/version.h"

// Foundation
#include "ume/types.h"
#include "ume/status.h"
#include "ume/config.h"

// Platform
#include "ume/platform/platform.h"
#include "ume/platform/clock.h"

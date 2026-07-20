/// @file test_hazard_pointers.cpp
/// @brief Unit tests for Hazard Pointers memory reclamation.

#include "ume/concurrency/hazard_pointers.h"

#include <gtest/gtest.h>

namespace ume::concurrency {
namespace {

TEST(HazardPointersTest, AcquireAndReleaseSlot) {
    HazardPointerDomain domain;

    size_t slot1 = domain.acquire_slot();
    EXPECT_LT(slot1, kMaxHazardSlots);

    size_t slot2 = domain.acquire_slot();
    EXPECT_LT(slot2, kMaxHazardSlots);
    EXPECT_NE(slot1, slot2);

    domain.release_slot(slot1);
    domain.release_slot(slot2);
}

TEST(HazardPointersTest, ProtectPreventsReclamation) {
    HazardPointerDomain domain;
    int target_value = 42;

    size_t slot = domain.acquire_slot();
    domain.protect(slot, &target_value);

    bool deleted = false;
    domain.retire(&target_value, [&deleted](const void*) { deleted = true; });

    // Reclaim while protected
    size_t reclaimed = domain.reclaim();
    EXPECT_EQ(reclaimed, 0U);
    EXPECT_FALSE(deleted);

    // Unprotect and reclaim
    domain.clear(slot);
    reclaimed = domain.reclaim();
    EXPECT_EQ(reclaimed, 1U);
    EXPECT_TRUE(deleted);

    domain.release_slot(slot);
}

} // namespace
} // namespace ume::concurrency

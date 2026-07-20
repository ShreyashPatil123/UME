/// @file test_types.cpp
/// @brief Unit tests for UME core types.

#include "ume/types.h"

#include <gtest/gtest.h>
#include <unordered_set>

namespace ume {
namespace {

TEST(TypesTest, ObjectId_Sentinel) {
    EXPECT_EQ(to_raw(ObjectId::kNull), 0ULL);
}

TEST(TypesTest, ObjectId_Comparison) {
    ObjectId id1 = static_cast<ObjectId>(100);
    ObjectId id2 = static_cast<ObjectId>(200);
    EXPECT_NE(id1, id2);
    EXPECT_EQ(to_raw(id1), 100ULL);
}

TEST(TypesTest, ObjectId_HashInUnorderedSet) {
    std::unordered_set<ObjectId> set;
    set.insert(static_cast<ObjectId>(1));
    set.insert(static_cast<ObjectId>(2));
    set.insert(static_cast<ObjectId>(1));

    EXPECT_EQ(set.size(), 2U);
    EXPECT_TRUE(set.contains(static_cast<ObjectId>(1)));
    EXPECT_TRUE(set.contains(static_cast<ObjectId>(2)));
    EXPECT_FALSE(set.contains(static_cast<ObjectId>(3)));
}

TEST(TypesTest, TierId_Sentinel) {
    EXPECT_EQ(to_raw(TierId::kInvalid), 0xFFFFU);
}

TEST(TypesTest, DeviceId_Sentinel) {
    EXPECT_EQ(to_raw(DeviceId::kInvalid), 0xFFFFU);
}

TEST(TypesTest, EdgeId_Sentinel) {
    EXPECT_EQ(to_raw(EdgeId::kInvalid), 0xFFFFFFFFU);
}

TEST(TypesTest, Timestamp_Values) {
    EXPECT_EQ(to_raw(Timestamp::kZero), 0ULL);
    EXPECT_EQ(to_raw(Timestamp::kMax), std::numeric_limits<uint64_t>::max());
}

TEST(TypesTest, InternedString_Sentinel) {
    EXPECT_EQ(to_raw(InternedString::kNull), 0U);
}

TEST(TypesTest, TierClass_Values) {
    EXPECT_NE(TierClass::kVram, TierClass::kRam);
    EXPECT_NE(TierClass::kPinnedRam, TierClass::kNvme);
    EXPECT_NE(TierClass::kCompressedRam, TierClass::kRemote);
    EXPECT_NE(TierClass::kUnified, TierClass::kCxl);
}

TEST(TypesTest, AccessType_Values) {
    EXPECT_NE(AccessType::kRead, AccessType::kWrite);
    EXPECT_NE(AccessType::kWrite, AccessType::kReadWrite);
}

TEST(TypesTest, CompressionCodec_Values) {
    EXPECT_NE(CompressionCodec::kNone, CompressionCodec::kLz4);
    EXPECT_NE(CompressionCodec::kZstd, CompressionCodec::kSnappy);
}

TEST(TypesTest, ToRaw_RoundTrip) {
    ObjectId obj = static_cast<ObjectId>(42ULL);
    EXPECT_EQ(to_raw(obj), 42ULL);

    TierId tier = static_cast<TierId>(7U);
    EXPECT_EQ(to_raw(tier), 7U);

    DeviceId dev = static_cast<DeviceId>(3U);
    EXPECT_EQ(to_raw(dev), 3U);

    EdgeId edge = static_cast<EdgeId>(100U);
    EXPECT_EQ(to_raw(edge), 100U);

    Timestamp ts = static_cast<Timestamp>(123456789ULL);
    EXPECT_EQ(to_raw(ts), 123456789ULL);

    InternedString str = static_cast<InternedString>(12U);
    EXPECT_EQ(to_raw(str), 12U);
}

} // namespace
} // namespace ume

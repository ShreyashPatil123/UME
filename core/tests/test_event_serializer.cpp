/// @file test_event_serializer.cpp
/// @brief Google Test suite for EventSerializer (Milestone M2 Task T002).

#include "ume/event/event_serializer.h"

#include <gtest/gtest.h>
#include <vector>

namespace ume::event {
namespace {

TEST(EventSerializerTest, ComputeCrc32NonZero) {
    const uint8_t data[] = {'U', 'M', 'E', '_', 'E', 'V', 'E', 'N', 'T'};
    uint32_t crc1 = compute_crc32(data, sizeof(data));
    EXPECT_NE(crc1, 0U);

    // Identical data produces identical CRC
    uint32_t crc2 = compute_crc32(data, sizeof(data));
    EXPECT_EQ(crc1, crc2);

    // Modified data produces different CRC
    const uint8_t mod_data[] = {'U', 'M', 'E', '_', 'E', 'V', 'E', 'N', 'X'};
    uint32_t crc3 = compute_crc32(mod_data, sizeof(mod_data));
    EXPECT_NE(crc1, crc3);
}

TEST(EventSerializerTest, SerializeAndDeserializeObjectEvent) {
    Event orig =
        Event::create_object_event(EventId{101}, Timestamp{123456789ULL}, EventType::kObjectCreated,
                                   ObjectId{42}, TierId{1}, 2097152);

    std::vector<uint8_t> buffer(sizeof(Event));
    auto ser_res = EventSerializer::serialize(orig, buffer.data(), buffer.size());
    ASSERT_TRUE(ser_res.ok());
    EXPECT_EQ(ser_res.value(), sizeof(Event));

    auto deser_res = EventSerializer::deserialize(buffer.data(), buffer.size());
    ASSERT_TRUE(deser_res.ok());

    Event deser = deser_res.value();
    EXPECT_EQ(deser.header.id, orig.header.id);
    EXPECT_EQ(deser.header.timestamp, orig.header.timestamp);
    EXPECT_EQ(deser.header.type, orig.header.type);
    EXPECT_EQ(deser.header.object_id, orig.header.object_id);
    EXPECT_EQ(deser.payload.object.size_bytes, orig.payload.object.size_bytes);
}

TEST(EventSerializerTest, ValidateBufferBoundary) {
    std::vector<uint8_t> small_buffer(32, 0);
    auto val_res = EventSerializer::validate(small_buffer.data(), small_buffer.size());
    EXPECT_FALSE(val_res.ok());
    EXPECT_EQ(val_res.status(), Status::kSerializationError);

    auto deser_res = EventSerializer::deserialize(small_buffer.data(), small_buffer.size());
    EXPECT_FALSE(deser_res.ok());
}

} // namespace
} // namespace ume::event

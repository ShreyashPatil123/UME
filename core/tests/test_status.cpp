/// @file test_status.cpp
/// @brief Unit tests for UME Status and Result handling.

#include "ume/status.h"

#include <gtest/gtest.h>
#include <string>

namespace ume {
namespace {

TEST(StatusTest, IsOk) {
    EXPECT_TRUE(is_ok(Status::kOk));
    EXPECT_FALSE(is_ok(Status::kInvalidArgument));
    EXPECT_FALSE(is_ok(Status::kNotFound));
    EXPECT_FALSE(is_ok(Status::kInvalidConfig));
    EXPECT_FALSE(is_ok(Status::kFileNotFound));
}

TEST(StatusTest, IsError) {
    EXPECT_FALSE(is_error(Status::kOk));
    EXPECT_TRUE(is_error(Status::kInvalidArgument));
    EXPECT_TRUE(is_error(Status::kNotFound));
    EXPECT_TRUE(is_error(Status::kInvalidConfig));
    EXPECT_TRUE(is_error(Status::kFileNotFound));
}

TEST(StatusTest, StatusName) {
    EXPECT_EQ(status_name(Status::kOk), "kOk");
    EXPECT_EQ(status_name(Status::kInvalidArgument), "kInvalidArgument");
    EXPECT_EQ(status_name(Status::kNotFound), "kNotFound");
    EXPECT_EQ(status_name(Status::kInvalidConfig), "kInvalidConfig");
    EXPECT_EQ(status_name(Status::kFileNotFound), "kFileNotFound");
}

TEST(ResultTest, IntValue) {
    Result<int> res(42);
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(res.value(), 42);
}

TEST(ResultTest, IntError) {
    Result<int> res(Status::kNotFound);
    EXPECT_FALSE(res.ok());
    EXPECT_EQ(res.status(), Status::kNotFound);
}

TEST(ResultTest, ValueOr) {
    Result<int> res_ok(42);
    EXPECT_EQ(res_ok.value_or(0), 42);

    Result<int> res_err(Status::kNotFound);
    EXPECT_EQ(res_err.value_or(0), 0);
}

TEST(ResultTest, VoidOk) {
    Result<void> res;
    EXPECT_TRUE(res.ok());
}

TEST(ResultTest, VoidError) {
    Result<void> res(Status::kInvalidArgument);
    EXPECT_FALSE(res.ok());
    EXPECT_EQ(res.status(), Status::kInvalidArgument);
}

TEST(ResultTest, PointerValue) {
    const char* str = "hello";
    Result<const char*> res(str);
    EXPECT_TRUE(res.ok());
    EXPECT_STREQ(res.value(), "hello");
}

// Helper for testing UME_RETURN_IF_ERROR
Status helper_return_if_error(Status s) {
    UME_RETURN_IF_ERROR(s);
    return Status::kOk;
}

TEST(MacroTest, ReturnIfError) {
    EXPECT_TRUE(is_ok(helper_return_if_error(Status::kOk)));

    Status err = helper_return_if_error(Status::kNotFound);
    EXPECT_TRUE(is_error(err));
    EXPECT_EQ(err, Status::kNotFound);
}

// Helper for testing UME_ASSIGN_OR_RETURN
Result<int> helper_assign_or_return(Result<int> res_in) {
    UME_ASSIGN_OR_RETURN(int val, res_in);
    return val * 2;
}

TEST(MacroTest, AssignOrReturn) {
    Result<int> res_ok = helper_assign_or_return(Result<int>(21));
    EXPECT_TRUE(res_ok.ok());
    EXPECT_EQ(res_ok.value(), 42);

    Result<int> res_err = helper_assign_or_return(Result<int>(Status::kInvalidArgument));
    EXPECT_FALSE(res_err.ok());
    EXPECT_EQ(res_err.status(), Status::kInvalidArgument);
}

} // namespace
} // namespace ume

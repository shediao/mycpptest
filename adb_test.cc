
#include "adb_path.h"
#include <gtest/gtest.h>

auto home_dir = boost::filesystem::path(std::getenv("HOME"));
#ifdef __linux__
auto default_android_sdk = home_dir / "Android" / "Sdk";
#endif  // __linux__
#ifdef __APPLE__
auto default_android_sdk = home_dir / "Library" / "Android" / "sdk";
#endif  // __APPLE__

TEST(Adb, GetAdbdVersion) {
  ASSERT_GT(GetAdbdVersion(5037), 0);
}
TEST(Adb, GetDefaultAdb) {

  auto defaultAdb = GetDefaultAdb().path();
  ASSERT_EQ(defaultAdb, default_android_sdk / "platform-tools" / "adb") << defaultAdb;
}
TEST(Adb, GetNewestAdb) {
  ASSERT_EQ(GetNewestAdb().path(), default_android_sdk / "platform-tools" / "adb");
}
TEST(Adb, GetAdbFromPath) {
  ASSERT_NE(GetAdbFromPath().path(), default_android_sdk / "platform-tools" / "adb");
}
TEST(Adb, GetAllAdbs) {
  ASSERT_GT(GetAllAdbs().size(), 0);
}

TEST(Adb, Shell) {
  auto adb = GetDefaultAdb();
  auto r1 = adb.shell("echo", "-n",  "xxxx");
  ASSERT_EQ(r1.std_out , "xxxx");
  ASSERT_EQ(r1.std_err , "");
  ASSERT_EQ(r1.exit_code , 0);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

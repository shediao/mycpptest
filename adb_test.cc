
#include <iterator>
#include <gtest/gtest.h>


#include "adb.h"

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
  ASSERT_EQ(GetAdbFromPath().path(), default_android_sdk / "platform-tools" / "adb");
}
TEST(Adb, GetAllAdbs) {
  ASSERT_GT((int)GetAllAdbs().size(), 0);
}

TEST(Adb, Version) {
  auto adb = GetDefaultAdb();
  ASSERT_GT(adb.version(),39);
}

TEST(Adb, Shell) {
  auto adb = GetDefaultAdb();
  auto r1 = adb.shell("echo", "-n",  "xxxx");
  ASSERT_EQ(r1.std_out , "xxxx");
  ASSERT_EQ(r1.std_err , "");
  ASSERT_EQ(r1.exit_code , 0);
}

TEST(Adb, PushAndPull) {
  auto adb = GetDefaultAdb();
  int ret = adb.push("./adb_push_test_data.txt", "/data/local/tmp/");
  ASSERT_EQ(ret, 0);

  ASSERT_EQ(adb.shell("test -f /data/local/tmp/adb_push_test_data.txt && echo -n yes || echo -n no").std_out, "yes");
  ASSERT_EQ(adb.shell("cat", "/data/local/tmp/adb_push_test_data.txt").std_out, "1234567890\n");

  boost::filesystem::path tmp("mytestfile.txt");
  if (exists(tmp)) { remove(tmp); }
  ASSERT_EQ(adb.pull("/data/local/tmp/adb_push_test_data.txt", tmp.c_str()), 0);

  ASSERT_TRUE(exists(tmp));

  std::ifstream input(tmp.c_str());
  ASSERT_TRUE(input.is_open());
  std::string pull_file_data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
  ASSERT_EQ(pull_file_data, "1234567890\n");
  remove(tmp);
  adb.shell("test -f /data/local/tmp/adb_push_test_data.txt && rm -f /data/local/tmp/adb_push_test_data.txt");
  ASSERT_EQ(adb.shell("test -f /data/local/tmp/adb_push_test_data.txt && echo -n yes || echo -n no").std_out, "no");
}

std::vector<std::string> getAndroidDevices(int port);
TEST(Adb, devices) {
  auto adb = GetDefaultAdb();
  ASSERT_EQ(adb.devices(), getAndroidDevices(5037));
  for (auto&& device:getAndroidDevices(5037)) {
    std::cout << "device:" << device << std::endl;
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#ifndef ANDROID_DEVICE
#define ANDROID_DEVICE

#include "adb.h"

class AndroidDevice {
 public:
  explicit AndroidDevice(std::string serial):serial(std::move(serial)),adb(GetDefaultAdb()) { };
  void install();
  void uninstall();
  void list3partypackages();
  void listpackages();
  void start_activite();
  void stop_activite();
  int getPid();


 private:
  std::string serial;
  Adb adb;
};

#endif  // ANDROID_DEVICE


/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "crash_collector"

#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include <string>
#include <vector>

#include <base/file.h>
#include <cutils/properties.h>
#include <log/logger.h>
#include <private/android_filesystem_config.h>
#include <utils/String8.h>

#include "client/linux/minidump_writer/linux_core_dumper.h"
#include "client/linux/minidump_writer/minidump_writer.h"

#undef DISALLOW_COPY_AND_ASSIGN  // Defined in breakpad's header.
#include "coredump_writer.h"

namespace {

using android::String8;

const char kOutputDirectory[] = "/data/system/crash_reports";
const int kMaxNumReports = 16;

// Makes room for the new crash report by deleting old files when necessary.
bool MakeRoomForNewReport() {
  // Enumerate reports.
  std::unique_ptr<DIR, int(*)(DIR*)> dir(opendir(kOutputDirectory), closedir);
  if (!dir)
    return false;

  std::vector<time_t> dump_mtimes;  // Modification time of dump files.
  std::vector<std::pair<time_t, String8>> all_files;
  while (struct dirent* entry = readdir(dir.get())) {
    // Skip uninteresting entries.
    if (entry->d_name == std::string(".") ||
        entry->d_name == std::string(".."))
      continue;

    String8 filename = String8(kOutputDirectory).appendPath(entry->d_name);
    struct stat attributes;
    if (stat(filename.string(), &attributes))
      return false;
    all_files.push_back(std::make_pair(attributes.st_mtime, filename));
    if (filename.getPathExtension() == ".dmp")
      dump_mtimes.push_back(attributes.st_mtime);
  }
  dir.reset();

  // Remove old files.
  if (dump_mtimes.size() >= kMaxNumReports) {
    // Sort the vector (newer file comes first).
    std::sort(dump_mtimes.rbegin(), dump_mtimes.rend());

    const time_t threshold = dump_mtimes[kMaxNumReports - 1];
    for (const auto& file : all_files) {
      const time_t mtime = file.first;
      const String8& filename = file.second;
      if (mtime <= threshold) {
        if (unlink(filename))
          return false;
      }
    }
  }
  return true;
}

// Returns the specified system property.
std::string GetSystemProperty(const std::string& key) {
  char buf[PROPERTY_VALUE_MAX];
  property_get(key.c_str(), buf, "");
  return std::string(buf);
}

// Writes metadata as JSON file.
bool WriteMetadata(ssize_t coredump_size,
                   const std::string& pid,
                   const std::string& uid,
                   const std::string& gid,
                   const std::string& signal,
                   const std::string& username,
                   const std::string& exec_name,
                   const std::string& filename) {
  std::string content = "{";
  content += "\"version\":\"" + GetSystemProperty("ro.build.id") + "\"";
  content += ",";
  content += "\"coredump_size\":" + std::to_string(coredump_size);
  content += ",";
  content += "\"pid\":" + pid;
  content += ",";
  content += "\"uid\":" + uid;
  content += ",";
  content += "\"gid\":" + gid;
  content += ",";
  content += "\"signal\":" + signal;
  content += ",";
  content += "\"username\":\"" + username + "\"";
  content += ",";
  content += "\"exec_name\":\"" + exec_name + "\"";
  content += "}";
  return android::base::WriteStringToFile(
      content, filename, S_IRUSR | S_IWUSR, AID_SYSTEM, AID_SYSTEM);
}

// Converts the specified coredump file to a minidump.
bool ConvertCoredumpToMinidump(const std::string& coredump_filename,
                               const std::string& procfs_dir,
                               const std::string& minidump_filename) {
  google_breakpad::MappingList mappings;
  google_breakpad::AppMemoryList memory_list;
  google_breakpad::LinuxCoreDumper dumper(
      0, coredump_filename.c_str(), procfs_dir.c_str());
  bool success = google_breakpad::WriteMinidump(
      minidump_filename.c_str(), mappings, memory_list, &dumper) &&
      chown(minidump_filename.c_str(), AID_SYSTEM, AID_SYSTEM) == 0;
  unlink(coredump_filename.c_str());
  return success;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 7) {
    ALOGE("Insufficient args.");
    return 1;
  }
  const std::string pid_string = argv[1];
  const std::string uid_string = argv[2];
  const std::string gid_string = argv[3];
  const std::string signal_string = argv[4];
  const std::string crash_time = argv[5];
  const std::string exec_name = argv[6];

  const uid_t uid = std::stoi(uid_string);
  const uid_t appid = uid % AID_USER;
  if (appid >= AID_APP) {  // Ignore non-system crashes.
    return 0;
  }

  // Username lookup.
  std::string username;
  for (size_t i = 0; i < android_id_count; ++i) {
    if (android_ids[i].aid == appid) {
      username = android_ids[i].name;
      break;
    }
  }
  // Delete old crash reports.
  if (!MakeRoomForNewReport()) {
    ALOGE("Failed to delete old crash reports.");
    return 1;
  }
  // Read coredump from stdin.
  const std::string basename =
      std::string(kOutputDirectory) + "/" + crash_time + "." + pid_string;
  const std::string coredump = basename + ".core";
  CoredumpWriter coredump_writer;
  const ssize_t coredump_size =
      coredump_writer.WriteCoredump(STDIN_FILENO, coredump);
  if (coredump_size > 0) {
    // Convert coredump to minidump.
    const std::string procfs_dir = "/proc/" + pid_string;
    const std::string minidump = basename + ".dmp";
    if (!ConvertCoredumpToMinidump(coredump, procfs_dir, minidump)) {
      ALOGE("Failed to convert coredump to minidump.");
    }
  } else {
    ALOGE("Failed to copy coredump from stdin.");
  }
  // Write metadata.
  const std::string metadata = basename + ".meta";
  if (!WriteMetadata(coredump_size, pid_string, uid_string, gid_string,
                     signal_string, username, exec_name, metadata)) {
    ALOGE("Failed to write metadata.");
    return 1;
  }
  return 0;
}

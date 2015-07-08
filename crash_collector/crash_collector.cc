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

namespace {

using android::String8;

const char kOutputDirectory[] = "/data/system/crash_reports";
const int kMaxNumReports = 16;
const int kMaxCoredumpSize = 100*1024*1024;

// Makes room for the new crash report by deleting old files when necessary.
bool MakeRoomForNewReport() {
  // Enumerate reports.
  std::unique_ptr<DIR, int(*)(DIR*)> dir(opendir(kOutputDirectory), closedir);
  if (!dir)
    return false;

  std::vector<time_t> dump_mtimes;  // Modification time of dump files.
  std::vector<std::pair<time_t, String8>> all_files;
  while (struct dirent* entry = readdir(dir.get())) {
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
bool WriteMetadata(const std::string& exec_name,
                   const std::string& filename) {
  std::string content = "{";
  content += "\"version\":\"" + GetSystemProperty("ro.build.id") + "\"";
  content += ",";
  content += "\"exec_name\":\"" + exec_name + "\"";
  content += "}";
  return android::base::WriteStringToFile(
      content, filename, S_IRUSR | S_IWUSR, AID_SYSTEM, AID_SYSTEM);
}

// Copies fd_src's contents to fd_dest.
bool CopyFile(int fd_src, int fd_dest) {
  const size_t kBufSize = 32768;
  char buf[kBufSize];
  size_t total = 0;
  while (true) {
    int rv = TEMP_FAILURE_RETRY(read(fd_src, buf, kBufSize));
    if (rv == 0)
      break;
    if (rv == -1 || !android::base::WriteFully(fd_dest, buf, rv))
      return false;

    // Limit the destination file size.
    total += rv;
    if (total > kMaxCoredumpSize)
      return false;
  }
  return true;
}

// Copies STDIN to the specified file.
bool CopyStdinToFile(const std::string& dest) {
  int fd = TEMP_FAILURE_RETRY(open(
      dest.c_str(), O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR));
  if (fd == -1) {
    ALOGE("Failed to open: %s %d", dest.c_str(), errno);
    return false;
  }
  bool success = CopyFile(STDIN_FILENO, fd);
  close(fd);
  if (!success)
    unlink(dest.c_str());
  return success;
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
  if (argc < 4) {
    ALOGE("Insufficient args.");
    return 1;
  }
  const std::string pid_string = argv[1];
  const std::string crash_time = argv[2];
  const std::string exec_name = argv[3];

  // Delete old crash reports.
  if (!MakeRoomForNewReport()) {
    ALOGE("Failed to delete old crash reports.");
    return 1;
  }
  // Write metadata.
  const std::string basename =
      std::string(kOutputDirectory) + "/" + crash_time + "." + pid_string;
  const std::string metadata = basename + ".meta";
  if (!WriteMetadata(exec_name, metadata)) {
    ALOGE("Failed to write metadata.");
    return 1;
  }
  // Read coredump from stdin.
  const std::string coredump = basename + ".core";
  if (!CopyStdinToFile(coredump)) {
    ALOGE("Failed to copy coredump from stdin.");
    return 1;
  }
  // Convert coredump to minidump.
  const std::string procfs_dir = "/proc/" + pid_string;
  const std::string minidump = basename + ".dmp";
  if (!ConvertCoredumpToMinidump(coredump, procfs_dir, minidump)) {
    ALOGE("Failed to convert coredump to minidump.");
    return 1;
  }
  return 0;
}

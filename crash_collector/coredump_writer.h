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

#ifndef COREDUMP_WRITER_H_
#define COREDUMP_WRITER_H_

#include <set>
#include <string>
#include <vector>

#include <elf.h>
#include <link.h>

#include <base/macros.h>

class CoredumpWriter {
 public:
  CoredumpWriter();
  ~CoredumpWriter();

  // Reads coredump from |fd_src|, writes it to |coredump_filename|, and
  // returns the number of bytes written, or -1 on errors.
  ssize_t WriteCoredump(int fd_src, const std::string& coredump_filename);

 private:
  using Ehdr = ElfW(Ehdr);
  using Phdr = ElfW(Phdr);

  // Virtual address occupied by a mapped file.
  using FileRange = std::pair<long, long>;
  using FileRangeSet = std::set<FileRange>;

  class FdReader;

  ssize_t WriteCoredumpToFD(int fd_src, int fd_dest);

  // Reads ELF header, all program headers, and NOTE segment from fd_src.
  bool ReadUntilNote(FdReader* reader,
                     Ehdr* elf_header,
                     std::vector<Phdr>* program_headers,
                     std::vector<char>* note_buf);

  // Extracts a set of address ranges occupied by mapped files from NOTE segment.
  bool GetFileRangeSet(const std::vector<char>& note_buf,
                       FileRangeSet* file_range_set);

  // Filters out unneeded segments.
  void FilterSegments(const std::vector<Phdr>& program_headers,
                      const FileRangeSet& file_range_set,
                      std::vector<Phdr>* program_headers_filtered);

  DISALLOW_COPY_AND_ASSIGN(CoredumpWriter);
};

#endif  // COREDUMP_WRITER_H_

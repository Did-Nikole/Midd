/*
 * Copyright (C) 2026 Nikole Smith (ApptsolutioNZ - appsolutionz.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "midd.h"
#include "Logger.hpp"
#include <cstdio>

Midd::Result Midd::midd(std::istream &input, std::ostream &output,
                        const Options &opt) {
  unsigned long long count = opt.one_indexed ? 1 : 0;

  std::string read_line;
  bool outputLine = !opt.start.enabled;
  bool hasStripped = opt.printable_only;

  Logger::getInstance().log("Entering Midd::midd");
  Logger::getInstance().log(
      "Options: start.enabled=" + std::to_string(opt.start.enabled) +
      ", start.value=" + std::to_string(opt.start.value) +
      ", end.enabled=" + std::to_string(opt.end.enabled) +
      ", end.value=" + std::to_string(opt.end.value) + ", one_indexed=" +
      std::to_string(opt.one_indexed) + ", lines=" + std::to_string(opt.lines));

  bool doBookEnd = false;
  unsigned long long last_printed_line = count;

  while (std::getline(input, read_line)) {

    if (opt.start.enabled && count >= opt.start.value) {
      if (!outputLine) {
        Logger::getInstance().log("Line " + std::to_string(count) +
                                  ": Start condition met, enabling output.");
        outputLine = true;
      }
    }

    if (outputLine) {
      if (!doBookEnd) {
        doBookEnd = true;
        if (!opt.quiet) {
          std::string bookmark = opt.bookmarkstart;
          size_t pos = bookmark.find("%");
          if (pos != std::string::npos) {
            bookmark.replace(pos, 1, std::to_string(count));
          }
          output << bookmark << "\n";
        }
      }
      output << (opt.lines ? std::to_string(count) + "\t" : "")
             << stripNonPrintable(read_line, hasStripped) << "\n";
      last_printed_line = count;
    }

    if (opt.end.enabled && count >= opt.end.value) {
      Logger::getInstance().log("Line " + std::to_string(count) +
                                ": End condition met, breaking loop.");
      break; // exit immediately after printing the last line
    }

    count++;
  }

  if (doBookEnd && !opt.quiet) {
    std::string bookmark = opt.bookmarkend;
    size_t pos = bookmark.find("%d");
    if (pos != std::string::npos) {
      bookmark.replace(pos, 2, std::to_string(last_printed_line));
    }
    output << bookmark << "\n";
  }

  Logger::getInstance().log("Exiting Midd::midd. Total lines read: " +
                            std::to_string(count));
  return {.isError = false};
};

Midd::Result Midd::parseRange(const std::string &range, Options &opt) {
  // if range doesnt contain ":" return error invalid range format
  size_t colon_pos = range.find(":");
  if (colon_pos == std::string::npos) {
    return {.isError = true, .error = "Invalid range format"};
  }

  auto isValidComponent = [](const std::string &s) {
    if (s.empty() || s == "$") return true;
    for (char c : s) {
      if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
  };

  // if start of range is not $ or empty string set opt.start.enabled to true
  if (range.substr(0, colon_pos) != "$" && colon_pos > 0) {
    std::string start_str = range.substr(0, colon_pos);
    if (!isValidComponent(start_str)) {
      return {.isError = true, .error = "Invalid range format"};
    }
    try {
      opt.start.enabled = true;
      opt.start.value = std::stoull(start_str);
    } catch (...) {
      return {.isError = true, .error = "Range value out of bounds"};
    }
  }

  if (range.substr(colon_pos + 1) != "$" && colon_pos + 1 < range.size()) {
    std::string end_str = range.substr(colon_pos + 1);
    if (!isValidComponent(end_str)) {
      return {.isError = true, .error = "Invalid range format"};
    }
    try {
      opt.end.enabled = true;
      opt.end.value = std::stoull(end_str);
    } catch (...) {
      return {.isError = true, .error = "Range value out of bounds"};
    }
  }
  // if range is invalid return error
  if (opt.start.enabled && opt.end.enabled && opt.start.value > opt.end.value) {
    return {.isError = true,
            .error = "Start must be less than or equal to end"};
  }
  return {.isError = false};
}

std::string Midd::stripNonPrintable(const std::string &input,
                                    bool &hasStripped) {
  std::string cleaned;
  cleaned.reserve(input.size());

  for (size_t i = 0; i < input.size(); ++i) {
    char c = input[i];
    // Check if it starts an ANSI escape sequence: ESC (0x1b)
    if (c == '\x1b') {
      hasStripped = true;
      if (i + 1 < input.size() && input[i + 1] == '[') {
        i += 2;
        while (i < input.size() &&
               !std::isalpha(static_cast<unsigned char>(input[i]))) {
          i++;
        }
        continue;
      }
    }

    if (std::isprint(static_cast<unsigned char>(c)) || c == '\t' || c == '\r' ||
        c == '\n') {
      cleaned.push_back(c);
    } else {
      hasStripped = true;
    }
  }
  return cleaned;
}
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
#pragma once

#include <istream>
#include <string>
#include <vector>
class Midd {
public:
  struct Result {
    bool isError = false;
    std::string error = "";
  };

  struct ull_w_overide {
    bool enabled;
    unsigned long long value;
  };

  struct Options {
    bool debug = false;
    bool lines = false;
    bool one_indexed = false;
    bool quiet = false;
    bool printable_only = false;
    ull_w_overide start = {.enabled = false, .value = 0};
    ull_w_overide end = {.enabled = false, .value = 0};
    std::string bookmarkstart = "[SLICE START: %]";
    std::string bookmarkend = "[SLICE END: %]";
  };

  static Result midd(std::istream &input, std::ostream &output,
                     const Options &opt);

  static std::string stripNonPrintable(const std::string &input,
                                       bool &hasStripped);

  static Result parseRange(const std::string &range, Options &opt);
};
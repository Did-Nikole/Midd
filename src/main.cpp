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
#define ARGSPARSERRENEWED_IMPLEMENTATION
#include "ArgsParser.hpp"
#include "Logger.hpp"
#include "midd.h"
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>

int main(int argc, char **argv) {

  std::map<std::string, std::optional<ArgValue>> results;
  ArgsParser parser;

  // std::stringstream ss_input;
  // std::stringstream ss_output;

  std::istream *input_ptr = &std::cin;
  std::ostream *output_ptr = &std::cout;

  /* argument definitions
   * Flag	Argument	Description	Default
   * -i	string Path to the input file.
   *    - default STDIN
   * -o	string	Path to the output file.
   *    - default STDOUT
   * -r	string	The target line range formatted strictly as start:end.
   *    - default $:$ (Full file)
   * -l	None	Enables visual line numbering prefixes in the output.
   *    - default Disabled
   * -1	None	Shifts range parsing and line numbering to 1-indexed mode.
   *    - default 0-indexed
   * -q	None	Quiet mode. Suppresses informational slice boundary markers.
   *    - default Disabled
   * -p	None	Printable-only mode. Strips non-printable characters.
   *    - Disabled
   * -h	None	Display help message.
   *    - default Disabled
   */

  parser.addArgument({.type = ArgType::BOOLEAN,
                      .shortFlag = "-h",
                      .longFlag = "--help",
                      .description = "Display help message",
                      .required = false,
                      .defaultValue = false});

  parser.addArgument({.type = ArgType::STRING,
                      .shortFlag = "-r",
                      .longFlag = "--range",
                      .description = "Range to extract: <start>:<end>"
                                     " (if param is $ or blank its"
                                     " assumed to be sof or eof)",
                      .required = false,
                      .defaultValue = std::string("$:$")});

  parser.addArgument({.type = ArgType::STRING,
                      .shortFlag = "-i",
                      .longFlag = "--input",
                      .description = "Input file",
                      .required = false,
                      .defaultValue = std::string("stdin")});

  parser.addArgument({.type = ArgType::STRING,
                      .shortFlag = "-o",
                      .longFlag = "--output",
                      .description = "Output file",
                      .required = false,
                      .defaultValue = std::string("stdout")});

  parser.addArgument({.type = ArgType::BOOLEAN,
                      .shortFlag = "-l",
                      .longFlag = "--lines",
                      .description = "Prefix line numbers",
                      .required = false,
                      .defaultValue = false});

  parser.addArgument({.type = ArgType::BOOLEAN,
                      .shortFlag = "-1",
                      .longFlag = "--one-indexed",
                      .description = "Start line numbering"
                                     " at 1 instead of default 0",
                      .required = false,
                      .defaultValue = false});

  parser.addArgument({.type = ArgType::BOOLEAN,
                      .shortFlag = "-q",
                      .longFlag = "--quiet",
                      .description = "Suppress informational output",
                      .required = false,
                      .defaultValue = false});

  parser.addArgument({.type = ArgType::BOOLEAN,
                      .shortFlag = "-p",
                      .longFlag = "--printable-only",
                      .description = "Print only printable characters",
                      .required = false,
                      .defaultValue = false});

  parser.addArgument({.type = ArgType::BOOLEAN,
                      .shortFlag = "-d",
                      .longFlag = "--debug",
                      .description = "Enable debug logging to midd.log",
                      .required = false,
                      .defaultValue = false});

  // parse arguments
  if (!parser.parse(argc, argv, results)) {
    parser.showHelp();
    return 1;
  }

  // check if help was requested
  if (results.at("help").has_value() &&
      std::get<bool>(results.at("help").value())) {
    parser.showHelp();
    return 0;
  }

  // check if input was provided
  if (results.at("input").has_value()) {
    auto instring = std::get<std::string>(results.at("input").value());
    if (instring != "stdin") {
      auto *infile = new std::ifstream(instring);
      if (!infile->is_open()) {
        std::cerr << "Error: Cannot open input file '" << instring
                  << "': " << std::strerror(errno) << std::endl;
        delete infile;
        return 1;
      }
      input_ptr = infile;
    }
  }

  // check if output was provided
  if (results.at("output").has_value()) {
    auto outstring = std::get<std::string>(results.at("output").value());
    if (outstring != "stdout") {
      auto *outfile = new std::ofstream(outstring);
      if (!outfile->is_open()) {
        std::cerr << "Error: Cannot open output file '" << outstring
                  << "': " << std::strerror(errno) << std::endl;
        delete outfile;
        if (input_ptr != &std::cin) {
          delete input_ptr;
        }
        return 1;
      }
      output_ptr = outfile;
    }
  }

  // alloc opts
  Midd::Options opts;

  if (results.at("quiet").has_value()) {
    opts.quiet = std::get<bool>(results.at("quiet").value());
    Logger::getInstance().log("Quiet mode enabled");
  }
  if (results.at("debug").has_value()) {
    opts.debug = std::get<bool>(results.at("debug").value());
    Logger::getInstance().log("Debug mode enabled");
  }
  if (results.at("lines").has_value()) {
    opts.lines = std::get<bool>(results.at("lines").value());
    Logger::getInstance().log("Lines mode enabled");
  }
  if (results.at("one-indexed").has_value()) {
    opts.one_indexed = std::get<bool>(results.at("one-indexed").value());
    Logger::getInstance().log("One-indexed mode enabled");
  }
  if (results.at("quiet").has_value()) {
    opts.quiet = std::get<bool>(results.at("quiet").value());
  }
  if (results.at("printable-only").has_value()) {
    opts.printable_only = std::get<bool>(results.at("printable-only").value());
  }

  // Initialize logger in executable's directory
  std::string argv0 = argv[0];
  size_t pos = argv0.find_last_of("/\\");
  std::string exe_dir = (pos == std::string::npos) ? "." : argv0.substr(0, pos);
  Logger::getInstance().init(exe_dir + "/midd.log", opts.debug);

  Logger::getInstance().log(
      "Starting midd with command line arguments processed.");

  // process options into opts
  auto parse_result = Midd::parseRange(
      std::get<std::string>(results.at("range").value()), opts);
  if (parse_result.isError) {
    std::cerr << "Error: " << parse_result.error << std::endl;
    return 1;
  }

  auto mid_result = Midd::midd(*input_ptr, *output_ptr, opts);

  if (input_ptr != &std::cin) {
    delete input_ptr;
  }
  if (output_ptr != &std::cout) {
    delete output_ptr;
  }

  if (mid_result.isError) {
    std::cerr << mid_result.error << std::endl;
    return 1;
  }

  return 0;
}
/**
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __STRINGS_H__
#define __STRINGS_H__

#include <algorithm>
#include <sstream>
#include <string>
#include <map>
#include <vector>

namespace strings {

// Flags indicating how remove should operate.
enum Mode {
  PREFIX,
  SUFFIX,
  ANY
};


inline std::string remove(
    const std::string& from,
    const std::string& substring,
    Mode mode = ANY)
{
  std::string result = from;

  if (mode == PREFIX) {
    if (from.find(substring) == 0) {
      result = from.substr(substring.size());
    }
  } else if (mode == SUFFIX) {
    if (from.rfind(substring) == from.size() - substring.size()) {
      result = from.substr(0, from.size() - substring.size());
    }
  } else {
    size_t index;
    while ((index = result.find(substring)) != std::string::npos) {
      result = result.erase(index, substring.size());
    }
  }

  return result;
}


inline std::string trim(
    const std::string& from,
    const std::string& chars = " \t\n\r")
{
  size_t start = from.find_first_not_of(chars);
  size_t end = from.find_last_not_of(chars);
  if (start == std::string::npos) { // Contains only characters in chars.
    return "";
  }

  return from.substr(start, end + 1 - start);
}

inline bool startsWith(const std::string& s, const std::string& prefix)
{
  return s.find(prefix) == 0;
}


inline bool endsWith(const std::string& s, const std::string& suffix)
{
  return s.rfind(suffix) == s.length() - suffix.length();
}


inline bool contains(const std::string& s, const std::string& substr)
{
  return s.find(substr) != std::string::npos;
}


inline std::string lower(const std::string& s)
{
  std::string result = s;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}


inline std::string upper(const std::string& s)
{
  std::string result = s;
  std::transform(result.begin(), result.end(), result.begin(), ::toupper);
  return result;
}

} // namespace strings {

#endif // __STRINGS_H__

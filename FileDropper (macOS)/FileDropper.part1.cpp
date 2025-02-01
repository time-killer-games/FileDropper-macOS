/*

 MIT License

 Copyright © 2020 Samuel Venable

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

*/

#include "FileDropper.h"
#include "SharedFuncs.h"

#include <sys/stat.h>

#include <algorithm>
#include <sstream>

#include <string>
#include <vector>

using std::string;
using std::vector;

using std::size_t;

extern "C" bool cocoa_file_dnd_get_enabled();
extern "C" void cocoa_file_dnd_set_enabled(bool enable);
extern "C" const char *cocoa_file_dnd_get_files();
extern "C" void *cocoa_file_dnd_get_hwnd();
extern "C" void cocoa_file_dnd_set_hwnd(void *hwnd);

static void *window_handle = NULL;

static string fname;
static string result;

static string def_pattern;
static bool def_allowfiles = true;
static bool def_allowdirs = true;
static bool def_allowmulti = true;

static bool file_exists(string fname) {
  struct stat sb;
  if (stat((char *)fname.c_str(), &sb) == 0 &&
    S_ISREG(sb.st_mode))
    return true;
  return false;
}

static bool directory_exists(string dname) {
  struct stat sb;
  if (stat((char *)dname.c_str(), &sb) == 0 &&
    S_ISDIR(sb.st_mode))
    return true;
  return false;
}

static string filename_name(string fname) {
  size_t fp = fname.find_last_of("/");
  return fname.substr(fp + 1);
}

static string filename_ext(string fname){
  fname = filename_name(fname);
  size_t fp = fname.find_last_of(".");
  if (fp == string::npos)
    return "";
  return fname.substr(fp);
}

static string string_replace_all(string str, string substr, string newstr) {
  size_t pos = 0;
  const size_t sublen = substr.length(), newlen = newstr.length();
  while ((pos = str.find(substr, pos)) != string::npos) {
    str.replace(pos, sublen, newstr);
    pos += newlen;
  }
  return str;
}

static std::vector<string> string_split(const string &str, char delimiter) {
  std::vector<string> vec;
  std::stringstream sstr(str);
  string tmp;
  while (std::getline(sstr, tmp, delimiter))
    vec.push_back(tmp);
  return vec;
}

static void file_dnd_add_files(string files) {
  if (files != "") {
    std::vector<string> pathVec = string_split(files, '\n');
    for (const string &path : pathVec) {
      if (file_exists(path) || directory_exists(path)) {
        if (fname != "") fname += "\n";
        fname += path;
      }
    }
    std::vector<string> nameVec = string_split(fname, '\n');
    sort(nameVec.begin(), nameVec.end());
    nameVec.erase(unique(nameVec.begin(), nameVec.end()), nameVec.end());
    std::vector<string>::size_type sz = nameVec.size();
    fname = "";
    for (std::vector<string>::size_type i = 0; i < sz; i += 1) {
      if (fname != "") fname += "\n";
      fname += nameVec[i];
    }
  }
}

static void file_dnd_apply_filter(string pattern, bool allowfiles, bool allowdirs, bool allowmulti) {
  if (pattern == "") { pattern = "."; }
  pattern = string_replace_all(pattern, " ", "");
  pattern = string_replace_all(pattern, "*", "");
  std::vector<string> extVec = string_split(pattern, ';');
  std::vector<string> nameVec = string_split(fname, '\n');
  std::vector<string>::size_type sz1 = nameVec.size();
  std::vector<string>::size_type sz2 = extVec.size();
  fname = "";
  for (std::vector<string>::size_type i2 = 0; i2 < sz2; i2 += 1) {
    for (std::vector<string>::size_type i1 = 0; i1 < sz1; i1 += 1) {
      if (extVec[i2] == "." || extVec[i2] == filename_ext(nameVec[i1])) {
        if (fname != "") fname += "\n";
        fname += nameVec[i1];
      }
    }
  }
  nameVec = string_split(fname, '\n');
  sz1 = nameVec.size();
  fname = "";
  if (allowmulti) {
    for (std::vector<string>::size_type i = 0; i < sz1; i += 1) {
      if (allowfiles && file_exists(nameVec[i])) {
        if (fname != "") fname += "\n";
        fname += nameVec[i];
      } else if (allowdirs && directory_exists(nameVec[i])) {
        if (fname != "") fname += "\n";
        fname += nameVec[i];
      }
    }
  } else {
    if (!nameVec.empty()) {
      if (allowfiles && file_exists(nameVec[0])) {
        if (fname != "") fname += "\n";
        fname += nameVec[0];
      } else if (allowdirs && directory_exists(nameVec[0])) {
        if (fname != "") fname += "\n";
        fname += nameVec[0];
      }
    }
  }
}

static string file_dnd_get_files_helper() {
  string str_file_dnd_get_files;
  if (cocoa_file_dnd_get_files() != NULL)
    str_file_dnd_get_files = cocoa_file_dnd_get_files();
  if (str_file_dnd_get_files != "") {
    file_dnd_add_files(str_file_dnd_get_files);
    file_dnd_apply_filter(def_pattern, def_allowfiles, def_allowdirs, def_allowmulti);
  }
  while (fname.back() == '\n')
    fname.pop_back();
  return fname;
}

double file_dnd_get_enabled() {
  return cocoa_file_dnd_get_enabled();
}

double file_dnd_set_enabled(double enable) {
  cocoa_file_dnd_set_enabled((bool)enable);
  if (!cocoa_file_dnd_get_enabled()) fname = "";
  return 0;
}

char *file_dnd_get_files() {
  if (!file_dnd_get_enabled()) {
    result = ""; return (char *)"";
  }
  string helper = file_dnd_get_files_helper();
  if (def_allowmulti) {
    if (result != "") result += "\n";
    result += helper;
    std::vector<string> pathVec = string_split(result, '\n');
    result = "";

    for (const string &path : pathVec) {
      if (file_exists(path) || directory_exists(path)) {
        if (result != "") result += "\n";
        result += path;
      }
    }
    std::vector<string> nameVec = string_split(result, '\n');
    sort(nameVec.begin(), nameVec.end());
    nameVec.erase(unique(nameVec.begin(), nameVec.end()), nameVec.end());
    std::vector<string>::size_type sz = nameVec.size();
    result = "";
    for (std::vector<string>::size_type i = 0; i < sz; i += 1) {
      if (result != "") result += "\n";
      result += nameVec[i];
    }
  } else {
    if (file_exists(helper) || directory_exists(helper))
      result = helper;
  }
  if (helper != "") {
    file_dnd_set_enabled(false);
    file_dnd_set_enabled(true);
  }
  return (char *)result.c_str();
}

double file_dnd_set_files(char *pattern, double allowfiles, double allowdirs, double allowmulti) {
  def_pattern = pattern;
  def_allowfiles = allowfiles;
  def_allowdirs = allowdirs;
  def_allowmulti = allowmulti;
  return 0;
}

void *file_dnd_get_hwnd() {
  return cocoa_file_dnd_get_hwnd();
}

double file_dnd_set_hwnd(void *hwnd) {
  cocoa_file_dnd_set_hwnd(hwnd);
  return 0;
}

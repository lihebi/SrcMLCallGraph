#ifndef UTILS_H
#define UTILS_H

#include <pugixml.hpp>
#include <stdlib.h>
#include <vector>
#include <set>
#include <string>
#include <map>
#include <stack>
#include <fstream>


namespace utils {

  /*******************************
   ** File utils
   *******************************/

  std::vector<std::string> get_files(const std::string &folder);
  void get_files(const std::string &folder, std::vector<std::string>& vs);
  void get_files_by_extension(const std::string& folder, std::vector<std::string>& vs, const std::string& s);
  void get_files_by_extension(const std::string& folder, std::vector<std::string>& vs, const std::vector<std::string>& extension);
  bool file_exists(const std::string& file);
  bool is_file(const std::string &file);
  bool is_dir(const std::string &file);


  /*******************************
   * read/write
   *******************************/
  void write_file(const std::string& file, const std::string& content);
  void append_file(const std::string& file, const std::string& content);
  std::string read_file(const std::string& file);
  // folders
  void remove_folder(const std::string& folder);
  void create_folder(const std::string& folder);

  void visualize_dot_graph(const std::string& dot);

  std::string create_tmp_dir(std::string s="/tmp/helium-XXXXXX");

  std::vector<std::string> query_xml(const std::string& xml_file, const std::string& query);
  std::string query_xml_first(const std::string& xml_file, const std::string& query);


  /*******************************
   ** Srcml Utils
   *******************************/

  void file2xml(const std::string& filename, pugi::xml_document& doc);
  void string2xml(const std::string& code, pugi::xml_document& doc);


  std::string exec(const char* cmd, int *status=NULL, int timeout=0);
  // with input
  std::string exec_in(const char* cmd, const char* input, int *status=NULL, unsigned int timeout=0) ;
} // end namespace utils
#endif /* UTILS_H */


#include "utils.h"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

namespace fs = boost::filesystem;

/*******************************
 ** FileUtil
 *******************************/

/**
 * Get all files in folder.
 */
std::vector<std::string> utils::get_files(const std::string& folder) {
  std::vector<std::string> vs;
  get_files(folder, vs);
  return vs;
}
/**
 * get all files of the folder.
 * @param[in] folder
 * @param[out] vs
 */
void utils::get_files(const std::string& folder, std::vector<std::string>& vs) {
  fs::path project_folder(folder);
  fs::recursive_directory_iterator it(folder), eod;
  BOOST_FOREACH (fs::path const & p, std::make_pair(it, eod)) {
    if (is_regular_file(p)) {
      vs.push_back(p.string());
    }
  }
}

/**
 * Get files in a folder, with ONE specific extension.
 * @param[in] folder
 * @param[out] vs
 * @param[in] extension a suffix as string
 */
void utils::get_files_by_extension(
  const std::string& folder,
  std::vector<std::string>& vs,
  const std::string& extension
) {
  if (!is_dir(folder)) return;
  std::vector<std::string> ve {extension};
  get_files_by_extension(folder, vs, ve);
}


bool utils::is_file(const std::string &file) {
  struct stat sb;
  if (stat(file.c_str(), &sb) == 0 && S_ISREG(sb.st_mode)) {
    return true;
  }
  return false;
}
bool utils::is_dir(const std::string &file) {
  struct stat sb;
  if (stat(file.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
    return true;
  }
  return false;
}


/**
 * Get files in a folder, with some specific extensions.
 * @param[in] folder folder to search
 * @param[out] vs a list of matched files
 * @param[in] extension a list of extensions
 */
void utils::get_files_by_extension(
  const std::string& folder,
  std::vector<std::string>& vs,
  const std::vector<std::string>& extension
) {
  if (!is_dir(folder)) return;
  fs::path project_folder(folder);
  fs::recursive_directory_iterator it(folder), eod;
  BOOST_FOREACH(fs::path const& p, std::make_pair(it, eod)) {
    if (is_regular_file(p)) {
      std::string with_dot = p.extension().string();
      // the extension may not exist
      if (!with_dot.empty()) {
        // if extension does not exist,
        // the substr(1) will cause exception of out of range of basic_string
        std::string without_dot = with_dot.substr(1);
        if (std::find(extension.begin(), extension.end(), with_dot) != extension.end()
        || std::find(extension.begin(), extension.end(), without_dot) != extension.end()) {
          vs.push_back(p.string());
        }
      }
    }
  }
}

bool
utils::file_exists(const std::string& file) {
  fs::path file_path(file);
  return fs::exists(file_path);
}

/**
 * Write file with content.
 * Will overwrite. If the file doesn't exist, it will create it recursively(the parent directory)
 */
void
utils::write_file(const std::string& file, const std::string& content) {
  fs::path file_path(file);
  fs::path dir = file_path.parent_path();
  if (!fs::exists(dir)) {
    fs::create_directories(dir);
  }
  std::ofstream os;
  os.open(file_path.string());
  if (os.is_open()) {
    os << content;
    os.close();
  }
}

/**
 * Append content to file. Create if not existing.
 */
void
utils::append_file(const std::string& file, const std::string& content) {
  fs::path file_path(file);
  fs::path dir = file_path.parent_path();
  if (!fs::exists(dir)) {
    fs::create_directories(dir);
  }
  std::ofstream os;
  os.open(file_path.string(), std::ios_base::app);
  if (os.is_open()) {
    os<<content;
    os.close();
  }
}

/**
 * Read a file into string.
 */
std::string
utils::read_file(const std::string& file) {
  std::ifstream is;
  is.open(file);
  std::string code;
  if (is.is_open()) {
    std::string line;
    while(getline(is, line)) {
      code += line+"\n";
    }
    is.close();
  }
  return code;
}

/**
 * rm -r <folder>
 */
void
utils::remove_folder(const std::string& folder) {
  fs::path folder_path(folder);
  if (fs::exists(folder_path)) {
    fs::remove_all(folder_path);
  }
}

/**
 * mkdir -p <folder>
 */
void
utils::create_folder(const std::string& folder) {
  if (folder.empty()) return;
  fs::path folder_path(folder);
  if (!fs::exists(folder_path)) {
    fs::create_directories(folder_path);
  }
}

void utils::visualize_dot_graph(const std::string& dot) {
  std::string dir = create_tmp_dir();
  utils::write_file(dir + "/out.dot", dot);
  std::string cmd_png = "dot -Tpng "+dir+"/out.dot -o "+dir+"/out.png";
  utils::exec(cmd_png.c_str());
  std::cout << "wrote to file: " + dir + "/out.png"  << "\n";
#ifdef __MACH__
  std::string display_cmd = "open "+dir+"/out.png";
#else
  std::string display_cmd = "display "+dir+"/out.png";
#endif
  utils::exec(display_cmd.c_str());
}

/**
 * create tmp dir, return it.
 * @input s /tmp/helium-XXXXXX (must have 6 X at the end.)
 */
std::string utils::create_tmp_dir(std::string s) {
  // char tmp_dir[] = "/tmp/helium-test-temp.XXXXXX";
  std::string sub = s.substr(s.find_last_not_of('X'));
  if (sub.size() !=7) return "";
  sub = sub.substr(1);
  assert(sub.size() == 6 && "tmp dir url format error!");
  if (sub.find_first_not_of('X') != std::string::npos) return "";
  char tmp_dir[s.size()+1];
  strcpy(tmp_dir, s.c_str());
  char *result = mkdtemp(tmp_dir);
  if (result == NULL) return "";
  std::string dir = tmp_dir;
  return dir;
}

/*******************************
 ** SrcmlUtil
 *******************************/

/**
 * Query query on code.
 * return the first matching.
 */
std::string utils::query_xml_first(const std::string& xml_file, const std::string& query) {
  pugi::xml_document doc;
  file2xml(xml_file, doc);
  pugi::xml_node root_node = doc.document_element();
  pugi::xml_node node = root_node.select_node(query.c_str()).node();
  return node.child_value();
}
/**
 * Query "query" on "code".
 * Return all matching.
 * Will not use get_text_content, but use child_value() for a xml tag.
 * Only support tag value currently, not attribute value.
 */
std::vector<std::string> utils::query_xml(const std::string& xml_file, const std::string& query) {
  std::vector<std::string> result;
  pugi::xml_document doc;
  file2xml(xml_file, doc);
  pugi::xml_node root_node = doc.document_element();
  pugi::xpath_node_set nodes = root_node.select_nodes(query.c_str());
  for (auto it=nodes.begin();it!=nodes.end();it++) {
    pugi::xml_node node = it->node();
    result.push_back(node.child_value());
  }
  return result;
}




/**
 * convert a file into srcml output
 * @param[in] filename
 * @param[out] doc doc must be created by caller
 */
void utils::file2xml(const std::string &filename, pugi::xml_document& doc) {
  std::string cmd;
  cmd = "srcml --position " + filename;
  // cmd = "srcml " + filename;
  std::string xml = exec(cmd.c_str(), NULL);
  doc.load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
}
// c code will be converted into xml, and loaded by doc
/**
 * Convert a string of code into srcml output.
 * Only support C code for now.
 * @param[in] code
 * @param[out] doc
 */
void utils::string2xml(const std::string &code, pugi::xml_document& doc) {
  std::string cmd = "srcml --position -lC++";
  // std::string cmd = "srcml -lC";
  std::string xml = exec_in(cmd.c_str(), code.c_str(), NULL);
  doc.load_string(xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
}


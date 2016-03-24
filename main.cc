#include <iostream>
#include <pugixml.hpp>
#include "utils.h"
#include "main.h"
#include "assert.h"
#include "analysis.h"

std::set<Class*> classes;
std::set<Function*> functions;
std::map<std::string, Function*> func_m;
std::map<std::string, Class*> class_m;
std::map<std::string, std::set<Class*> > method_m;


/**
 * 
 * 
 * scan all functions, preferably the main function
 * from main, get call, and resovle it.
 */

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "usage: executable /path/to/benchmark/folder"  << "\n";
  }
  /**
   * scan all files, pass them into srcml document
   */
  std::vector<std::string> files;
  utils::get_files_by_extension(argv[1], files, {"cpp", "h", "cc", "hpp", "c"});
  // use this to hold all documents, but I will not directly query them
  std::vector<pugi::xml_document*> docs;
  for (std::string file : files) {
    pugi::xml_document *doc = new pugi::xml_document;
    utils::file2xml(file, *doc);
    docs.push_back(doc);
  }
  /**
   * get all class definition, record the name and their method
   * Get the class hierarchy!
   * A : B : C
   * Now only get the name of super class.
   * After I gather all classes, I match them by name.
   * I do not consider namespaces yet.
   * So if there're two classes for the same name, just choose one as you like
   */

  /**
   * Also get all function definitions
   */

  // std::set<Class*> classes;
  std::set<Function*> all_functions;
  for (pugi::xml_document *doc : docs) {
    pugi::xml_node node =doc->document_element();
    pugi::xpath_node_set nodes = node.select_nodes("//class");
    for (auto node : nodes) {
      Class *cl = new Class(node.node());
      classes.insert(cl);
    }
    /**
     * Only top level function (not function inside a class)
     * FIXME not sure if the url is correct
     */
    nodes = node.select_nodes("/unit/function");
    for (auto node : nodes) {
      /**
       * This will have both function, and the method definition: AAA::foo()
       */
      Function *func = new Function(node.node());
      all_functions.insert(func);
    }
  }

  /**
   * Build index for classes.
   */
  // std::map<std::string, Class*> class_m;
  for (Class *cl : classes) {
    class_m[cl->GetName()] = cl;
  }


  /**
   * split all_functions into functions and method_defs
   * Find the Class Method Def Node.
   */
  for (Function *func : all_functions) {
    if (func->IsMethodDef()) {
      std::string method_name = func->GetMethodName();
      std::string method_class = func->GetMethodClass();
      if (class_m.count(method_class) == 1) {
        Method *m = class_m[method_class]->LookUpMethod(method_name);
        if (m) {
          m->SetDefNode(func->GetDefNode());
        }
      }
    } else {
      functions.insert(func);
    }
  }

  /**
   * Build index for functions
   * Map from name to Function*
   */
  // std::map<std::string, Function*> func_m;
  for (Function *func : functions) {
    func_m[func->GetName()] = func;
  }

  /**
   * Build class hierarchy(SetSuper() by lookup super name)
   */
  for (Class *cl : classes) {
    std::string supername = cl->GetSuperName();
    if (!supername.empty() && class_m.count(supername) == 1) {
      cl->SetSuper(class_m[supername]);
    }
  }

  /**
   * For the class, inspect its super class methods.
   * If the method does not appear in its method(by name lookup), add it.
   */
  for (Class *cl : classes) {
    Class *tmp = cl;
    while (tmp->GetSuper()) {
      tmp = tmp->GetSuper();
      for (Method *m : tmp->GetMethods()) {
        if (cl->LookUpMethod(m->GetName()) == NULL) {
          cl->AddMethod(m);
        }
      }
    }
  }

  /**
   * Build index for class methods
   * From method name to a list of classes that have that method.
   * Also take care of class hierarchy
   */
  // std::map<std::string, std::set<Class*> > method_m;
  for (Class *cl : classes) {
    for (Method *m : cl->GetMethods()) {
      method_m[m->GetName()].insert(cl);
    }
  }


  /*******************************
   ** DO the dirty job
   *******************************/
  // if (func_m.count("main") == 1) {
  //   // have main function, go from there.
  //   RA(func_m["main"]);
  // }
  // RA();
  // CHA();
  // RTA();
  RA_lib();
  CHA_lib();
  RTA_lib();
}

void Class::methodHelper(pugi::xml_node pubnode) {
  std::string name = pubnode.name();
  MethodKind kind;
  if (name == "public") kind = MK_Public;
  else if (name == "private") kind = MK_Private;
  else if (name == "protected") kind = MK_Protected;
  else {
    assert(false);
  }
  for (pugi::xml_node func_decl : pubnode.children("function_decl")) {
    std::string specifier = func_decl.child("specifier").child_value();
    bool virt = false;
    if (specifier == "virtual") virt = true;
    Method *method = new Method(
                                this,
                                func_decl.child("name").child_value(),
                                kind,
                                virt
                                );
    m_methods.insert(method);
  }
  for (pugi::xml_node func : pubnode.children("function")) {
    std::string specifier = func.child_value("specifier");
    bool virt = false;
    if (specifier == "virtual") virt = true;
    Method *method = new Method(
                                this,
                                func.child_value("name"),
                                MK_Public,
                                virt
                                );
    method->SetDefNode(func);
    m_methods.insert(method);
  }
  
}

void Class::AddMethod(Method *m) {
  m_methods.insert(m);
  m_method_m[m->GetName()] = m;
}

/**
 * node is <class>
 */
Class::Class(pugi::xml_node node) {
  m_node = node;
  m_name = node.child("name").child_value();
  if (node.child("super")) {
    m_super_name = node.child("super").child("name").child_value();
  }
  // methods
  /**
   * I do not handle overload, i.e. foo(int), foo(bar), just choose one
   */
  for (pugi::xml_node pubnode : node.child("block").children("public")) {
    methodHelper(pubnode);
  }
  for (pugi::xml_node prinode : node.child("block").children("private")) {
    methodHelper(prinode);
  }
  for (pugi::xml_node pronode : node.child("block").children("protected")) {
    methodHelper(pronode);
  }
  /**
   * Build the method map: m_method_m from method name to Method*
   * TODO Now actually I do not want to distinguash the public,private
   */
  for (Method *m : m_methods) {
    m_method_m[m->GetName()] = m;
    if (m->Kind() == MK_Public) {
      m_pub_methods.insert(m);
    }
  }
}

Function::Function(pugi::xml_node node) {
  std::string op = node.child("name").child("operator").child_value();
  if (op.empty()) {
    // regular function
    m_name = node.child("name").child_value();
  } else if (op == "::") {
    // method def
    std::vector<pugi::xml_node> names;
    for (pugi::xml_node n : node.child("name").children("name")) {
      names.push_back(n);
    }
    if (names.size() != 2) return;
    m_method_name = names[1].child_value();
    m_method_class = names[0].child_value();
    m_method_def = true;
  } else {
    return;
  }
  std::string specifier = node.child("specifier").child_value();
  if (specifier == "static") m_static = true;
  m_def_node = node;
}

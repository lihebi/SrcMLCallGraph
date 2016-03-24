#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <vector>
#include <set>
#include <deque>
#include <string>
#include <map>
#include <stack>
#include <fstream>
#include <pugixml.hpp>



class Class;
class Function;
class Method;

class Class {
public:
  Class(pugi::xml_node);
  ~Class() {}
  std::string GetName() {return m_name;}
  void SetSuper(Class* cl) {m_super = cl;}
  std::string GetSuperName() {return m_super_name;}
  Class *GetSuper() {return m_super;}
  void AddMethod(Method *m);
  Method *LookUpMethod(std::string name) {
    if (m_method_m.count(name)) return m_method_m[name];
    else return NULL;
  }
  std::set<Method*> GetMethods() {return m_methods;}
  std::set<Method*> GetPubMethods() {return m_pub_methods;}
private:
  void methodHelper(pugi::xml_node pubnode);
  pugi::xml_node m_node;
  std::string m_name;
  Class* m_super = NULL;
  std::string m_super_name;
  /**
   * public, private, protected methods
   */
  std::set<Method*> m_pub_methods;
  // std::set<std::string> m_pri_methods;
  // std::set<std::string> m_pro_methods;
  /**
   * All Methods
   */
  std::set<Method*> m_methods;
  std::map<std::string, Method*> m_method_m;
};
class CHA {
public:
private:
};

typedef enum {
  MK_Public,
  MK_Private,
  MK_Protected
} MethodKind;

class Method {
public:
  /**
   * Method can only be created by given the class it associated to
   */
  Method(Class *cl, std::string name, MethodKind kind, bool virt) {
    m_class = cl;
    m_name = name;
    m_kind = kind;
    m_virtuality = virt;
  }
  ~Method() {}
  void SetDefNode(pugi::xml_node node) {m_def_node = node;}
  pugi::xml_node GetDefNode() {return m_def_node;}
  std::string GetName() {return m_name;}
  std::string GetLabel() {
    if (m_class==NULL) return "";
    return m_class->GetName() + "::" + m_name;
  }
  MethodKind Kind() {return m_kind;}
private:
  Class *m_class = NULL;
  bool m_virtuality = false;
  std::string m_name;
  MethodKind m_kind;
  pugi::xml_node m_def_node;
};

class Function {
public:
  /**
   * Function can only be created by its defintion node.
   */
  Function(pugi::xml_node);
  ~Function() {}
  std::string GetName() {return m_name;}
  pugi::xml_node GetDefNode() {return m_def_node;}
  bool IsStatic() {return m_static;}
  /**
   * method definition related.
   */
  bool IsMethodDef() {return m_method_def;}
  std::string GetMethodName() {return m_method_name;}
  std::string GetMethodClass() {return m_method_class;}
private:
  std::string m_name;
  pugi::xml_node m_def_node;
  bool m_static = false;
  /**
   * Method Definition related
   */
  bool m_method_def = false;
  std::string m_method_name;
  std::string m_method_class;
};

extern std::set<Class*> classes;
extern std::set<Function*> functions;
extern std::map<std::string, Function*> func_m;
extern std::map<std::string, Class*> class_m;
extern std::map<std::string, std::set<Class*> > method_m;


#endif /* MAIN_H */

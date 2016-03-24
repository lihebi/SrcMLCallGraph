#include "analysis.h"
#include <iostream>
#include "utils.h"

#include <regex>

// std::set<Class*> classes;
// std::set<Function*> functions;
// std::map<std::string, Function*> func_m;
// std::map<std::string, Class*> class_m;
// std::map<std::string, std::set<Class*> > method_m;


std::string get_text(pugi::xml_node node) {
  std::string text;
  if (!node) return "";
  for (pugi::xml_node n : node.children()) {
    if (n.type() == pugi::node_element) {
      text += get_text(n);
    } else {
      text += n.value();
    }
  }
  return text;
}



std::string ra_dot = "digraph {\n";
std::deque<FuncOrMethod> ra_worklist;
std::set<std::string> ra_visited; // include all in worklist

void RA() {
  std::cout <<"RA"  << "\n";
  std::cout <<"Some Statistic"  << "\n";
  if (func_m.count("main") == 1) {
    // have main function, go from there.
    ra_worklist.push_back(FuncOrMethod(func_m["main"]));
    while (!ra_worklist.empty()) {
      FuncOrMethod f = ra_worklist.front();
      ra_worklist.pop_front();
      ra_recur(f);
    }
  }
  ra_dot += "\n}";
  std::cout <<ra_dot  << "\n";
  utils::visualize_dot_graph(ra_dot);
}

void RA_lib() {
  for (Function *func : functions) {
    if (!func->IsStatic()) {
      ra_worklist.push_back(FuncOrMethod(func));
    }
  }
  for (Class *cl : classes) {
    for (Method *m : cl->GetPubMethods()) {
      ra_worklist.push_back(FuncOrMethod(m));
    }
  }
  while (!ra_worklist.empty()) {
    FuncOrMethod f = ra_worklist.front();
    ra_worklist.pop_front();
    ra_recur(f);
  }
  ra_dot += "\n}";
  std::cout <<ra_dot  << "\n";
  utils::visualize_dot_graph(ra_dot);
}

/**
 * RA: from main, all callsite,
 * there will be an edge from main to all the methods with that name.
 */
void ra_recur(FuncOrMethod f) {
  pugi::xml_node func_node;
  std::string master_label;
  if (f.func) {
    func_node = f.func->GetDefNode();
    master_label = f.func->GetName();
  } else if (f.method) {
    func_node = f.method->GetDefNode();
    master_label = f.method->GetLabel();
  } else {
    return;
  }
  pugi::xpath_node_set nodes = func_node.select_nodes(".//call");
  for (auto nn : nodes) {
    // std::string name = node.node().child_value("name");
    pugi::xml_node node = nn.node();
    if (node.child("name").child("operator")) {
      // this is a method
      std::vector<pugi::xml_node> names;
      for (pugi::xml_node node : node.child("name").children("name")) {
        names.push_back(node);
      }
      // only consider a->b and a.b
      // May need to make sure the <operator> is . or ->
      if (names.size() != 2) continue;
      std::string class_name = names[0].child_value();
      std::string method_name = names[1].child_value();
      // resolve this method call
      // TODO
      if (method_m.count(method_name) == 1) {
        std::set<Class*> classes = method_m[method_name];
        for (Class* cl : classes) {
          Method *m = cl->LookUpMethod(method_name);
          std::string label = m->GetLabel();
          if (ra_visited.count(label) != 1) {
            ra_dot += "\""+master_label+"\"" + " -> " + "\""+label+"\"" + "\n";
            ra_visited.insert(label);
            ra_worklist.push_back(FuncOrMethod(m));
          }
        }
      }
    } else {
      std::string func_name = node.child_value("name");
      // resolve this function call
      if (func_m.count(func_name) == 1) {
        Function *f = func_m[func_name];
        std::string label = f->GetName();
        if (ra_visited.count(label) != 1) {
          // ra_dot += master_label + " -> " + label + "\n";
          ra_dot += "\""+master_label+"\"" + " -> " + "\""+label+"\"" + "\n";
          ra_visited.insert(label);
          ra_worklist.push_back(FuncOrMethod(f));
        }
      }
    }
  }
}

/*******************************
 ** CHA
 *******************************/

// from main, all callsite =e.m()=, static look up the type of =e=.
// There will be an edge from main to all the methods with the name m, but in a class of =e='s subtype.

// std::set<Class*> classes;
// std::set<Function*> functions;
// std::map<std::string, Function*> func_m;
// std::map<std::string, Class*> class_m;
// std::map<std::string, std::set<Class*> > method_m;

std::string decl_get_name(pugi::xml_node node) {
  // this is an array
  if (node.child("name").child("name")) {
    return node.child("name").child_value("name");
  }
  // has a <decl><name>xxx</name></decl>
  return node.child_value("name");
}
std::string decl_get_type(pugi::xml_node node) {
  return node.child("type").child("name").child_value();
}

std::string resolve_var(pugi::xml_node node, std::string var) {
  pugi::xml_node func;
  std::string ret;
  while (node.parent()) {
    node = node.parent();
    if (strcmp(node.name(), "function") == 0) {
      func = node;
    }
  }
  if (!func) return "";
  pugi::xpath_node_set decls = func.select_nodes(".//decl");
  for (auto d : decls) {
    pugi::xml_node decl = d.node();
    if (decl_get_name(decl) == var) {
      ret = decl_get_type(decl);
      break;
    }
  }
  return ret;
}



std::string cha_dot = "digraph {\n";
std::deque<FuncOrMethod> cha_worklist;
std::set<std::string> cha_visited; // include all in worklist

void CHA() {
  if (func_m.count("main") == 1) {
    // have main function, go from there.
    cha_worklist.push_back(FuncOrMethod(func_m["main"]));
    while (!cha_worklist.empty()) {
      FuncOrMethod f = cha_worklist.front();
      cha_worklist.pop_front();
      cha_recur(f);
    }
  }
  cha_dot += "\n}";
  utils::visualize_dot_graph(cha_dot);
}

void CHA_lib() {
  for (Function *func : functions) {
    if (!func->IsStatic()) {
      cha_worklist.push_back(FuncOrMethod(func));
    }
  }
  for (Class *cl : classes) {
    for (Method *m : cl->GetPubMethods()) {
      cha_worklist.push_back(FuncOrMethod(m));
    }
  }
  while (!cha_worklist.empty()) {
    FuncOrMethod f = cha_worklist.front();
    cha_worklist.pop_front();
    cha_recur(f);
  }
  cha_dot += "\n}";
  std::cout <<cha_dot  << "\n";
  utils::visualize_dot_graph(cha_dot);
}

bool is_super(Class *sup, Class *sub) {
  while (sub) {
    if (sub == sup) return true;
    sub = sub->GetSuper();
  }
  return false;
}

void cha_recur(FuncOrMethod f) {
  pugi::xml_node func_node;
  std::string master_label;
  if (f.func) {
    func_node = f.func->GetDefNode();
    master_label = f.func->GetName();
  } else if (f.method) {
    func_node = f.method->GetDefNode();
    master_label = f.method->GetLabel();
  } else {
    return;
  }
  pugi::xpath_node_set nodes = func_node.select_nodes(".//call");
  for (auto nn : nodes) {
    // std::string name = node.node().child_value("name");
    pugi::xml_node node = nn.node();
    if (node.child("name").child("operator")) {
      // this is a method
      std::vector<pugi::xml_node> names;
      for (pugi::xml_node node : node.child("name").children("name")) {
        names.push_back(node);
      }
      // only consider a->b and a.b
      // May need to make sure the <opechator> is . or ->
      if (names.size() != 2) continue;
      // this class name is actually a variable name.
      // Resolve this variable name to refine the search space
      std::string class_name = names[0].child_value();
      std::string type_class = resolve_var(names[0], class_name);
      std::cout <<class_name  << "\n";
      std::cout <<type_class  << "\n";
      
      if (class_m.count(type_class) == 0) {
        continue;
      }
      // all the method should belong to its subclass
      Class *target = class_m[type_class];
      std::string method_name = names[1].child_value();
      // resolve this method call
      // TODO
      if (method_m.count(method_name) == 1) {
        std::set<Class*> classes = method_m[method_name];
        for (Class* cl : classes) {
          // the KEY of CHA
          if (!is_super(target, cl)) continue;
          Method *m = cl->LookUpMethod(method_name);
          std::string label = m->GetLabel();
          if (cha_visited.count(label) != 1) {
            cha_dot += "\""+master_label+"\"" + " -> " + "\""+label+"\"" + "\n";
            cha_visited.insert(label);
            cha_worklist.push_back(FuncOrMethod(m));
          }
        }
      }
    } else {
      std::string func_name = node.child_value("name");
      // resolve this function call
      if (func_m.count(func_name) == 1) {
        Function *f = func_m[func_name];
        std::string label = f->GetName();
        if (cha_visited.count(label) != 1) {
          // cha_dot += master_label + " -> " + label + "\n";
          cha_dot += "\""+master_label+"\"" + " -> " + "\""+label+"\"" + "\n";
          cha_visited.insert(label);
          cha_worklist.push_back(FuncOrMethod(f));
        }
      }
    }
  }
}


/*******************************
 ** RTA
 *******************************/

// Get a set of class names instantiated so far. As with CHA, but the class must be in the set.

// std::set<Class*> classes;
// std::set<Function*> functions;
// std::map<std::string, Function*> func_m;
// std::map<std::string, Class*> class_m;
// std::map<std::string, std::set<Class*> > method_m;

std::set<std::string> get_all_newd_class(std::string code) {
  std::set<std::string> ret;
  std::regex new_reg("\\bnew\\b\\s*\\b([a-zA-Z_1-9:]*)\\b");
  std::regex_search(code, new_reg);

  std::smatch new_match;
  std::regex_search(code, new_match, new_reg);
  for (size_t i=0;i<new_match.size();i++) {
    // the first is entire match
    // the followings are () matches
    new_match[i]; // string
    std::cout <<new_match[i]  << "\n";
    ret.insert(new_match[i]);
  }
  return ret;
}

std::string rta_dot = "digraph {\n";
std::deque<FuncOrMethod> rta_worklist;
std::set<std::string> rta_visited; // include all in worklist
std::set<std::string> rta_instantiated;

void RTA() {
  std::cout <<"RTA"  << "\n";
  std::cout <<"Some Statistic"  << "\n";
  if (func_m.count("main") == 1) {
    // have main function, go from there.
    rta_worklist.push_back(FuncOrMethod(func_m["main"]));
    while (!rta_worklist.empty()) {
      FuncOrMethod f = rta_worklist.front();
      rta_worklist.pop_front();
      rta_recur(f);
    }
  }
  rta_dot += "\n}";
  std::cout <<rta_dot  << "\n";
  utils::visualize_dot_graph(rta_dot);
}

void RTA_lib() {
  for (Function *func : functions) {
    if (!func->IsStatic()) {
      rta_worklist.push_back(FuncOrMethod(func));
    }
  }
  for (Class *cl : classes) {
    for (Method *m : cl->GetPubMethods()) {
      rta_worklist.push_back(FuncOrMethod(m));
    }
  }
  while (!rta_worklist.empty()) {
    FuncOrMethod f = rta_worklist.front();
    rta_worklist.pop_front();
    rta_recur(f);
  }
  rta_dot += "\n}";
  std::cout <<rta_dot  << "\n";
  utils::visualize_dot_graph(rta_dot);
}
void rta_recur(FuncOrMethod f) {
  /**
   * This is flow-insensitive RTA
   */
  pugi::xml_node func_node;
  std::string master_label;
  if (f.func) {
    func_node = f.func->GetDefNode();
    master_label = f.func->GetName();
  } else if (f.method) {
    func_node = f.method->GetDefNode();
    master_label = f.method->GetLabel();
  } else {
    return;
  }
  /**
   * Get the instantiated class
   */
  std::string func_code = get_text(func_node);
  std::set<std::string> newd_class = get_all_newd_class(func_code);
  rta_instantiated.insert(newd_class.begin(), newd_class.end());
  pugi::xpath_node_set nodes = func_node.select_nodes(".//call");
  for (auto nn : nodes) {
    // std::string name = node.node().child_value("name");
    pugi::xml_node node = nn.node();
    if (node.child("name").child("operator")) {
      // this is a method
      std::vector<pugi::xml_node> names;
      for (pugi::xml_node node : node.child("name").children("name")) {
        names.push_back(node);
      }
      // only consider a->b and a.b
      // May need to make sure the <opertator> is . or ->
      if (names.size() != 2) continue;
      std::string method_name = names[1].child_value();
      // this class name is actually a variable name.
      // Resolve this variable name to refine the search space
      std::string class_name = names[0].child_value();
      std::string type_class = resolve_var(names[0], class_name);
      std::cout <<class_name  << "\n";
      std::cout <<type_class  << "\n";
      
      if (class_m.count(type_class) == 0) {
        continue;
      }
      // all the method should belong to its subclass
      Class *target = class_m[type_class];
      // resolve this method call
      // TODO
      if (method_m.count(method_name) == 1) {
        std::set<Class*> classes = method_m[method_name];
        for (Class* cl : classes) {
          /**
           * Class must be instantiated.
           */
          if (rta_instantiated.count(cl->GetName()) == 0) continue;
          // the KEY of CHA
          if (!is_super(target, cl)) continue;
          Method *m = cl->LookUpMethod(method_name);
          std::string label = m->GetLabel();
          if (rta_visited.count(label) != 1) {
            rta_dot += "\""+master_label+"\"" + " -> " + "\""+label+"\"" + "\n";
            rta_visited.insert(label);
            rta_worklist.push_back(FuncOrMethod(m));
          }
        }
      }
    } else {
      std::string func_name = node.child_value("name");
      // resolve this function call
      if (func_m.count(func_name) == 1) {
        Function *f = func_m[func_name];
        std::string label = f->GetName();
        if (rta_visited.count(label) != 1) {
          // rta_dot += master_label + " -> " + label + "\n";
          rta_dot += "\""+master_label+"\"" + " -> " + "\""+label+"\"" + "\n";
          rta_visited.insert(label);
          rta_worklist.push_back(FuncOrMethod(f));
        }
      }
    }
  }
}

#include <iostream>

class A {
public:
  virtual void foo() {}
};

class B : public A{
public:
  virtual void foo() {}
private:
};

class C : public B {
public:
  virtual void foo();
};

class D {
public:
  void foo();
};

int main() {
  A *a = new A();
  a->foo();
}

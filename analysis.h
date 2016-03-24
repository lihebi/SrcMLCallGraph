#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "main.h"
struct FuncOrMethod {
  FuncOrMethod(Function *f) : func(f) {}
  FuncOrMethod(Method *m) : method(m) {}
  Function *func = NULL;
  Method *method = NULL;
};

void RA();
void RA_lib();
void ra_recur(FuncOrMethod f);

void RTA();
void RTA_lib();
void rta_recur(FuncOrMethod f);

void CHA();
void CHA_lib();
void cha_recur(FuncOrMethod f);
#endif /* ANALYSIS_H */

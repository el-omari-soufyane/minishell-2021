/*
  Projet minishell - Licence 3 Info - PSI 2021
 
  Nom : EL OMARI
  Prénom : Soufyane
  Num. étudiant : 22113404

  Nom : CHOUKRI
  Prénom : Marouane
  Num. étudiant : 

  Nom : DOUMI
  Prénom : Saloua
  Num. étudiant : 

  Groupe de projet : 20
  Date : 21/11/2021
 
  Gestion des commandes internes du minishell (headers).
 
 */

#ifndef BUILTIN_H
#define BUILTIN_H

#include "processus.h"

int is_builtin(const char* cmd);
int builtin(process_t* proc);

int cd(const char* path, int fderr);
int export(const char* var, const char* value, int fderr);
int unset(const char *var, int fderr);
int exit_shell(int ret, int fdout);
#endif

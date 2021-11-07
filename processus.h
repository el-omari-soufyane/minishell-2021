/*
  Projet minishell - Licence 3 Info - PSI 2021
 
  Nom :
  Prénom :
  Num. étudiant :
  Groupe de projet :
  Date :
 
  Gestion des processus (headers).
 
 */

#ifndef PROCESSUS_H
#define PROCESSUS_H

#include <unistd.h>

typedef struct process_t {
  pid_t pid; // PID du nouveau processus
  char* path; // Chemin de l'exécutable
  char** argv; // Tableau des arguments
  int stdin, stdout, stderr; // Descripteurs I/O
  int fdclose[2]; // Éventuels descripteurs à fermer
  int status; // Status du processus (pour waitpid())
  int bg;  // Lancer la commande en arrière plan ?
  
  struct process_t* next; // Prochaine commande inconditionnelle
  struct process_t* next_failure; // Prochaine en cas d'échec
  struct process_t* next_success; // Prochaine en cas de succés
} process_t;

int init_process(process_t* proc);
int set_env(process_t* proc);
int launch_cmd(process_t* proc);

#endif

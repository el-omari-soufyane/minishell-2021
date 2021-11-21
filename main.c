/*
  Projet minishell - Licence 3 Info - PSI 2021
 
  Nom :
  Prénom :
  Num. étudiant :
  Groupe de projet :
  Date :
 
  Parsing de la ligne de commandes utilisateur (implémentation).
 
 */

#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "processus.h"

#define MAX_LINE_SIZE 1024
#define MAX_CMD_SIZE 256

int main(int argc, char *argv[])
{
  char path[MAX_LINE_SIZE];
  char line[MAX_LINE_SIZE];
  char *cmdline[MAX_CMD_SIZE];
  process_t cmds[MAX_CMD_SIZE];

  while (1)
  {
    getcwd(path, MAX_LINE_SIZE);
    // (r�-)_pInitialiser les variables/structures
    init_process(cmds);
    // Affichage d'une invite de commande
    printf("mini@shell:%s$ ", path);
    // Lecture d'une ligne de commandes
    fgets(line, MAX_LINE_SIZE, stdin);
    if(strcmp(line, "\n") == 0) continue;

    // "Nettoyage" de la ligne de commandes
    trim(line);
    clean(line);
    // D�coupage en "tokens"
    tokenize(line, cmdline);

    // Parsing de la ligne pour remplir les structures
    // de cmds.
    parse_cmd(cmdline, cmds);

    // Lancement des commandes dans l'ordre attendu,
    // avec les �ventuelles redirections et conditionnements
    // d'ex�cution.
    launch_cmd(cmds);
    //toString(cmds); break;
  }
  return -1;
}

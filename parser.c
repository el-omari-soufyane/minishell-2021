/*
  Projet minishell - Licence 3 Info - PSI 2021
 
  Nom :
  Prénom :
  Num. étudiant :
  Groupe de projet :
  Date :
 
  Parsing de la ligne de commandes utilisateur (implémentation).
 
 */

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>

#include "parser.h"
#include "processus.h"

/*
  Suppression des espaces en début et fin de chaîne.
      Ex : "   chaîne   " => "chaîne"
 
  str : chaîne à modifier
 
  Retourne 0 ou un code d'erreur.
 */
int trim(char* str) {
  assert(str!=NULL);
  int debut = 0;
  int fin = strlen(str);
  for(int i = fin - 1; str[i] == ' ' || str[i] == '\t' || str[i] == '\n'; i++) {
    fin = fin - 1;
  }
  str[fin] = '\0';
  for(int i = 0; str[i] == ' ' || str[i] == '\t' || str[i] == '\n'; i++) {
    debut = debut + 1;
  }
  memmove(str, str + debut, 1 + strlen(str) -  debut);
  return 0;
}

/*
  Suppression des doublons d'espaces.
      Ex : "cmd1   -arg  ;   cmd2  <  input"
        => "cmd1 -arg ; cmd2 < input"
 
  str: chaîne à modifier
 
  Retourne 0 ou un code d'erreur.
 */
int clean(char* str) {
  assert(str!=NULL);
  int count = 0;
  for(int i=0; i<strlen(str); i++) {
    int j = i;
    while(str[j] == ' ' || str[j] == '\t') {
      j++;
    }
    if(j > i+1) memmove(str+(i+1), str+j, 1 + strlen(str) - j);
  }
  return 0;
}

/*
  Découpage de la chaîne en ses éléments.
    Ex : "ls -l | grep ^a ; cat"
      => {"ls", "-l", "|", "grep", "^a", ";", "cat", NULL}
  str : chaîne à découper (peut être modifiée)
  tokens : tableau des pointeurs sur les éléments de la
           chaîne. Terminé par NULL.
 
  Retourne 0 ou un code d'erreur.
 */
int tokenize(char* str, char* tokens[]) {
  assert(str!=NULL);
  assert(tokens!=NULL);
  int position = 0;
  char* mot;

  mot = strtok(str, " ");
  tokens[position] = mot;
  position++;
  
  while(mot != NULL) {
    mot = strtok(NULL, " ");
    tokens[position] = mot;
    position = position + 1;
  }

  tokens[position] = NULL;
  return 0;
}

/*
  S'agit-il d'un mot réservé du shell ?
  Les mots réservés sont par exemple :
    ";", "&", "<", "2>", "||", ...
 
  tok : la chaîne à tester
 
  Retourne 1 s'il s'agit d'un mot réservé, 0 sinon.
 */
int is_reserved(const char* tok) {
  assert(tok!=NULL);
  const char* reserved[] = {";", "&", "&&", "||", "|", ">", "<", ">>", "2>", "2>>", "<<"};
    int size = sizeof reserved / sizeof *reserved;
    for(int i=0; i<size; i++) {
        if(strcmp(tok, reserved[i]) == 0) return 1;
    }
    return 0;
}

/*
  Remplit le tableau de commandes en fonction du contenu
  de tokens.
    Ex : {"ls", "-l", "|", "grep", "^a", NULL} =>
         {{path = "ls",
           argv = {"ls", "-l", NULL},
           bg = 1,
           ...},
          {path = "grep",
           argv = {"grep", "^a", NULL},
           bg = 0,
           ...}}
  tokens : le tableau des éléments de la ligne de
           commandes (peut être modifié)
  commands : le tableau dans lequel sont stockés les
             différentes structures représentant le
             commandes.
  Retourne 0 ou un code d'erreur.
 */
int parse_cmd(char* tokens[], process_t* commands) {
    assert(tokens!=NULL);
    assert(commands!=NULL);
    
    int position = 0;
    int commandNumber = 0;
    for(int i=0; tokens[i] != NULL; i++) {
        if(is_reserved(tokens[i]) == 0 && tokens[i+1] != NULL) continue;
        int end = is_reserved(tokens[i]) == 0 ? i+1 : i;
        int argc = position;
        int count = 0;
        char** argv = malloc(sizeof(char*)*(end-argc));
        for(argc; argc < end; argc++) {
            argv[count] = tokens[argc];
            count = count + 1;
        }
        commands[commandNumber].path = argv[0];
        commands[commandNumber].argv = argv;
        commandNumber = commandNumber + 1;
        position = i + 1;
    }
    return 0;
}

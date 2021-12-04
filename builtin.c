/*
  Projet minishell - Licence 3 Info - PSI 2021
 
  Nom : EL OMARI
  Prénom : Soufyane
  Num. étudiant : 22113404

  Nom : CHOUKRI
  Prénom : Marouane
  Num. étudiant : 22113416

  Nom : DOUMI
  Prénom : Saloua
  Num. étudiant : 22112260

  Groupe de projet : 20
  Date : 03/12/2021
 
  Gestion des commandes internes du minishell (implémentation).
 
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include "builtin.h"
#include "processus.h"

/*
 La chaîne passée représente-t-elle une commande
 interne ?
 
 cmd : chaîne de caractères.
 
 Retourne 1 si la chaîne passée désigne une
 commande interne (cd, exit, ...)
 */
int is_builtin(const char *cmd)
{
  assert(cmd != NULL);
  char *builtin_list[] = {"cd", "export", "unset", "exit", NULL};
  int i = 0;
  while (builtin_list[i] != NULL)
  {
    /* On compare la commande entrée avec les commandes
       builtin qu'on a implémenté.
    */
    if (strcmp(builtin_list[i], cmd) == 0)
    {
      return 1;
    }
    i++;
  }
  return 0;
}

/*
  Exécute la commande interne
 
  proc : process_t dont le contenu a été initialisé
         au moment du parsing
 
  Retourne 0 si tout s'est bien passé, la valeur de
  retour de la commande ou -1 si la commande n'est
  pas reconnue.
 */

int builtin(process_t *proc)
{
  assert(proc != NULL);
  /* 
    On vérifie si la commande est builtin,
    Si oui : On exécute la fonction associée à cette commande
    Sinon : On retourne -1
  */
  if (is_builtin(proc->argv[0]))
  {
    if (strcmp(proc->argv[0], "cd") == 0)
    {
      return cd(proc->argv[1], proc->stderr);
    }
    else if (strcmp(proc->argv[0], "export") == 0)
    {
      /* 
        On fait la décomposition de variable d'environnement
        si l'utilisateur a exporté une variable avec une valeur
        Exemple : export VAR=PSI2021
          -> Dans ce cas, on doit décomposer la chaîne 
            en deux variables

        Exemple : export VAR  
          -> Dans ce cas, on prend le nom de la variable et on envoie 
          une chaîne vide comme deuxième argument de la fonction "export"
      */
      if (strchr(proc->argv[1], '=') != NULL)
      {
        int position = 0;
        char *mot;
        char *val[2];

        mot = strtok(proc->argv[1], "=");
        val[position] = mot;
        position++;

        while (mot != NULL)
        {
          mot = strtok(NULL, "=");
          val[position] = mot;
          position = position + 1;
        }
        return export(val[0], val[1], proc->stderr);
      }
      else
      {
        return export(proc->argv[1], "", proc->stderr);
      }
    }
    else if (strcmp(proc->argv[0], "unset") == 0)
    {
      return unset(proc->argv[1], proc->stderr);
    }
    else if (strcmp(proc->argv[0], "exit") == 0)
    {
      return exit_shell(0, proc->stdout);
    }
  }
  return -1;
}

/*
  Change directory : change le répertoire de travail
  courant du minishell.

  path : le chemin vers lequel déplacer le CWD
  fderr : le descripteur de la sortie d'erreur pour
          affichage éventuel (erreur du cd)
  
  Retourne le code de retour de l'appel système.
 */

int cd(const char *path, int fderr)
{
  assert(path != NULL);
  int changed = chdir(path);
  /* 
    La fonction de "chdir" permet de changer 
    le répertoire de travail et renvoie 0 
    en cas de succès ou un code d'erreur en cas
    d'erreur.

    Si la fonction chdir renvoie 0, on change
    le path et on renvoie 0.
    Sinon, on écrit un message d'erreur dans 
    le descripteur de la sortie d'erreur.
  */
  if (changed == 0)
  {
    char new_path[1024];
    getcwd(new_path, sizeof(new_path));
    return 0;
  }
  char *message = "Chemin invalide !\n";
  write(fderr, message, strlen(message));
  close(fderr);
  return changed;
}

/*
  Ajout/modification d'une variable d'environnement.
 
  var : nom de la variable
  value : valeur à lui donner
  fderr : le descripteur de la sortie d'erreur pour
          affichage éventuel
 
  Retourne le code de retour de l'appel système.
 */

int export(const char *var, const char *value, int fderr)
{
  assert(var != NULL);
  assert(value != NULL);
  /* 
    On utilise la fonction setenv pour ajouter
    et/ou modifier une variable d'environnement.
    On met le troisième argument à 1 pour modifier
    la variable s'il existe déjà.

    En cas d'erreur, un message d'erreur est écrit
    dans le descripteur de la sortie d'erreur.
  */
  int env = setenv(var, value, 1);
  printf("%s=%s\n", var, getenv(var));
  if (env != 0)
  {
    char *message = "Erreur d'exportation de variable\n";
    write(fderr, message, strlen(message));
    close(fderr);
  }
  return env;
}

/*
  Effacer une variable d'environnement.
 
  var : nom de la variable
  fderr : le descripteur de la sortie d'erreur pour
          affichage éventuel
 
  Retourne le code de retour de l'appel système.
 */

int unset(const char *var, int fderr)
{
  assert(var != NULL);
  /* 
    La fonction unsetenv permet d'effacer une variable
    d'environnement.
    En cas d'erreur, un message d'erreur est écrit
    dans le descripteur de la sortie d'erreur.
  */
  int varUnset = unsetenv(var);
  if (varUnset != 0)
  {
    char *message = "Erreur de redéfinition de variable\n";
    write(fderr, message, strlen(message));
    close(fderr);
  }
  return varUnset;
}

/*
  Quitter le minishell
 
  ret : code de retour du minishell
  fdout : descripteur de la sortie standard pour
          l'affichage éventuel d'un message de sortie
 
  Retourne un code d'erreur en cas d'échec.
 */

int exit_shell(int ret, int fdout)
{
  exit(ret);
  return ret;
}

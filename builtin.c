/*
  Projet minishell - Licence 3 Info - PSI 2021
 
  Nom :
  Prénom :
  Num. étudiant :
  Groupe de projet :
  Date :
 
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
  if (is_builtin(proc->argv[0]))
  {
    if (strcmp(proc->argv[0], "cd") == 0)
    {
      if (proc->argv[1] == NULL)
        return -1;
      return cd(proc->argv[1], proc->stderr);
    }
    else if (strcmp(proc->argv[0], "export") == 0)
    {
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
  if (changed == 0)
  {
    char new_path[1024];
    getcwd(new_path, sizeof(new_path));
    return 0;
  }
  char *message = "Chemin invalide !";
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
  int env = setenv(var, value, 1);
  printf("%s=%s\n", var, getenv(var));
  if (env != 0)
  {
    char *message = "Erreur d'exportation de variable";
    write(fderr, message, strlen(message));
    close(fderr);
  }
  return env;
}

int unset(const char *var, int fderr)
{
  assert(var != NULL);
  int varUnset = unsetenv(var);
  if (varUnset != 0)
  {
    char *message = "Erreur de redéfinition de variable";
    write(fderr, message, strlen(message));
    close(fderr);
  }
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
  return ret;
}

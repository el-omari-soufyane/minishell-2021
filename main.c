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
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE_SIZE 1024
#define MAX_CMD_SIZE 256

typedef struct process_t
{
  pid_t pid;                 // PID du nouveau processus
  char *path;                // Chemin de l'exécutable
  char **argv;               // Tableau des arguments
  int stdin, stdout, stderr; // Descripteurs I/O
  int fdclose[2];            // Éventuels descripteurs à fermer
  int status;                // Status du processus (pour waitpid())
  int bg;                    // Lancer la commande en arrière plan ?

  struct process_t *next;         // Prochaine commande inconditionnelle
  struct process_t *next_failure; // Prochaine en cas d'échec
  struct process_t *next_success; // Prochaine en cas de succés
} process_t;

/*
  Suppression des espaces en début et fin de chaîne.
      Ex : "   chaîne   " => "chaîne"
 
  str : chaîne à modifier
 
  Retourne 0 ou un code d'erreur.
 */
int trim(char *str)
{
  assert(str != NULL);
  int debut = 0;
  int fin = strlen(str);
  for (int i = fin - 1; str[i] == ' ' || str[i] == '\t' || str[i] == '\n'; i++)
  {
    fin = fin - 1;
  }
  str[fin] = '\0';
  for (int i = 0; str[i] == ' ' || str[i] == '\t' || str[i] == '\n'; i++)
  {
    debut = debut + 1;
  }
  memmove(str, str + debut, 1 + strlen(str) - debut);
  return 0;
}

/*
  Suppression des doublons d'espaces.
      Ex : "cmd1   -arg  ;   cmd2  <  input"
        => "cmd1 -arg ; cmd2 < input"
 
  str: chaîne à modifier
 
  Retourne 0 ou un code d'erreur.
 */
int clean(char *str)
{
  assert(str != NULL);
  int count = 0;
  for (int i = 0; i < strlen(str); i++)
  {
    int j = i;
    while (str[j] == ' ' || str[j] == '\t')
    {
      j++;
    }
    if (j > i + 1)
      memmove(str + (i + 1), str + j, 1 + strlen(str) - j);
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
int tokenize(char *str, char *tokens[])
{
  assert(str != NULL);
  assert(tokens != NULL);
  int position = 0;
  char *mot;

  mot = strtok(str, " ");
  tokens[position] = mot;
  position++;

  while (mot != NULL)
  {
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
int is_reserved(const char *tok)
{
  assert(tok != NULL);
  const char *reserved[] = {";", "&", "&&", "||", "|", ">", "<", ">>", "2>", "2>>", "<<"};
  int size = sizeof reserved / sizeof *reserved;
  for (int i = 0; i < size; i++)
  {
    if (strcmp(tok, reserved[i]) == 0)
      return 1;
  }
  return 0;
}

void toString(process_t *proc)
{
  for (int i = 0; i < MAX_CMD_SIZE; i++)
  {
    printf("\nPID : %d || path : %s || argv = {", proc[i].pid, proc[i].path);
    int ind = 0;
    for (ind; proc[i].argv[ind] != NULL; ind++)
    {
      printf("%s, ", proc[i].argv[ind]);
    }
    printf("%s}", proc[i].argv[ind]);
    printf(" || next = %s", proc[i].next->path);
  }
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

int parse_cmd(char *tokens[], process_t *commands)
{
  assert(tokens != NULL);
  assert(commands != NULL);

  int position = 0;
  int commandNumber = 0;

  for (int i = 0; tokens[i] != NULL; i++)
  {
    if (is_reserved(tokens[i]) == 0)
    {
      commands[commandNumber].path = tokens[position];
      commands[commandNumber].argv = &tokens[position];
    }
    if (strcmp(tokens[i], ";") == 0)
    {
      if (tokens[i + 1] != NULL) // Si jamais l’utilisateur a terminé la ligne avec un ;
        commands[commandNumber].next = &commands[commandNumber] + 1;
      tokens[i] = NULL;
      ++commandNumber;
      position = i + 1;
      continue;
    }
    if (strcmp(tokens[i], ">") == 0)
    {
      commands[commandNumber].stdout = open(tokens[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0644);
      if (commands[commandNumber].stdout == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("open");
        // Rien ne sera exécuté
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], ">>") == 0)
    {
      commands[commandNumber].stdout = open(tokens[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);
      if (commands[commandNumber].stdout == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("open");
        // Rien ne sera exécuté
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], "2>") == 0)
    {
      commands[commandNumber].stderr = open(tokens[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0644);
      if (commands[commandNumber].stderr == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("open");
        // Rien ne sera exécuté
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], "2>>") == 0)
    {
      commands[commandNumber].stderr = open(tokens[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);
      if (commands[commandNumber].stderr == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("open");
        // Rien ne sera exécuté
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], "|") == 0)
    {
      commands[commandNumber].bg = 1;
      tokens[i] = NULL;
      ++commandNumber;
      position = i + 1;
      continue;
    }
    if (tokens[i + 1] == NULL) // Si jamais l’utilisateur a terminé la ligne avec un ;
      commands[commandNumber].next = &commands[commandNumber] + 1;
  }
  return 0;
}

/*
  Lancer la commande en fonction des attributs de
  la structure et initialiser les champs manquants.
 
  On crée un nouveau processus, on détourne
  éventuellement les entrée/sorties.
  On conserve le PID dans la structure du processus
  appelant (le minishell).
  On attend si la commande est lancée en "avant-plan"
  et on initialise le code de retour.
  On rend la main immédiatement sinon.
 
  proc : un pointeur sur la structure contenant les
         informations pour l'exécution de la commande.
 
  Retourne 0 ou un code d'erreur.
  
 */
int launch_cmd(process_t *proc)
{
  assert(proc != NULL);
  int status;
  int saved_stdout, saved_stderr;
  do
  {
    saved_stdout = dup(1);
    saved_stderr = dup(2);
    status = 0;
    pipe(proc->fdclose);
    proc->pid = fork();
    if (proc->pid == 0)
    {
      if (proc->bg == 1)
      {
        close(proc->fdclose[0]);
        dup2(proc->fdclose[1], 1);
        close(proc->fdclose[1]);
      }
      if (proc->stdout != 0)
      {
        dup2(proc->stdout, 1);
        close(proc->stdout);
      }
      if (proc->stderr != 0)
      {
        dup2(proc->stderr, 2);
        close(proc->stderr);
      }
      execvp(*proc->argv, proc->argv);
      perror(*proc->argv);
      exit(1);
    }
    else
    {
      wait(&status);
      if (proc->bg == 1)
      {
        close(proc->fdclose[1]);
        dup2(proc->fdclose[0], 0);
        close(proc->fdclose[0]);
        proc++;
        execvp(*proc->argv, proc->argv);
        perror("pipe");
        exit(1);
      }
      if (proc->stdout != 0)
      {
        close(proc->stdout);
      }
      if (proc->stderr != 0)
      {
        close(proc->stderr);
      }
      dup2(saved_stdout, 1);
      close(saved_stdout);
      dup2(saved_stderr, 2);
      close(saved_stderr);
      proc++;
    }
  } while (proc->next != NULL);
}

/*
  Initialiser une structure process_t avec les
  valeurs par défaut.
 
  proc : un pointeur sur la structure à initialiser
 
  Retourne 0 ou un code d'erreur.
 */
int init_process(process_t *proc)
{
  assert(proc != NULL);
  //assert(check_zero(proc, sizeof(*proc))==0);
  do
  {
    proc->path = NULL;
    proc->argv = NULL;
    proc->stdin = 0;
    proc->next = NULL;
    proc++;
  } while (proc->next != NULL);
}

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
    scanf("%[^\n]%*c", line);
    if (strcmp(line, "") == 0)
      continue;

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

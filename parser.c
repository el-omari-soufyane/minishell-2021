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
int trim(char *str)
{
  assert(str != NULL);
  /* 
    On commance par la fin de la chaîne,
    tant qu'il y a un espace/tabulation/retour à la ligne,
    on incrémente le nombre d'espaces jusqu'on arrivera à
    un caractère.
    On remplace la position où on arrêté et
    on l'affecte par '\0' 
  */
  int debut = 0;
  int fin = strlen(str);
  for (int i = fin - 1; str[i] == ' ' || str[i] == '\t' || str[i] == '\n'; i++)
  {
    fin = fin - 1;
  }
  str[fin] = '\0';

  /* 
    On accumule le nombre d'espaces depuis le début de la chaîne,
    puis on utilise la fonction memmove pour déplacer la chaîne
    depuis la position où on n'a pas trouvé un espace.
  */

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
  /* 
    En utilisant le même principe de la fonction trim,
    mais cette fois, à chaque caractère, si il est un espace
    on boucle tant qu'il y a d'autres espaces après
    pour les éliminer
  */
  for (int i = 0; i < (int) strlen(str); i++)
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

  /* 
    En utilisant la fonction "strtok", qui nous permet
    de séparer une chaîne de caractèrs par un délimiteur
    qu'on lui passe en argument.

    Après utiliser les fonctions "trim" et "clean",
    on sait qu'il existe un seul espace entre les mots
    afin de les séparer par "strtok" et les stocker
    dans le tableau des tokens.
  */

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
  const char *reserved[] = {";", "&&", "||", "|", ">", "<", ">>", "2>", "2>>"};
  int size = sizeof reserved / sizeof *reserved;
  for (int i = 0; i < size; i++)
  {
    if (strcmp(tok, reserved[i]) == 0)
      return 1;
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
int parse_cmd(char *tokens[], process_t *commands)
{
  assert(tokens != NULL);
  assert(commands != NULL);

  int position = 0;
  int commandNumber = 0;

  for (int i = 0; tokens[i] != NULL; i++)
  {
    /* Si le token actuel est un mot réservé
      on doit ajouter la commande avant avec leurs arguments.
      Lorsqu'on termine le traitement de chaque mot réservé,
      on incrémente le nombre de commandes et on change la
      position du dernière commande vers la nouvelle.
    */
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
    if (strcmp(tokens[i], "<") == 0)
    {
      /* On utilise le descripteur de l'entrée standard
      pour lire les données entrantes.
      */
      commands[commandNumber].stdin = open(tokens[i + 1], O_RDONLY);
      if (commands[commandNumber].stdin == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("Erreur de stdin");
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], ">") == 0)
    {
      /* On utilise le descripteur de la sortie standard
        pour rediriger la sortie standard vers ce fichier.
      */
      commands[commandNumber].stdout = open(tokens[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0644);
      if (commands[commandNumber].stdout == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("Erreur de stdout");
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], ">>") == 0)
    {
      /* On utilise le descripteur de la sortie standard
        pour rediriger en ajoutant la sortie standard vers
        la fin du fichier sans écraser son contenu.
      */
      commands[commandNumber].stdout = open(tokens[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);
      if (commands[commandNumber].stdout == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("Erreur stdout append");
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], "2>") == 0)
    {
      /* On utilise le descripteur de la sortie d'erreurs
        pour rediriger la sortie d'erreurs vers le fichier.
      */
      commands[commandNumber].stderr = open(tokens[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0644);
      if (commands[commandNumber].stderr == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("Erreur stderr");
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], "2>>") == 0)
    {
      /* On utilise le descripteur de la sortie d'erreurs
        pour rediriger en ajoutant la sortie d'erreurs vers
        la fin du fichier sans écraser son contenu.
      */
      commands[commandNumber].stderr = open(tokens[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);
      if (commands[commandNumber].stderr == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("Erreur stderr append");
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], "|") == 0)
    {
      /* On commence à créer un tube pour les deux commandes,
        et on ouvert les descripteurs de la sortie standard de la première
        commandes, et l'entrée standard de la deuxième.
      */
      pipe(commands[commandNumber].fdclose);
      commands[commandNumber].stdout = commands[commandNumber].fdclose[1];
      commands[commandNumber + 1].stdin = commands[commandNumber].fdclose[0];
      commands[commandNumber].next = &commands[commandNumber] + 1;

      tokens[i] = NULL;
      ++commandNumber;
      position = i + 1;
      continue;
    }
    if (strcmp(tokens[i], "&&") == 0)
    {
      /* le mot réservé && permet d'exécuter la prochaine
        commande lorsque la commande actuelle est exécutée avec
        succès, alors cette fois on doit affecter la prochaine commande
        dans next_success parce qu'elle dépend de code de retour de la commande
        actuelle.
      */
      if (tokens[i + 1] != NULL)
        commands[commandNumber].next_success = &commands[commandNumber] + 1;
      tokens[i] = NULL;
      ++commandNumber;
      position = i + 1;
      continue;
    }

    if (strcmp(tokens[i], "||") == 0)
    {
      /* le mot réservé || permet d'exécuter la prochaine
        commande lorsque la commande actuelle n'est pas exécutée avec
        succès, alors cette fois on doit affecter la prochaine commande
        dans next_failure.
      */
      if (tokens[i + 1] != NULL)
        commands[commandNumber].next_failure = &commands[commandNumber] + 1;
      tokens[i] = NULL;
      ++commandNumber;
      position = i + 1;
      continue;
    }
  }
  return 0;
}

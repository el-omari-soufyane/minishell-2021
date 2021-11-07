/*
  Projet minishell - Licence 3 Info - PSI 2021
 
  Nom :
  Prénom :
  Num. étudiant :
  Groupe de projet :
  Date :
 
  Gestion des processus (implémentation).
 
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

#include "processus.h"
#include "builtin.h"

#ifndef NDEBUG
int check_zero(void* ptr, size_t size) {
  int result=0;
  for (size_t i=0; i<size; ++i) {
    result+=*((char*) ptr++);
  }
  return result;
}
#endif

/*
  Initialiser une structure process_t avec les
  valeurs par défaut.
 
  proc : un pointeur sur la structure à initialiser
 
  Retourne 0 ou un code d'erreur.
 */
int init_process(process_t* proc) {
  assert(proc!=NULL);
  assert(check_zero(proc, sizeof(*proc))==0);
  
}

/*
  Remplacer les noms de variables d'environnement
  par leurs valeurs.
 
  proc : un pointeur sur la structure à modifier.
 
  Retourne 0 ou un code d'erreur.
 */
int set_env(process_t* proc) {
  assert(proc!=NULL);
  
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
int launch_cmd(process_t* proc) {
    assert(proc!=NULL);
    int status;
    int saved_stdout, saved_stderr;
    do{
        saved_stdout = dup(1);
        saved_stderr = dup(2);
        status = 0;
        pipe(proc->fdclose);
        proc->pid = fork();
        if(proc->pid  == 0) {
            if(proc->bg == 1) {
                close(proc->fdclose[0]);
                dup2(proc->fdclose[1], 1);
                close(proc->fdclose[1]);
            }
            if(proc->stdout != 0) {
                dup2(proc->stdout, 1);
                close(proc->stdout);
            }
            if(proc->stderr != 0) {
                dup2(proc->stderr, 2);
                close(proc->stderr);
            }
            execvp(*proc->argv, proc->argv);
            perror(*proc->argv);
            exit(1);
        } else {
            wait(&status);
            if(proc->bg == 1) {
                close(proc->fdclose[1]);
                dup2(proc->fdclose[0], 0);
                close(proc->fdclose[0]);
                proc++;
                execvp(*proc->argv, proc->argv);
                perror("pipe");
                exit(1);
            }
            if(proc->stdout != 0) {
                close(proc->stdout);
            }
            if(proc->stderr != 0) {
                close(proc->stderr);
            }
            dup2(saved_stdout, 1);
            close(saved_stdout);
            dup2(saved_stderr, 2);
            close(saved_stderr);
            proc++;
        }
    }while(proc->next != NULL);
}

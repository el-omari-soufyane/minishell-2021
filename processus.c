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
int check_zero(void *ptr, size_t size)
{
    int result = 0;
    for (size_t i = 0; i < size; ++i)
    {
        result += *((char *)ptr++);
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
int init_process(process_t *proc)
{
    assert(proc != NULL);
    assert(check_zero(proc, sizeof(*proc)) == 0);
    do
    {
        proc->path = NULL;
        proc->argv = NULL;
        proc->stdin = 0;
        proc->stdout = 1;
        proc->stderr = 2;
        proc->next = NULL;
        proc->next_success = NULL;
        proc->next_failure = NULL;
        proc++;
    } while (proc != NULL);
}

/*
  Remplacer les noms de variables d'environnement
  par leurs valeurs.
 
  proc : un pointeur sur la structure à modifier.
 
  Retourne 0 ou un code d'erreur.
 */
int set_env(process_t *proc)
{
    assert(proc != NULL);

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
        assert(proc!=NULL);
    int tmp_stdin, tmp_stdout, tmp_stderr;
    
    do{
        tmp_stdin = dup(0);
        tmp_stdout = dup(1);
        tmp_stderr = dup(2);
        
        if(is_builtin(proc->argv[0]) == 1) {
            int done = builtin(proc);
            if(done != 0) {
                perror(*proc->argv+1);
            }
            if(proc->next != NULL) {
                proc = proc->next;
                continue;
            }
            
            if(proc->next_success != NULL) {
                if(proc->status == 0) {
                    proc = proc->next_success;
                    continue;
                } else {
                    proc = NULL;
                }
            }
            
            if(proc->next_failure != NULL) {
                if(proc->status != 0) {
                    proc = proc->next_failure;
                    continue;
                } else {
                    proc = NULL;
                }
            }
            proc = NULL;
        } else {
            proc->status = 0;
            proc->pid = fork();
            if(proc->pid  == 0) {
                if(proc->bg == 1) {
                    // S'il s'agit d'une commande Pipe
                    close(proc->fdclose[0]);
                    dup2(proc->fdclose[1], 1);
                    close(proc->fdclose[1]);
                }
                if(proc->stdin > 0) {
                    // S'il s'agit d'une commande > ou >>
                    dup2(proc->stdin, 0);
                    close(proc->stdin);
                }
                if(proc->stdout > 1) {
                    // S'il s'agit d'une commande > ou >>
                    dup2(proc->stdout, 1);
                    close(proc->stdout);
                    close(proc->stdin);
                }
                if(proc->stderr > 2) {
                    // S'il s'agit d'une commande 2> ou 2>>
                    dup2(proc->stderr, 2);
                    close(proc->stderr);
                }
                execvp(*proc->argv, proc->argv);
                perror(*proc->argv);
                return -1;
            } else {
                wait(&proc->status);
                if(proc->bg == 1) {
                    close(proc->fdclose[1]);
                    dup2(proc->fdclose[0], 0);
                    close(proc->fdclose[0]);
                    proc++;
                    execvp(*proc->argv, proc->argv);
                    perror("pipe");
                    return -1;
                }
                if(proc->stdin > 0) {
                    close(proc->stdin);
                }
                if(proc->stdout > 1) {
                    close(proc->stdout);
                }
                if(proc->stderr > 2) {
                    close(proc->stderr);
                }
                // Revenir à l'état standard des sorties standard et erreurs
                dup2(tmp_stdin, 0);
                close(tmp_stdin);
                dup2(tmp_stdout, 1);
                close(tmp_stdout);
                dup2(tmp_stderr, 2);
                close(tmp_stderr);
                
                if(proc->next != NULL) {
                    proc = proc->next;
                    continue;
                }
                
                if(proc->next_success != NULL) {
                    if(proc->status == 0) {
                        proc = proc->next_success;
                        continue;
                    } else {
                        proc = NULL;
                    }
                }
                
                if(proc->next_failure != NULL) {
                    if(proc->status != 0) {
                        proc = proc->next_failure;
                        continue;
                    } else {
                        proc = NULL;
                    }
                }
                
                proc = NULL;
            }
        }
    }while(proc != NULL);
    return 0;
}
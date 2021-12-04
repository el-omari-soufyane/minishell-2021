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
        proc = proc->next;
    } while (proc != NULL);
    return 0;
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
    /*
        Pour remplacer une variable d'environnement
        par sa valeur, on doit vérifier qu'elle est
        précédée par le symbole '$'.
        Si c'est le cas, on prend le nom de la variable
        après le '$' et on remplace l'argument
        par la valeur de variable d'environnement.
    */
    int i = 0;
    while (proc->argv[i] != NULL)
    {
        if (proc->argv[i][0] == '$')
        {
            char *var = proc->argv[i];
            memmove(var, var + 1, strlen(var));
            proc->argv[i] = getenv(var);
        }
        i++;
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
    /*
        Les variables déclarées ci-dessous, sont des
        variables qui dupliquent les sorties standard et
        d'erreurs ainsi que de l'entrée standard afin que
        chaque processus les utilise seulement à l'exécution.
    */
    int tmp_stdin, tmp_stdout, tmp_stderr;
    do
    {
        tmp_stdin = dup(0);
        tmp_stdout = dup(1);
        tmp_stderr = dup(2);

        /*
            On vérifie si la commande à exécutée contient des
            variables d'environnement dans ses arguments.
        */
        set_env(proc);

        /*
            Si la commande est builtin, on gére
            la prochaine commande.
        */
        if (is_builtin(proc->argv[0]) == 1)
        {
            int done = builtin(proc);
            if (done != 0)
            {
                perror(proc->argv[1]);
            }
            if (proc->next != NULL)
            {
                proc = proc->next;
                continue;
            }

            if (proc->next_success != NULL)
            {
                if (proc->status == 0)
                {
                    proc = proc->next_success;
                    continue;
                }
                else
                {
                    proc = NULL;
                }
            }

            if (proc->next_failure != NULL)
            {
                if (proc->status != 0)
                {
                    proc = proc->next_failure;
                    continue;
                }
                else
                {
                    proc = NULL;
                }
            }
            proc = NULL;
        }
        else
        {
            /*
                Sinon, on crée un processus fils
                pour exécuter la commande, et le
                processu père va attendre le status
                de la commande.
            */
            proc->status = 0;
            proc->pid = fork();
            if (proc->pid == 0)
            {
                if (proc->stdin > 0 && proc->stdout > 1)
                {
                    // S'il s'agit d'une commande |
                    close(proc->fdclose[0]);
                    dup2(proc->fdclose[1], 1);
                    close(proc->fdclose[1]);
                }
                if (proc->stdin > 0)
                {
                    // S'il s'agit d'une commande <
                    dup2(proc->stdin, 0);
                    close(proc->stdin);
                }
                if (proc->stdout > 1)
                {
                    // S'il s'agit d'une commande > ou >>
                    dup2(proc->stdout, 1);
                    close(proc->stdout);
                    close(proc->stdin);
                }
                if (proc->stderr > 2)
                {
                    // S'il s'agit d'une commande 2> ou 2>>
                    dup2(proc->stderr, 2);
                    close(proc->stderr);
                }
                execvp(*proc->argv, proc->argv);
                perror(*proc->argv);
                exit(1);
            }
            else
            {
                if (proc->stdin > 0 && proc->stdout > 1)
                {
                    close(proc->fdclose[1]);
                    dup2(proc->fdclose[0], 0);
                    close(proc->fdclose[0]);
                    proc++;
                    execvp(*proc->argv, proc->argv);
                    perror("pipe error");
                    exit(1);
                }
                wait(&proc->status);
                if (proc->stdin > 0)
                {
                    close(proc->stdin);
                }
                if (proc->stdout > 1)
                {
                    close(proc->stdout);
                }
                if (proc->stderr > 2)
                {
                    close(proc->stderr);
                }
                // Revenir à l'état standard des sorties standard et d'erreurs
                dup2(tmp_stdin, 0);
                close(tmp_stdin);
                dup2(tmp_stdout, 1);
                close(tmp_stdout);
                dup2(tmp_stderr, 2);
                close(tmp_stderr);

                // S'il s'agit d'une commande ;
                if (proc->next != NULL)
                {
                    proc = proc->next;
                    continue;
                }

                // S'il s'agit d'une commande &&
                if (proc->next_success != NULL)
                {
                    if (proc->status == 0)
                    {
                        proc = proc->next_success;
                        continue;
                    }
                    else
                    {
                        proc = NULL;
                    }
                }

                // S'il s'agit d'une commande ||
                if (proc->next_failure != NULL)
                {
                    if (proc->status != 0)
                    {
                        proc = proc->next_failure;
                        continue;
                    }
                    else
                    {
                        proc = NULL;
                    }
                }

                proc = NULL;
            }
        }
    } while (proc != NULL);
    return 0;
}
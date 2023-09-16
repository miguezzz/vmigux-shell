#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //acesso a ficheiros e directorias
#include <sys/wait.h> //Report status of stopped child process.
#include <fcntl.h> /*
                        The fcntl is a system call. It allows the program to place a read or a write lock. 
                        This function can be used to amend the file properties that are either opened already 
                        or can be opened through any action applied to it. It is a versatile function 
                        and is used to modify files in many ways like open, read and write, etc.
                   */
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_INPUT_SIZE 1024 //evitaremos overflow
#define MAX_ARG_SIZE 64 //evitar que os argumentos se tornem excessivamente longos (medida de seguranca)

int main(void) {

    while(1) { //executando "pra sempre"

        //username / diretorio atual
        //https://www.tutorialspoint.com/c_standard_library/c_function_getenv.htm
        char *user = getenv("USER");
        char cwd[1024]; //cwd == current working directory
        getcwd(cwd, sizeof(cwd));

        //cria o prompt personalizado
        char prompt[2048];
        //https://www.geeksforgeeks.org/snprintf-c-library/
        snprintf(prompt, sizeof(prompt), "%s@%s> ", user, cwd);

        //le o comando do user e exibe o prompt personalizado
        char *input = readline(prompt);

        if (input == NULL) {
            // ctrl + d || erro ao ler a entrada, sai do vmigux
            break;
        }

        if (strlen(input) > 0) {
            //comando vai pro historico
            add_history(input);

            char *commands[MAX_INPUT_SIZE];
            int command_counter = 0;

            //divide a entrada em comandos separados por ";"
            char *token = strtok(input, ";");
            while (token != NULL) {
                commands[command_counter] = token;
                command_counter++;
                token = strtok(NULL, ";");
            }

            for (int i = 0; i < command_counter; i++) {
                char *command = commands[i];

                char expanded_command[MAX_INPUT_SIZE];
                char *token = strtok(command, " ");
                strcpy(expanded_command, "");


                while (token != NULL) {
                    char *env_value = getenv(token + 1); // Ignora o "$" no início
                    if (env_value != NULL) {
                        //The strcat() function concatenates the destination string and the source string, and the result is stored in the destination string.
                        //https://www.programiz.com/c-programming/library-function/string.h/strcat
                        strcat(expanded_command, env_value);
                    } else {
                        strcat(expanded_command, token);
                    }

                    strcat(expanded_command, " "); // Adiciona um espaço entre os tokens
                    token = strtok(NULL, " ");
                }

                // Remove a quebra de linha da entrada para evitar problemas com o \n
                expanded_command[strlen(expanded_command) - 1] = '\0';

                //armazenando tokens (comando e argumentos)
                char *tokens[MAX_ARG_SIZE];
                int token_counter = 0;

                //divide entrada em tokens usando o espaco (" ") como divisor
                token = strtok(expanded_command, " ");
                while (token != NULL) {
                    tokens[token_counter] = token;
                    token_counter++;
                
                    //proximo token
                    token = strtok(NULL, " ");
                }

                //verifica se o usuario inseriu um comando
                if (token_counter > 0) {
                    //primeiro token eh o comando
                    char *command = tokens[0];
                    //o resto dos tokens sao os args
                    char **args = &tokens[1];

                //comandos internos
                /*
                    Comandos internos são comandos que são tratados pelo próprio shell em vez de serem executados como programas externos.
                    Alguns exemplos de comandos internos comuns incluem "cd" para mudar de diretório,
                    "help" para exibir informações de ajuda e "exit" para sair do shell.
                */
                if (strcmp(command, "help") == 0) {
                    //exibe informacoes de ajuda
                    printf("Bem-vindo(a) ao vmigux-shell!\n");
                    printf("Comandos disponíveis:\n");
                    printf("cd [diretório]: Mudar de diretório\n");
                    printf("help: Exibir mensagem de ajuda\n");
                    printf("vexit: Sair do vmigux-shell\n");
                    command_counter++;
                }
                else if (strcmp(command, "vexit") == 0) {
                    //sai do vmigux-shell
                    printf("Saindo do vmigux-shell... see you!\n");
                    exit(EXIT_SUCCESS);
                }
                else {
                    //comando n reconhecido. partir para a execucao de comandos externos

                    //cria um novo processo
                    pid_t pid = fork();

                    if (pid == 0) {
                        //processo filho
                        //executa o comando + os args
                        //"Use execvp com a função fork para criar um processo filho e executar um programa diferente em C"
                        execvp(command, args);

                        //se execvp retornar, houve erro
                        //a função perror mapeia o erro numérico, contido na variável global errno, para uma mensagem de erro. Em seguida, a função imprime essa mensagem de erro na saída
                        perror("Error executing command");
                        exit(EXIT_FAILURE);
                    }
                    else if (pid > 0) {
                        //proccesso pai
                        //aguarda a conclusao do processo filho
                        waitpid(pid, NULL, 0);
                    }
                    else {
                        perror("Error creating a new process");
                        exit(EXIT_FAILURE);
                    }

                    //prox comando separado por ";"
                    command = strtok(NULL, ";");
                }
            }
        }

        //sempre bom liberar a memoria alocada kkkkk
        free(input);
    }

    return 0;
}
}
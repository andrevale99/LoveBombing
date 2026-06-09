#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void process_command(char *rx_buffer)
{
    char *cmd;
    char *value;

    /* Remove \r e \n */
    rx_buffer[strcspn(rx_buffer, "\r\n")] = '\0';

    cmd = strtok(rx_buffer, " ");
    value = strtok(NULL, " ");

    if ((cmd != NULL) && (value != NULL))
    {
        printf("Comando: %s\n", cmd);
        printf("Valor: %s\n", value);

        int number = atoi(value);
        printf("Valor inteiro: %d\n", number);
    }
    else
    {
        printf("Formato invalido\n");
    }
}

int main(int argc, char **argv)
{
    char recv[] = {"S 43"};
    process_command(recv);
    return 0;
}
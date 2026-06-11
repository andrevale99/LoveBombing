#include "letter.h"

int8_t letter_decoder(letter_t *letter, char *rx_buffer)
{
    if (!letter || !rx_buffer)
        return -1;

    char *cmd;
    char *value_str;

    /* Remove \r e \n */
    rx_buffer[strcspn(rx_buffer, "\r\n")] = '\0';

    cmd = strtok(rx_buffer, " ");
    value_str = strtok(NULL, " ");

    if ((cmd == NULL) || (value_str == NULL))
        return -2;

    strncpy(letter->cmd, cmd, sizeof(letter->cmd) - 1);
    letter->cmd[LETTER_CMD_SIZE - 1] = '\0';

    letter->value = atoi(value_str);

    return 0;
}

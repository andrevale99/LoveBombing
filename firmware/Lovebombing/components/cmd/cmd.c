#include "cmd.h"

int8_t cmd_decoder(cmd_t *cmd, char *rx_buffer)
{
    if (!cmd || !rx_buffer)
        return -1;

    char *cmd_string;
    char *value_str;

    /* Remove \r e \n */
    rx_buffer[strcspn(rx_buffer, "\r\n")] = '\0';

    cmd_string = strtok(rx_buffer, " ");
    value_str = strtok(NULL, " ");

    if ((cmd_string == NULL) || (value_str == NULL))
        return -2;

    strncpy(cmd->cmd, cmd_string, sizeof(cmd->cmd) - 1);
    cmd->cmd[CMD_STRING_SIZE - 1] = '\0';

    cmd->value = atoi(value_str);

    return 0;
}

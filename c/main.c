#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include "dynamixel_sdk/dynamixel_sdk.h"

// Control table addresses
#define ADDR_TORQUE_ENABLE       64
#define ADDR_GOAL_POSITION       116
#define ADDR_PRESENT_POSITION    132

#define PROTOCOL_VERSION         2.0
#define BAUDRATE                 1000000
#define DEVICENAME               "COM10"

// Right arm motor IDs
#define ID_M1                    1
#define ID_M3                    3
#define ID_M5                    5

#define M1_INICIAL  2048
#define M3_INICIAL  2250
#define M5_INICIAL  1650


// Checks communication and motor errors
int check_errors(int port_num)
{
    int result;
    uint8_t error;

    result = getLastTxRxResult(port_num, PROTOCOL_VERSION);

    if (result != COMM_SUCCESS)
    {
        printf("Erro de comunicacao: %s\n",
               getTxRxResult(PROTOCOL_VERSION, result));
        return 0;
    }

    error = getLastRxPacketError(port_num, PROTOCOL_VERSION);

    if (error != 0)
    {
        printf("Erro do motor: %s\n",
               getRxPacketError(PROTOCOL_VERSION, error));
        return 0;
    }

    return 1;
}

// Sends position command
int move(int port_num, uint8_t id, uint32_t position)
{
    write4ByteTxRx(
        port_num,
        PROTOCOL_VERSION,
        id,
        ADDR_GOAL_POSITION,
        position
    );

    return check_errors(port_num);
}

// Enables or disables torque
int set_torque(int port_num, uint8_t id, uint8_t state)
{
    write1ByteTxRx(
        port_num,
        PROTOCOL_VERSION,
        id,
        ADDR_TORQUE_ENABLE,
        state
    );

    return check_errors(port_num);
}

int main(void)
{
    int i;
    int success = 1;

    uint8_t motors[] = {
        ID_M1,
        ID_M3,
        ID_M5
    };

    printf("1 - INICIO\n");
    fflush(stdout);

    // 1. Cria o port handler
    int port_num = portHandler(DEVICENAME);

    printf("2 - portHandler OK\n");
    fflush(stdout);

    // 2. Inicializa o packet handler
    packetHandler();

    printf("3 - packetHandler OK\n");
    fflush(stdout);

    // 3. Abre a porta
    if (!openPort(port_num))
    {
        printf("4 - ERRO ao abrir a porta!\n");
        return 1;
    }

    printf("4 - PORTA ABERTA\n");
    fflush(stdout);

    // 4. Configura o baudrate
    if (!setBaudRate(port_num, BAUDRATE))
    {
        printf("Erro ao configurar baudrate!\n");
        closePort(port_num);
        return 1;
    }

    printf("Communication established!\n");

    // 5. Liga o torque
    for (i = 0; i < 3; i++)
    {
        printf("Ativando torque do motor ID %d...\n",
               motors[i]);

        if (!set_torque(port_num, motors[i], 1))
        {
            success = 0;
            break;
        }
    }

    // 6. Posiciona os motores na pose inicial
    if (success)
    {
// M3 sobe 90 graus anti-horário
move(port_num, ID_M3, M3_INICIAL + 683);
Sleep(1000);

// M1 gira 90 graus anti-horário
move(port_num, ID_M1, M1_INICIAL + 1024);
Sleep(1000);
    }

    // 7. Movimento de onda
    if (success)
    {
        printf("Starting waving movement...\n");

        for (i = 0; i < 4; i++)
        {
            if (!move(port_num, ID_M5, 1100))
            {
                success = 0;
                break;
            }

            Sleep(350);

            if (!move(port_num, ID_M5, 1700))
            {
                success = 0;
                break;
            }

            Sleep(350);
        }
    }

    // 8. Retorna para a posicao inicial
    if (success)
    {
        printf("Returning to initial position...\n");

        if (!move(port_num, ID_M5, M5_INICIAL))
            success = 0;

        if (success && !move(port_num, ID_M3, M3_INICIAL))
            success = 0;

        if (success && !move(port_num, ID_M1, M1_INICIAL))
            success = 0;

        if (success)
            Sleep(1500);
    }

    // 9. Desliga torque
    for (i = 0; i < 3; i++)
    {
        set_torque(port_num, motors[i], 0);
    }

    // 10. Fecha a porta
    closePort(port_num);

    if (success)
    {
        printf("Movement completed!\n");
        return 0;
    }
    else
    {
        printf("An error occurred during execution!\n");
        return 1;
    }
}
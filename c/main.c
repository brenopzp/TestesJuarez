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
    
    // int port_num;
    int i;
    int success = 1;

        printf("1 - INICIO\n");
    fflush(stdout);

    int port_num = portHandler(DEVICENAME);
    printf("2 - portHandler OK\n");
    fflush(stdout);

    packetHandler();
    printf("3 - packetHandler OK\n");
    fflush(stdout);

    if (!openPort(port_num))
    {
        printf("4 - ERRO ao abrir a porta!\n");
        return 1;
    }

    printf("4 - PORTA ABERTA\n");
    fflush(stdout);


    uint8_t motors[] = {ID_M1, ID_M3, ID_M5};

   // port_num = portHandler(DEVICENAME);
    packetHandler();

    // 1. Open port
    if (!openPort(port_num))
    {
        printf("Error opening port %s\n", DEVICENAME);
        return 1;
    }

    // 2. Configure baudrate
    if (!setBaudRate(port_num, BAUDRATE))
    {
        printf("Error configuring baudrate!\n");
        closePort(port_num);
        return 1;
    }

    printf("Communication established!\n");

    // 3. Enable motor torque
    for (i = 0; i < 3; i++)
    {
        if (!set_torque(port_num, motors[i], 1))
        {
            success = 0;
            break;
        }
    }

    // 4. Prepare arm
    if (success)
    {
        printf("Preparing the arm...\n");

        if (!move(port_num, ID_M1, 2048))
            success = 0;

        if (success && !move(port_num, ID_M3, 2800))
            success = 0;

        if (success && !move(port_num, ID_M5, 1400))
            success = 0;

        if (success)
            Sleep(1500);
    }

    // 5. Waving movement
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

    // 6. Return to initial position
    if (success)
    {
        printf("Returning to initial position...\n");

        if (!move(port_num, ID_M5, 2048))
            success = 0;

        if (success && !move(port_num, ID_M3, 2048))
            success = 0;

        if (success && !move(port_num, ID_M1, 2048))
            success = 0;

        if (success)
            Sleep(1500);
    }

    // 7. Disable torque
    for (i = 0; i < 3; i++)
    {
        set_torque(port_num, motors[i], 0);
    }

    // 8. Close communication
    closePort(port_num);

    if (success)
        printf("Movement completed!\n");
    else
        printf("An error occurred during execution!\n");

    return success ? 0 : 1;
}
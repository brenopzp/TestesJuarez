
#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include "dynamixel_sdk/dynamixel_sdk.h"

#define DEVICENAME "COM3"
#define BAUDRATE 57600
#define PROTOCOL_VERSION 2.0

#define ID_TEST 1

#define ADDR_TORQUE_ENABLE 64
#define ADDR_OPERATING_MODE 11
#define ADDR_GOAL_POSITION 116
#define ADDR_PRESENT_POSITION 132

// Verifica a comunicacao
int verificar(int port_num)
{
    int resultado = getLastTxRxResult(
        port_num, PROTOCOL_VERSION
    );

    if (resultado != COMM_SUCCESS)
    {
        printTxRxResult(PROTOCOL_VERSION, resultado);
        return 0;
    }

    uint8_t erro = getLastRxPacketError(
        port_num, PROTOCOL_VERSION
    );

    if (erro != 0)
    {
        printRxPacketError(PROTOCOL_VERSION, erro);
        return 0;
    }

    return 1;
}

int main(void)
{
    int port_num = portHandler(DEVICENAME);
    packetHandler();

    if (!openPort(port_num))
    {
        printf("Erro ao abrir a porta!\n");
        return 1;
    }

    if (!setBaudRate(port_num, BAUDRATE))
    {
        printf("Erro no baud rate!\n");
        closePort(port_num);
        return 1;
    }

    printf("Comunicacao estabelecida!\n");

    // Verifica o modo de operacao
    uint8_t modo = read1ByteTxRx(
        port_num, PROTOCOL_VERSION,
        ID_TEST, ADDR_OPERATING_MODE
    );

    if (!verificar(port_num) || modo != 3)
    {
        printf("Motor nao esta no Position Control Mode!\n");
        closePort(port_num);
        return 1;
    }

    // Verifica o torque
    uint8_t torque = read1ByteTxRx(
        port_num, PROTOCOL_VERSION,
        ID_TEST, ADDR_TORQUE_ENABLE
    );

    if (!verificar(port_num))
    {
        closePort(port_num);
        return 1;
    }

    // Le a posicao atual
    uint32_t posicao = read4ByteTxRx(
        port_num, PROTOCOL_VERSION,
        ID_TEST, ADDR_PRESENT_POSITION
    );

    if (!verificar(port_num) || posicao > 4095)
    {
        printf("Posicao invalida para o teste!\n");
        closePort(port_num);
        return 1;
    }

    printf("Posicao atual: %u\n", posicao);

    // Se o torque estiver desligado, prepara
    // o alvo atual antes de ativa-lo.
    if (torque == 0)
    {
        write4ByteTxRx(
            port_num, PROTOCOL_VERSION,
            ID_TEST, ADDR_GOAL_POSITION, posicao
        );

        if (!verificar(port_num))
        {
            closePort(port_num);
            return 1;
        }

        write1ByteTxRx(
            port_num, PROTOCOL_VERSION,
            ID_TEST, ADDR_TORQUE_ENABLE, 1
        );

        if (!verificar(port_num))
        {
            closePort(port_num);
            return 1;
        }
    }

    // Le novamente a posicao
    posicao = read4ByteTxRx(
        port_num, PROTOCOL_VERSION,
        ID_TEST, ADDR_PRESENT_POSITION
    );

    if (!verificar(port_num) || posicao > 4095)
    {
        printf("Erro ao ler a posicao!\n");
        closePort(port_num);
        return 1;
    }

    // Deslocamento pequeno de 20 unidades
    uint32_t alvo = (posicao <= 4075)
                    ? posicao + 20
                    : posicao - 20;

    printf("Movendo para: %u\n", alvo);

    write4ByteTxRx(
        port_num, PROTOCOL_VERSION,
        ID_TEST, ADDR_GOAL_POSITION, alvo
    );

    if (!verificar(port_num))
    {
        closePort(port_num);
        return 1;
    }

    Sleep(1000);

    // Retorna a posicao original
    write4ByteTxRx(
        port_num, PROTOCOL_VERSION,
        ID_TEST, ADDR_GOAL_POSITION, posicao
    );

    if (!verificar(port_num))
    {
        closePort(port_num);
        return 1;
    }

    Sleep(1000);

    printf("Teste concluido!\n");

    // Mantem o torque ligado para sustentar a posicao.
    closePort(port_num);

    return 0;
}

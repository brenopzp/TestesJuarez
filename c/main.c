#include <stdio.h>
#include <windows.h>
#include "dynamixel_sdk/dynamixel_sdk.h"

// Endereços da Tabela de Controlo (Protocolo 2.0 - MX-28 / X-Series)
#define ADDR_TORQUE_ENABLE          64
#define ADDR_GOAL_POSITION          116
#define ADDR_PRESENT_POSITION       132

#define PROTOCOL_VERSION            2.0
#define BAUDRATE                    1000000
#define DEVICENAME                  "COM3"  // Ajuste para a sua porta COM

// IDs dos motores do braço direito
#define ID_M1                       1
#define ID_M3                       3
#define ID_M5                       5

// Função para enviar o comando de posição
void mover(int port_num, uint8_t id, uint32_t posicao) {
    write4ByteTxRx(port_num, PROTOCOL_VERSION, id, ADDR_GOAL_POSITION, posicao);
}

// Função para ativar ou desativar o torque
void set_torque(int port_num, uint8_t id, uint8_t estado) {
    write1ByteTxRx(port_num, PROTOCOL_VERSION, id, ADDR_TORQUE_ENABLE, estado);
}

int main() {
    int port_num = portHandler(DEVICENAME);
    packetHandler();

    // 1. Abrir a porta e configurar baudrate
    if (!openPort(port_num) || !setBaudRate(port_num, BAUDRATE)) {
        printf("Erro ao conectar na porta %s\n", DEVICENAME);
        return -1;
    }

    // 2. Ativar torque nos três motores do braço
    set_torque(port_num, ID_M1, 1);
    set_torque(port_num, ID_M3, 1);
    set_torque(port_num, ID_M5, 1);

    printf("Iniciando o movimento de tchauzinho...\n");

    // 3. Levantar o braço para a posição de acenar
    // (Escala de 0 a 4095; 2048 representa a posição central / 180°)
    mover(port_num, ID_M1, 2048); // Ombro em posição neutra
    mover(port_num, ID_M3, 2800); // Levanta o braço para cima/frente
    mover(port_num, ID_M5, 1400); // Dobra o cotovelo apontando a mão para cima
    Sleep(1500);                  // Tempo para concluir a subida

    // 4. Ciclo de aceno ("tchauzinho") - 4 oscilações do antebraço (M5)
    for (int i = 0; i < 4; i++) {
        mover(port_num, ID_M5, 1100); // Inclina a mão para um lado
        Sleep(350);

        mover(port_num, ID_M5, 1700); // Inclina a mão para o outro lado
        Sleep(350);
    }

    // 5. Retornar o braço à posição de repouso (Centro = 2048)
    mover(port_num, ID_M5, 2048);
    mover(port_num, ID_M3, 2048);
    mover(port_num, ID_M1, 2048);
    Sleep(1500);

    // 6. Desativar torque e fechar comunicação
    set_torque(port_num, ID_M1, 0);
    set_torque(port_num, ID_M3, 0);
    set_torque(port_num, ID_M5, 0);

    closePort(port_num);
    printf("Movimento concluido com sucesso!\n");

    return 0;
}
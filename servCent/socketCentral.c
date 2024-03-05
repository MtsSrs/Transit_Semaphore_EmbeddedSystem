#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SERVER_PORT 5000
#define MAX_CLIENTS 10

int clientSockets[MAX_CLIENTS];
int clientIDs[MAX_CLIENTS];
int nextClientID = 0;
int numClients = 0;
pthread_t clientThreads[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool diagnosisPrinted[MAX_CLIENTS];
char clientBuffers[MAX_CLIENTS][2048];

void saveDataToFile(int clientID, char *data) {
    FILE *file = fopen("client_data.txt", "a");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo.");
        return;
    }
    fprintf(file, "Dados do Cliente %d:\n%s\n", clientID, data);
    fclose(file);
}

void clearDataFile() {
    FILE *file = fopen("client_data.txt", "w");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo.");
        return;
    }
    fclose(file);
}

void sendToAllClients(const char *message) {
    for (int i = 0; i < numClients; i++) {
        send(clientSockets[i], message, strlen(message), 0);
    }
}

void *handleClient(void *clientSocketPtr) {
    int clientSocket = *((int *)clientSocketPtr);
    int clientID;

    pthread_mutex_lock(&mutex);
    clientID = nextClientID++;
    clientIDs[clientID] = clientID;
    pthread_mutex_unlock(&mutex);

    char buffer[1024];
    ssize_t bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesRead] = '\0';
        strcpy(clientBuffers[clientID], buffer);
        saveDataToFile(clientID, buffer);
        diagnosisPrinted[clientID] = false;
    }

    pthread_mutex_lock(&mutex);
    numClients--;
    pthread_mutex_unlock(&mutex);
    close(clientSocket);

    return NULL;
}

void *menuThread() {
    while (1) {
        int choice;
        printf("####### Menu #######\n");
        printf("1 - Ver diagnóstico de conexões\n");
        printf("2 - Sair\n");
        printf("3 - Limpar arquivo de dados\n");
        printf("4 - Ativar Modo de Emergência\n");
        printf("5 - Ativar Modo Noturno\n");
        printf("6 - Voltar ao normal\n");
        printf("####################\n");

        scanf("%d", &choice);

        if (choice == 1) {
            if (numClients == 0) {
                printf("SEM CRUZAMENTOS CONECTADOS\n");
            } else {
                system("clear");
                for (int i = 0; i < numClients; i++) {
                    if (!diagnosisPrinted[i]) {
                        printf("Diagnóstico do Cliente %d:\n%s\n", clientIDs[i], clientBuffers[i]);
                        diagnosisPrinted[i] = true;
                    }
                }
            }
        } else if (choice == 2) {
            for (int i = 0; i < numClients; i++) {
                close(clientSockets[i]);
            }
            exit(0);
        } else if (choice == 3) {
            clearDataFile();
            printf("Arquivo de dados limpo.\n");
        } else if (choice == 4) {
            sendToAllClients("MODO_EMERGENCIA");
            printf("Modo de Emergência Ativado.\n");
        } else if (choice == 5) {
            sendToAllClients("MODO_NOTURNO");
            printf("Modo Noturno Ativado.\n");
        } else if (choice == 6) {
            sendToAllClients("NORMAL");
            printf("Semaforo voltando para o estado normal.\n");
        }
    }
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        diagnosisPrinted[i] = false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Erro ao criar o socket do servidor.");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Erro ao vincular o servidor ao endereço.");
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        perror("Erro ao iniciar o servidor.");
        close(serverSocket);
        return 1;
    }

    printf("Servidor central aguardando conexões...\n");

    pthread_t menu;
    pthread_create(&menu, NULL, menuThread, NULL);

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
        if (clientSocket == -1) {
            perror("Erro ao aceitar a conexão do cliente.");
            continue;
        }

        pthread_mutex_lock(&mutex);
        clientSockets[numClients] = clientSocket;
        numClients++;
        pthread_mutex_unlock(&mutex);

        pthread_t clientThread;
        if (pthread_create(&clientThread, NULL, handleClient, &clientSocket) != 0) {
            perror("Erro ao criar a thread do cliente.");
            close(clientSocket);
            continue;
        }

        clientThreads[numClients - 1] = clientThread;
    }

    close(serverSocket);
    return 0;
}

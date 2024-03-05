#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdbool.h>
#include "./include/tempos.h"

// Estrutura para guardar as pinagens
struct Pinagem
{
    int VERDE_SEMAFORO_1;
    int AMARELO_SEMAFORO_1;
    int VERDE_SEMAFORO_2;
    int AMARELO_SEMAFORO_2;
    int BOTAO_PED_1;
    int BOTAO_PED_2;
    int SENSOR_PRIN_1;
    int SENSOR_PRIN_2;
    int SENSOR_AUX_1;
    int SENSOR_AUX_2;
    int BUZZER;
};

struct Pinagem pinagem;

enum EstadosSemaforo
{
    VERDE_PRINCIPAL,
    AMARELO_PRINCIPAL,
    VERMELHO_PRINCIPAL,
    AMARELO_AUXILIAR,
    MODO_NOTURNO,
    MODO_EMERGENCIA,
};

// Estado inicial do semáforo
enum EstadosSemaforo estadoAtual = VERDE_PRINCIPAL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int pausarThreadPrincipal = 0;

// Definição de variáveis de tempo
time_t tempo_farol_verde_prin;
time_t tempo_farol_verde_aux;



// Variáveis de controle
int controlaAmareloPrincipal = 0;
int controlaAmareloAuxiliar = 0;
int carroParadoAuxiliar = 0;
int carroDirAuxiliar1 = 0;
int carroDirAuxiliar2 = 0;
int carroDirPrincipal1 = 0;
int carroDirPrincipal2 = 0;
int contagemCarrosPrincipal = 0;
int contagemCarrosAuxiliar = 0;
double velocidadeTotalPrincipal = 0;
double velocidadeTotalAuxiliar = 0;

// Retorno para o server
int avancouVermelho = 0;
int acimaVelocidade = 0;

// Protótipos das funções
void lerPinagemDoArquivo(const char *nomeArquivo);
void definirEstadoSemaforoPrin(int pino1, int pino2);
void definirEstadoSemaforoAux(int pino1, int pino2);
void aguardaPedestre(int botao, time_t tempoPassado, time_t tempoBotaoPress, int tempoMinFarol, void (*definirSemaforo)(enum EstadosSemaforo novoEstado), enum EstadosSemaforo estadoPedestre);
void botao1pressionado();
void botao2pressionado();
void abrirAuxiliar(time_t tempoCarroParou);
unsigned long medirDuracaoBotaoPressionado(int pinoBotao);
void *monitorarSensoresAuxiliares(void *arg);
void *monitorarSensoresPrincipais(void *arg);
void *contagemCarros(void *arg);
void atualizarSemaforo();
void setEstado(enum EstadosSemaforo novoEstado);
void temporizador();
void configurarGPIO();
void controlaSensoresAuxiliares(int pino, int sensor);
void controlaSensoresPrincipais(int pino, int sensor);
double calcularVelocidade(unsigned long duracaoSensor);
void ativarBuzzer(int pinoBuzzer);
void verificarVelocidadeExcessiva(double velocidadeMedida, int via, int sensor);
void enviarArquivoParaServidor(const char *nomeArquivo);
void enviarDadosParaServidor(int clientSocket, const char *dados);
int connectToServer();
void *tentaConectar(void *arg);
void avancouSinal(int sensor);
double calcularVelocidadeMedia(double velocidadeTotal, int contagemCarros);
ssize_t receberDadosDoServidor(int clientSocket, char *buffer, size_t bufferSize);
void *escutaServidor(void *arg);

int main(int argc, char *argv[])
{
    // Leia as informações de pinagem do primeiro arquivo
    const char *arquivoConfig = argv[1];
    lerPinagemDoArquivo(arquivoConfig);

    // Iniciando wiringPi
    if (wiringPiSetup() == -1)
    {
        printf("Erro ao inicializar a WiringPi.\n");
        return 1; // Encerra o programa com um código de erro
    }

    // Configuração GPIO
    configurarGPIO();

    // Thread para botões auxiliares
    pthread_t threadSensorAuxiliar;
    pthread_t threadSensorPrincipal;
    pthread_t contaCarro;
    pthread_create(&threadSensorAuxiliar, NULL, monitorarSensoresAuxiliares, NULL);
    pthread_create(&threadSensorPrincipal, NULL, monitorarSensoresPrincipais, NULL);
    pthread_create(&contaCarro, NULL, contagemCarros, NULL);

    // Thread principal do temporizador
    temporizador();

    // Terminando as threads
    pthread_join(threadSensorAuxiliar, NULL);
    pthread_join(threadSensorPrincipal, NULL);
    pthread_join(contaCarro, NULL);
    return 0;
}

// Função para ler as informações de pinagem de um arquivo e atribuí-las à estrutura
void lerPinagemDoArquivo(const char *nomeArquivo)
{
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (!arquivo)
    {
        printf("Erro ao abrir o arquivo de configuração.\n");
        exit(1);
    }

    fscanf(arquivo, "%d", &pinagem.VERDE_SEMAFORO_1);
    fscanf(arquivo, "%d", &pinagem.AMARELO_SEMAFORO_1);
    fscanf(arquivo, "%d", &pinagem.VERDE_SEMAFORO_2);
    fscanf(arquivo, "%d", &pinagem.AMARELO_SEMAFORO_2);
    fscanf(arquivo, "%d", &pinagem.BOTAO_PED_1);
    fscanf(arquivo, "%d", &pinagem.BOTAO_PED_2);
    fscanf(arquivo, "%d", &pinagem.SENSOR_PRIN_1);
    fscanf(arquivo, "%d", &pinagem.SENSOR_PRIN_2);
    fscanf(arquivo, "%d", &pinagem.SENSOR_AUX_1);
    fscanf(arquivo, "%d", &pinagem.SENSOR_AUX_2);
    fscanf(arquivo, "%d", &pinagem.BUZZER);

    fclose(arquivo);
}

// Função para definir o estado do semáforo principal
void definirEstadoSemaforoPrin(int pino1, int pino2)
{
    digitalWrite(pinagem.VERDE_SEMAFORO_1, pino1);
    digitalWrite(pinagem.AMARELO_SEMAFORO_1, pino2);
}

// Função para definir o estado do semáforo auxiliar
void definirEstadoSemaforoAux(int pino1, int pino2)
{
    digitalWrite(pinagem.VERDE_SEMAFORO_2, pino1);
    digitalWrite(pinagem.AMARELO_SEMAFORO_2, pino2);
}

// Função de mudança de estado após apertar botão
void aguardaPedestre(int botao, time_t tempoPassado, time_t tempoBotaoPress, int tempoMinFarol, void (*definirSemaforo)(enum EstadosSemaforo novoEstado), enum EstadosSemaforo estadoPedestre)
{
    double tempoDecorrido = difftime(tempoBotaoPress, tempoPassado);

    if ((tempoDecorrido * 1000) >= (tempoMinFarol * 1000))
    {
        printf("Botao %d entrou depois do tempo minimo\n", botao);
        setEstado(estadoPedestre);
        return;
    }
    else
    {
        delay((tempoMinFarol - tempoDecorrido) * 1000);
        printf("Botao %d entrou antes do tempo minimo\n", botao);
        setEstado(estadoPedestre);
        return;
    }
}

// Interrupção do botão 1
void botao1pressionado()
{
    time_t tempoBotaoPress = time(NULL);
    controlaAmareloPrincipal = 1;
    aguardaPedestre(1, tempo_farol_verde_prin, tempoBotaoPress, TEMPO_VERDE_MIN_PRINCIPAL, setEstado, VERMELHO_PRINCIPAL);
}

// Interrupção do botão 2
void botao2pressionado()
{
    time_t tempoBotaoPress = time(NULL);
    controlaAmareloAuxiliar = 1;
    aguardaPedestre(2, tempo_farol_verde_aux, tempoBotaoPress, TEMPO_VERDE_MIN_AUXILIAR, setEstado, VERDE_PRINCIPAL);
}

// Função para ativar buzzer
void ativarBuzzer(int pinoBuzzer)
{
    digitalWrite(pinoBuzzer, HIGH); // Ativa o buzzer
    delay(1000);
    digitalWrite(pinoBuzzer, LOW); // Desativa o buzzer
}

// Função que abre semaforo auxiliar assim que um carro para
void abrirAuxiliar(time_t tempoCarroParou)
{
    // marcador do tempo do farol verde principal == tempo do farol vermelho auxiliar
    double tempoDecorrido = difftime(tempoCarroParou, tempo_farol_verde_prin);

    if ((tempoDecorrido * 1000) >= (TEMPO_VERMELHO_MIN_AUXILIAR * 1000))
    {
        setEstado(VERMELHO_PRINCIPAL);
        return;
    }
    else
    {
        setEstado(VERMELHO_PRINCIPAL);
        return;
    }
}

// Monitoramento de tempo de botão com filtro
unsigned long medirDuracaoBotaoPressionado(int pinoBotao)
{
    unsigned long tempoPressionado = 0;
    int estadoAnterior = LOW;

    while (digitalRead(pinoBotao) == HIGH)
    {
        if (estadoAnterior == LOW)
        {
            tempoPressionado = millis();
        }
        estadoAnterior = HIGH;
    }

    unsigned long duracao = millis() - tempoPressionado;

    // Aplicar um filtro para evitar valores de tempo anômalos
    if (duracao < 10 || duracao > 3000)
    {
        duracao = 0; // Descarta leituras fora do intervalo desejado
    }
    return duracao;
}

// Função de sensor de velocidade
double calcularVelocidade(unsigned long duracaoSensor)
{
    // A distância média de um carro é de 2 metros
    double distanciaCarro = 2;

    // Converta o tempo do sensor para segundos (s)
    double duracaoSegundos = (double)duracaoSensor / 1000.0; // de ms para s

    // velocidade = distância / tempo
    double velocidadeCarroAtual = distanciaCarro / duracaoSegundos;

    velocidadeCarroAtual = velocidadeCarroAtual * 3.6; // de m/s para km/h

    return velocidadeCarroAtual;
}

// Verifica velocidade
void verificarVelocidadeExcessiva(double velocidadeMedida, int via, int sensor)
{
    double velocidadeMaxima;
    char nomeVia[10];

    // Defina a velocidade máxima com base na via (1 == principal ou 2 == auxiliar)
    if (via == 1)
    {
        velocidadeMaxima = LIMITE_VELOCIDADE_PRINCIPAL; // Defina a velocidade máxima da via principal
        strcpy(nomeVia, "Principal");
    }
    else
    {
        velocidadeMaxima = LIMITE_VELOCIDADE_AUXILIAR; // Defina a velocidade máxima da via auxiliar
        strcpy(nomeVia, "Auxiliar");
    }

    // Verifique se a velocidade medida é maior que a velocidade máxima
    if (velocidadeMedida > velocidadeMaxima)
    {
        printf("1 - Carro passou acima da velocidade permitida na via:%s\nno sensor:%d\n%.2f km/h\n", nomeVia, sensor, velocidadeMedida);
        acimaVelocidade++;
        ativarBuzzer(pinagem.BUZZER); // Ative o buzzer como sinal de alerta
    }
    else
    {
        printf("2 - Carro passou na velocidade permitida na via:%s\nno sensor:%d\n%.2f km/h\n", nomeVia, sensor, velocidadeMedida);
    }
}

// Avançou sinal vermelho
void avancouSinal(int sensor)
{
    printf("Passou no vermelho no cruzamento %d\n", sensor);
    avancouVermelho++;
    ativarBuzzer(pinagem.BUZZER);
}

void controlaSensoresAuxiliares(int pino, int sensor)
{
    unsigned long duracaoSensorAux = medirDuracaoBotaoPressionado(pino);
    double velocidade = calcularVelocidade(duracaoSensorAux);
    verificarVelocidadeExcessiva(velocidade, 2, sensor);

    if (duracaoSensorAux >= 1000)
    {
        controlaAmareloPrincipal = 1;
        time_t tempoEstado = time(NULL);
        abrirAuxiliar(tempoEstado);
    }
    else if (estadoAtual == VERDE_PRINCIPAL)
    {
        avancouSinal(sensor);
    }
    if (sensor == 1)
    {
        velocidadeTotalAuxiliar += velocidade;
        contagemCarrosAuxiliar++;
        carroDirAuxiliar1++;
    }
    else
    {
        velocidadeTotalAuxiliar += velocidade;
        contagemCarrosAuxiliar++;
        carroDirAuxiliar2++;
    }
}

void controlaSensoresPrincipais(int pino, int sensor)
{
    unsigned long duracaoSensorPrin = medirDuracaoBotaoPressionado(pino);
    double velocidade = calcularVelocidade(duracaoSensorPrin);
    verificarVelocidadeExcessiva(velocidade, 1, sensor);

    if (duracaoSensorPrin >= 1000)
    {
        printf("parou no principal %d\n", sensor);
    }
    else if (estadoAtual == VERMELHO_PRINCIPAL)
    {
        avancouSinal(sensor);
    }

    if (sensor == 1)
    {
        velocidadeTotalPrincipal += velocidade;
        contagemCarrosPrincipal++;
        carroDirPrincipal1++;
    }
    else
    {
        velocidadeTotalPrincipal += velocidade;
        contagemCarrosPrincipal++;
        carroDirPrincipal2++;
    }
}

// Thread para monitoramento das vias auxiliares e contagem de carros
void *monitorarSensoresAuxiliares(void *arg)
{
    while (1)
    {
        if (digitalRead(pinagem.SENSOR_AUX_1) == HIGH)
        {
            controlaSensoresAuxiliares(pinagem.SENSOR_AUX_1, 1);
        }
        else if (digitalRead(pinagem.SENSOR_AUX_2) == HIGH)
        {
            controlaSensoresAuxiliares(pinagem.SENSOR_AUX_2, 2);
        }
        delay(25);
    }

    return NULL;
}

// Thread para monitoramento das vias auxiliares e contagem de carros
void *monitorarSensoresPrincipais(void *arg)
{
    while (1)
    {
        if (digitalRead(pinagem.SENSOR_PRIN_1) == HIGH)
        {
            controlaSensoresPrincipais(pinagem.SENSOR_PRIN_1, 1);
        }
        else if (digitalRead(pinagem.SENSOR_PRIN_2) == HIGH)
        {
            controlaSensoresPrincipais(pinagem.SENSOR_PRIN_2, 2);
        }
        delay(25);
    }

    return NULL;
}

ssize_t receberDadosDoServidor(int clientSocket, char *buffer, size_t bufferSize)
{
    ssize_t bytesReceived = recv(clientSocket, buffer, bufferSize - 1, 0);
    if (bytesReceived < 0)
    {
        perror("Erro ao receber dados do servidor.");
    }
    else if (bytesReceived == 0)
    {
        printf("Servidor desconectado.\n");
    }
    else
    {
        buffer[bytesReceived] = '\0';  // Certifique-se de terminar a string corretamente
    }
    return bytesReceived;
}

void *escutaServidor(void *arg)
{
    int clientSocket = *(int *)arg;
    char mensagemServidor[1024];

    while (1)
    {
        ssize_t tamanhoMensagem = receberDadosDoServidor(clientSocket, mensagemServidor, sizeof(mensagemServidor));

        if (tamanhoMensagem > 0)
        {
            if (strcmp(mensagemServidor, "MODO_NOTURNO") == 0)
            {
                setEstado(MODO_NOTURNO);
            } else if (strcmp(mensagemServidor, "MODO_EMERGENCIA") == 0){
                setEstado(MODO_EMERGENCIA);
            } else if (strcmp(mensagemServidor, "NORMAL") == 0){
                setEstado(VERDE_PRINCIPAL);
            }
        }
        else if (tamanhoMensagem == 0)
        {
            printf("Servidor desconectado.\n");
            close(clientSocket);
    
            break;
        }
        else
        {
            perror("Erro ao receber dados do servidor.");
    
        }
    }

    return NULL;
}

void enviarDadosParaServidor(int clientSocket, const char *dados)
{
    ssize_t bytesSent = send(clientSocket, dados, strlen(dados), 0);
    if (bytesSent < 0)
    {
        perror("Erro ao enviar dados para o servidor.");
        close(clientSocket); // Fecha o socket se houver erro
    }
}

int connectToServer()
{
    struct sockaddr_in serverAddr;
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == -1)
    {
        perror("Erro ao criar o socket do cliente.");
        return -1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Erro ao conectar ao servidor.");
        close(clientSocket);
        return -1;
    }

    return clientSocket;
}

void *tentaConectar(void *arg)
{
    int *clientSocketPtr = (int *)arg;
    while (1)
    {
        *clientSocketPtr = connectToServer(); 
        if (*clientSocketPtr != -1)
        {
            pthread_t threadEscuta;
            if(pthread_create(&threadEscuta, NULL, escutaServidor, clientSocketPtr) != 0) 
            {
                perror("Erro ao criar thread de escuta.");
            }
            break; 
        }
        sleep(5); 
    }
    return NULL;
}

double calcularVelocidadeMedia(double velocidadeTotal, int contagemCarros)
{
    if (contagemCarros == 0)
        return 0; // Para evitar divisão por zero
    return velocidadeTotal / contagemCarros;
}

// Função unicamente pra voltar resultado dos carros
void *contagemCarros(void *arg)
{
    pthread_t threadConexao;
    int clientSocket = -1; // Inicializa como inválido
    char dados[1024];

    double velocidadeMediaPrincipal;
    double velocidadeMediaAuxiliar;

    // Inicie a thread para tentar se conectar em segundo plano
    pthread_create(&threadConexao, NULL, tentaConectar, &clientSocket);

    while (1)
    {
        // Certifique-se de ter uma conexão antes de tentar enviar dados
        if (clientSocket == -1)
        {
            sleep(1); // Espere um pouco antes de verificar novamente
            continue;
        }

        velocidadeMediaPrincipal = calcularVelocidadeMedia(velocidadeTotalPrincipal, contagemCarrosPrincipal);
        velocidadeMediaAuxiliar = calcularVelocidadeMedia(velocidadeTotalAuxiliar, contagemCarrosAuxiliar);

        printf("Qtd de carros que avançaram o vermelho %d\n", avancouVermelho);
        printf("Qtd de carros que passaram acima da velocidade %d\n", acimaVelocidade);
        printf("Velocidade média da via principal: %.2f km/h\n", velocidadeMediaPrincipal);
        printf("Velocidade média da via auxiliar: %.2f km/h\n", velocidadeMediaAuxiliar);

        snprintf(dados, sizeof(dados),
                 "-----------------------Informações--------------------------\n"
                 "Qtd. de carros avançaram o vermelho: %d\n"
                 "Qtd. de carros acima da velocidade: %d\n"
                 "Velocidade média da via principal: %.2f km/h\n"
                 "Velocidade média da via auxiliar: %.2f km/h\n"
                 "Qtd. na via Principal: %d\n"
                 "Qtd. na via Auxiliar: %d\n"
                 "------------------------------------------------------------\n",
                 avancouVermelho, acimaVelocidade,
                 velocidadeMediaPrincipal, velocidadeMediaAuxiliar,
                 carroDirPrincipal1 + carroDirPrincipal2,
                 carroDirAuxiliar1 + carroDirAuxiliar2);

        enviarDadosParaServidor(clientSocket, dados);

        delay(2000);
    }

    if (clientSocket != -1)
    {
        close(clientSocket);
    }
    return NULL;
}

// Função para atualizar o estado do semáforo
void atualizarSemaforo()
{
    switch (estadoAtual)
    {
    case VERDE_PRINCIPAL:
        tempo_farol_verde_prin = time(NULL);
        definirEstadoSemaforoPrin(0, 1); // Principal verde
        definirEstadoSemaforoAux(1, 1);  // Auxiliar Vermelho
        delay(TEMPO_VERDE_MAX_PRINCIPAL * 1000);
        if (controlaAmareloPrincipal == 0)
        {
            estadoAtual = AMARELO_PRINCIPAL;
        }
        else
        {
            controlaAmareloPrincipal = 0;
            estadoAtual = VERMELHO_PRINCIPAL;
        }
        break;

    case AMARELO_PRINCIPAL:
        ativarBuzzer(pinagem.BUZZER);
        definirEstadoSemaforoPrin(1, 0); // Principal amarelo
        delay(TEMPO_AMARELO * 1000);
        estadoAtual = VERMELHO_PRINCIPAL;
        break;

    case VERMELHO_PRINCIPAL:
        tempo_farol_verde_aux = time(NULL);
        definirEstadoSemaforoPrin(1, 1); // Principal Vermelho
        definirEstadoSemaforoAux(0, 1);  // Auxiliar Verde
        delay(TEMPO_VERMELHO_MAX_PRINCIPAL * 1000);
        if (controlaAmareloAuxiliar == 0)
        {
            estadoAtual = AMARELO_AUXILIAR;
        }
        else
        {
            controlaAmareloAuxiliar = 0;
            estadoAtual = VERDE_PRINCIPAL;
        }
        break;

    case AMARELO_AUXILIAR:
        ativarBuzzer(pinagem.BUZZER);
        definirEstadoSemaforoAux(1, 0); // Auxiliar amarelo
        delay(TEMPO_AMARELO * 1000);
        estadoAtual = VERDE_PRINCIPAL;
        break;
    
     case MODO_NOTURNO:
            while (1) {
                definirEstadoSemaforoAux(1,0);
                definirEstadoSemaforoPrin(1,0);
                delay(2000);
                definirEstadoSemaforoAux(0,0);
                definirEstadoSemaforoPrin(0,0);
                delay(2000);
            }
            break;
    
    case MODO_EMERGENCIA:
            while (1) {
                definirEstadoSemaforoAux(1,1);
                definirEstadoSemaforoPrin(0,1);
                delay(2000);
            }
            break;
    }
}

// Função para definir manualmente o estado do semáforo
void setEstado(enum EstadosSemaforo novoEstado)
{
    pthread_mutex_lock(&mutex);
    if (novoEstado == MODO_NOTURNO || novoEstado == MODO_EMERGENCIA) {
        pausarThreadPrincipal = 1;
    } else {
        pausarThreadPrincipal = 0;
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);
    
    estadoAtual = novoEstado;
    atualizarSemaforo();
}

// Loop de temporização principal
void temporizador()
{
    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (pausarThreadPrincipal) {
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);
        
        atualizarSemaforo();
    }
}

// Configurando pinos e inicialização
void configurarGPIO()
{
    pinMode(pinagem.VERDE_SEMAFORO_1, OUTPUT);
    pinMode(pinagem.AMARELO_SEMAFORO_1, OUTPUT);
    pinMode(pinagem.VERDE_SEMAFORO_2, OUTPUT);
    pinMode(pinagem.AMARELO_SEMAFORO_2, OUTPUT);
    pinMode(pinagem.BOTAO_PED_1, INPUT);
    pinMode(pinagem.BOTAO_PED_2, INPUT);
    pinMode(pinagem.SENSOR_PRIN_1, INPUT);
    pinMode(pinagem.SENSOR_PRIN_2, INPUT);
    pinMode(pinagem.SENSOR_AUX_1, INPUT);
    pinMode(pinagem.SENSOR_AUX_2, INPUT);
    pinMode(pinagem.BUZZER, OUTPUT);
    pullUpDnControl(pinagem.BOTAO_PED_1, PUD_UP);
    pullUpDnControl(pinagem.BOTAO_PED_2, PUD_UP);
    pullUpDnControl(pinagem.SENSOR_PRIN_1, PUD_DOWN);
    pullUpDnControl(pinagem.SENSOR_PRIN_2, PUD_DOWN);
    pullUpDnControl(pinagem.SENSOR_AUX_1, PUD_DOWN);
    pullUpDnControl(pinagem.SENSOR_AUX_2, PUD_DOWN);
    wiringPiISR(pinagem.BOTAO_PED_1, INT_EDGE_RISING, &botao1pressionado);
    wiringPiISR(pinagem.BOTAO_PED_2, INT_EDGE_RISING, &botao2pressionado);
}
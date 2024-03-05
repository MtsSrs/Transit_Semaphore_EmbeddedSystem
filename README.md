# Documentação - Matheus Soares - Trabalho 1

### Link para vídeo
[Video 1 - Trabalho](https://www.youtube.com/watch?v=oUPT0T6NXOU)

## 1. Como rodar
Após clonar o repositório basta colocar seu conteúdo dentro da placa

### 1.1 Servidor Distribuido
```bash
cd servDist
make
```
Como o cruzamento é configurado através de um documento txt, basta passar como arg no console qual o cruzamento será usado

Cruzamento 1:
```
./program ./configs/cruzamento1.txt
```
Cruzamento 2:
```
./program ./configs/cruzamento2.txt
```

### 1.2 Servidor Central
```bash
cd servCent
make
./central
```

Feito essas duas etapas o projeto está rodando

## 2. O que foi feito
|   ITEM    |   DETALHE  |   Feito  |
|-----------|------------|:---------:|
|**Servidor Central**    |       |       |
|**Interface (Monitoramento)**  |  Interface gráfica (via terminal, web, etc) apresentando os dados de **Fluxo de trânsito**, **Velocidade média da via** e **número de infrações** por cruzamento.  |   Foi feito como pedido.  |
|**Interface (Comandos)** | Mecanismo para acionar e desacionar o **Modo de emergência** e o **Modo noturno**. |   Na versão final ocorre um certo bug no ativamento dos modos, mas foi feito.   |
|**Servidores Distribuídos**    |       |       |
|**Controle dos Semáforos**    |  Controle do mecanismo de temporização dos semáforos seguindo a temporização (Tabela 2).  |   Feito como pedido.  |
|**Botões de travessia** |   Detecção dos botões de travessia de pedestres reduzindo o tempo de abertura do semáforo.    |   Feito como pedido.   |
|**Sensor de Passagem de Carros** |  Detecção dos sensores de passagem de carros (Vias laterais) detectando carros esperando no sinal vermelho, reduzindo o tempo de abertura do semáforo e detecção de infração por avanço de sinal vermelho (com alarme). |   Feito como pedido.  |
|**Sensor de Velocidade** |  Detecção da velocidade dos carros na via principal, reportando ao sersor central, quanto identificando, as infrações por velocidade e avanço de sinal vermelho. |   Feito como pedido. |
|**Geral**    |       |       |
|**Comunicação TCP/IP**  |   Correta implementação de comunicação entre os servidores usando o protocolo TCP/IP. Incluindo a inicialização dos serviços em qualquer ordem e o re-estabelecimento de conexão após queda de qualquer serviço. |   Foi feita a comunicação mas caso caia não fui capaz de fazer a reconexão.   |
|**Qualidade do Código / Execução** |   Utilização de boas práticas como o uso de bons nomes, modularização e organização em geral, bom desempenho da aplicação sem muito uso da CPU. |  O código ta modularizado (Funções) e usa menos de 5% da CPU, mas está mal organizado. |
|**Pontuação Extra** |   Qualidade e usabilidade acima da média. |   Não avaliado por mim.   |  

[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-718a45dd9cf7e7f842a935f5ebbe5719a5e09af4491e668f4dbf3b35d5cca122.svg)](https://classroom.github.com/online_ide?assignment_repo_id=12017606&assignment_repo_type=AssignmentRepo)
# Trabalho 1 (2023-2)

Trabalho 1 da disciplina de Fundamentos de Sistemas Embarcados (2023/2)

## 1. Objetivos

Este trabalho tem por objetivo a criação de um sistema distribuído para o controle e monitoramento de um grupo de cruzamentos de sinais de trânsito. O sistema deve ser desenvolvido para funcionar em um conjunto de placas Raspberry Pi com um ***servidor central*** responsável pelo controle e interface com o usuário controlador e ***servidores distribuídos*** para o controle local e monitoramento dos sinais do cruzamento junto aos respectivos sensores que monitoram as vias. Dentre os dispositivos envolvidos estão: o controle de temporizaçãio e acionamento dos sinais de trânsito, o acionmento de botões de passagens de pedestres, o monitoramento de sensores de passagem de carros bem como a velocidade da via e o avanço de sinal vermelho.

A Figura 1 ilustra cruzamentos de trânsito.

![Figura](https://img.freepik.com/free-vector/colored-isolated-city-isometric-composition-with-road-crosswalk-city-center-vector-illustration_1284-30528.jpg)
![Figura](https://upload.wikimedia.org/wikipedia/commons/7/75/Makati_intersection.jpg)

Cada cruzamento possui:
- 4 Sinais de Trânsito (Em pares);
- 2 botões de acionamento para pedestres (pedir passagem), uma para cada direção;
- 2 Sensores de velocidade/presença/passagem de carros (nas vias auxiliares, um em cada direção);
- 2 Sensores de velocidade/presença/passagem de carros (nas vias principais, um em cada direção);
- 1 Sinalização de áudio (buzzer) para sinalizar quando o sinal está mudando de estado (quando o cruzamento de pedestres irá ser fechado);

Cada cruzamento deverá ser controlado por um processo individual que esteja rodando em uma placa Raspberry Pi e cada controlador de cruzamento deve se comunicar via rede (TCP/IP) com o servidor central.

Na Figura 2 é possível ver a arquitetura do sistema.

![Figura](/figuras/arquitetura_trabalho_1.png)

## 2. Componentes do Sistema

Para simplificar a implementação e logística de testes do trabalho, a quantidade de cruzamentos será limitada a 2 sendo que devem ser implementados 2 serviços de controle dos cruzamentos e um servidor central. 

### O sistema do Servidor Central será composto por:
1. 01 Placa Raspberry Pi 3/4;

### Cada unidade dos Servidores Distribuídos será composto por:
1. 01 Placa Raspberry Pi 3/4;
2. 04 Saídas GPIO (LEDs) representando os semáforos;
3. 02 Entradas sendo os botões de pedestre;
4. 02 Entradas sendo os sensores de velocidade/presença/contagem de veículos das vias auxiliares (2 por cruzamento);
5. 02 Entradas sendo os sensores de velocidade/presença/contagem (4 por cruzamento);
6. Saída de áudio para efeito sonoro estado do sinal para deficientes auditivos;

## 3. Conexões entre os módulos do sistema

1. Os servidores distribuídos deverão se comunicar com o servidor central através do Protocolo TCP/IP (O formato das mensagens ficam à cargo do aluno. A sugestão é o uso do formato JSON);
2. Cada instância do servidor distribuído (uma por cruzamento) deve rodar em um processo paralelo em portas distintas, podendo ser executado em placas distintas; 
4. Cada entrada / saída está representada na Tabela abaixo. Cada servidor distribuído é responsável pelo controle de um cruzamento.

<center>
Tabela 1 - Pinout da GPIO da Raspberry Pi
</center>
<center> 


| Item                     | GPIO Cruzamento 1 | GPIO Cruzamento 2 | Direção |
|--------------------------|:----:|:----:|:-------:|
| Semáforo 1 - Pino 1      |  09  | 10 | Saída   |
| Semáforo 1 - Pino 2      |  11  | 08 | Saída   |
| Semáforo 2 - Pino 1      |  05  | 01 | Saída   |
| Semáforo 2 - Pino 2      |  06  | 18 | Saída   |
| Botão de Pedestre 1      |  13  | 23 | Entrada |
| Botão de Pedestre 2      |  19  | 25 | Entrada |
| Sensor Via Auxiliar 1    |  26  | 25 | Entrada |
| Sensor Via Auxiliar 2    |  22  | 12 | Entrada |
| Sensor Via Principal 1   |   0  | 16 | Entrada |
| Sensor Via Principal 1   |  27  | 20 | Entrada |
| Buzzer                   |  17  | 21 | Saída   |

</center> 

<!-- [Link do Dashboard - Cruzamento 1](http://164.41.98.25:443/dashboard/0fe7b8e0-031e-11ed-9f25-414fbaf2b065?publicId=ba042a80-0322-11ed-9f25-414fbaf2b065)  
[Link do Dashboard - Cruzamento 2](http://164.41.98.25:443/dashboard/d0680ee0-06d3-11ed-b55b-052a89b3b188?publicId=ba042a80-0322-11ed-9f25-414fbaf2b065)  
[Link do Dashboard - Cruzamento 3](http://164.41.98.25:443/dashboard/35007810-06d4-11ed-b55b-052a89b3b188?publicId=ba042a80-0322-11ed-9f25-414fbaf2b065)  
[Link do Dashboard - Cruzamento 4](http://164.41.98.25:443/dashboard/59bd6050-06d4-11ed-b55b-052a89b3b188?publicId=ba042a80-0322-11ed-9f25-414fbaf2b065)   -->


## 4. Requisitos

Os sistema de controle possui os seguintes requisitos:

### **Servidores Distribuídos**

O código do Servidor Distribuído deve ser desenvolvido em **Python**, **C** ou **C++**.

Os servidores distribuídos tem as seguintes responsabilidades:  
1. Controlar os **semáforos** (temporização) - cruzamento com 4 sinais: os semáforos da via principal tem temporização diferente dos das vias auxiliares conforme e tabela abaixo.
   
<center>
Tabela 2 - Temporização dos Semáforos
</center>
<center> 

| Estado                                            | Via Principal (s) | Via Auxiliar (s) | 
|---------------------------------------------------|:----:|:---:|
| Verde (mínimo)                                    |  10  |  5  |
| Verde (máximo)                                    |  20  | 10  |
| Amarelo                                           |   2  |  2  |
| Vermelho (mínimo)                                 |   5  | 10  |
| Vermelho (máximo)                                 |  10  | 20  |
<!-- | Vermelho Total (Vermelho em ambas as direções)    |   1  |  1  | -->

</center> 

2. Controlar o acionamento dos **botões de travessia** de pedestres (2 por cruzamento): ao acionar o botão, o sinal em questão deverá cumprir seu tempo mínimo (Ex: permanecer verde pelo tempo mínimo antes de fechar. Caso o tempo mínimo já tenha passado, o sinal irá mudar de estado imediatamente após o botão ser pressionado);
3. Controlar o acionamento dos **sensores de passagem de carros** nas vias auxiliares. Caso o sinal esteja fechado e um carro pare na via auxiliar, o comportamento será o mesmo que um pedestre pressionar o **botões de travessia**;
4. Contar a *passagem de carros* em cada direção e sentido do cruzamento (4 valores sepadados) e enviar esta informação periodicamente (2 segundos) ao servidor central;
5. Monitorar a velocidade das vias (principal e auxiliar) através dos **sensores de velocidade**. A velocidade de cada carro deverá ser reportada para o servidor central periodicamente. Veídulos acima da velocidade permitida deverão ser reportados ao servidor central e contabilizados separadamente. Além disso, é necessário soar um alarme ao detectar um veículo acima da velocidade permitida;  
Velocidade Máxima:  
    a. Via Principal: 80 km/h;  
    b. Via auxiliar: 60 km/h.
6. Efetuar o controle de *avanço do sinal vermelho* em ambas as vias. O número de veículos que avançam o sinal vermelho deverá ser reportado ao servidor central e o alarme deve ser disparado a cada detecção de infração;
7. Cada instância dos servidores distribuídos a ser executada deve automaticamente se configurar para o controle do cruzamento 1 ou 2 à partir de um arquivo de configuração (configuração de portas, pinos, etc).

### **Servidor Central**

O código do Servidor Central pode ser desenvolvido em **Python**, **C** ou **C++**. 

O servidor central tem as seguintes responsabilidades:  
1. Manter conexão com os servidores distribuídos (TCP/IP);  
2. Prover uma **interface** que mantenham atualizadas as seguintes informações por cruzamento:  
    a. **Fluxo de trânsito** na via principal e em cada via auxiliar (Carros/min);    
    b. **Velocidade média de cada via** (km/h);   
    c. **Número de infrações** (Por tipo: avanço de sinal e velocidade acima da permitida);  
3. Prover **mecanismo na interface** para:  
    a. **Modo de emergência**: liberar o fluxo de trânsito em uma via (os dois cruzamentos com a via principal em verde);     
    b. **Modo noturno** fazer o sinal amarelo piscar em todos os cruzamento;  
4. Armazenar de modo persistente (arquivo) o estado atual (número de infrações, velocidade da pista, etc) para que possa ser re-estabelecido em caso de queda de energia.

### **Geral**

1. Em qualquer uma das linguagens devem haver instruções explicitas de como instalar e rodar;  
2. Para C/C++ é mandatório o uso do Makefile e incluir todas as dependências no próprio projeto;  
3. Cada serviço (programa) deve poder ser iniciado independente dos demais e ficar aguardando o acionamento dos demais;  
4. Qualquer queda de comunicação entre os serviços, seja por falta de energia, erro de comunicação, etc deverá ser re-estabelecida automaticamente assim que o serviço voltar ao ar, sem perda de função.
5. Deverá haver um arquivo README no repositório descrevento o modo de instalação/execução e o modo de uso do programa.

## 5. Detalhes de Implementação

1. **Semáforos**: cada semáforo será controlado por dois pinos da GPIO e um multiplexador.

<center>
Tabela 3 - Tabela Verdade dos Semáforos
</center>
<center> 

| Semáforo      | Pino 1 | Pino 2 | 
|---------------|:------:|:------:|
| Desligado     |  0     |  0     |
| Verde         |  0     |  1     |
| Amarelo       |  1     |  0     |
| Vermelho      |  1     |  1     |

</center> 

2. **Botão de travessia de pedestre**: devem tratar o *debounce*. No simulador, o sinal do botão é acionado por um intervalo de 300 a 400 ms. O sinal é normalmente em baixa e ativado em alta \___|‾|___ .
3. **Sensor de Velocidade**: estes sensores são implementados através do sensor de efeito hall. O sensor de velocidade é um sensor que é acionado tanto na borda de subida \___|‾ quanto na borda de descida ‾‾|_ . O comprimento médio dos carros é de  2 metros. Portanto, na passagem de um carro, o sensor é acionado primeiro pela borade de subida e depois pela borda de descida. Neste caso, para calcular a velocidade do carro passando pelo sensor, é necessário calcular o intervalo de tempo entre o acionamentdo da borda de subida e a de descida. Em seguida, dividir a distância (2 metros) pelo intervalo de tempo medido.   
**Obs**: No simulador, o intervalo de tempo entre a ativação da borda de subida e a de descida é de no mínimo 15 ms e no máximo 300 ms. O sinal do sensor permanece em baixa (0.0V) enquanto está inativo e em alta (3.3V) quando ativado \___|‾|_ .

## 6. Critérios de Avaliação

A avaliação será realizada seguindo os seguintes critérios: 

<center>
Tabela 4 - Avaliação
</center>
<!-- 
|   ITEM    |   DETALHE  |   VALOR   |
|-----------|------------|:---------:|
|**Servidor Central**    |       |       |
|**Interface (Monitoramento)**  |  Interface gráfica (via terminal, web, etc) apresentando os dados de **Fluxo de trânsito**, **Velocidade média da via** e **número de infrações** por cruzamento.  |   1,0   |
|**Interface (Comandos)** | Mecanismo de acionar e desacionar o **Modo de emergência** e o **Modo noturno**. |   1,0   |
|**Servidores Distribuídos**    |       |       |
|**Controle dos Semáforos**    |  Controle do mecanismo de temporização dos semáforos seguindo a temporização (Tabela 2).  |   1,0   |
|**Botões de travessia** |   Detecção dos botões de travessia de pedestres reduzindo o tempo de abertura do semáforo (incluindo o *debounce*).    |   1,5   |
|**Sensor de Passagem de Carros** |  Detecção dos sensores de passagem de carros (Vias laterais) detectando carros esperando no sinal vermelho, reduzindo o tempo de abertura do semáforo e detecção de infração por avanço de sinal vermelho (com alarme). |   1,0  |
|**Sensor de Velocidade** |  Detecção da velocidade dos carros na via principal, reportando ao sersor central, quanto identificando, as infrações por velocidade e avanço de sinal vermelho (incluindo o alarme). |   1,5  |
|**Geral**    |       |       |
|**Comunicação TCP/IP**  |   Correta implementação de comunicação entre os servidores usando o protocolo TCP/IP. |   1,5   |
|**Qualidade do Código / Execução** |   Utilização de boas práticas como o uso de bons nomes, modularização e organização em geral, bom desempenho da aplicação sem muito uso da CPU. |  1,5 |
|**Pontuação Extra** |   Qualidade e usabilidade acima da média. |   0,5   |   -->

## 7. Referências

### Bibliotecas em Python

- gpiozero (https://gpiozero.readthedocs.io)
- RPi.GPIO (https://pypi.org/project/RPi.GPIO/)

A documentação da RPi.GPIO se encontra em
https://sourceforge.net/p/raspberry-gpio-python/wiki/Examples/

### Bibliotecas em C/C++

- WiringPi (http://wiringpi.com/)
- BCM2835 (http://www.airspayce.com/mikem/bcm2835/)
- PiGPIO (http://abyz.me.uk/rpi/pigpio/index.html)
- sysfs (https://elinux.org/RPi_GPIO_Code_Samples)

### Lista de Exemplos

Há um compilado de exemplos de acesso à GPIO em várias linguages de programação como C, C#, Ruby, Perl, Python, Java e Shell (https://elinux.org/RPi_GPIO_Code_Samples).

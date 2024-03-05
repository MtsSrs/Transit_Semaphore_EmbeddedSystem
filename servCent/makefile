# Nome do arquivo de saída (executável)
TARGET = central

# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall

# Bibliotecas
LIBS = -lpthread

# Arquivos fonte
SRCS = socketCentral.c

# Objetos (resultados da compilação dos arquivos fonte)
OBJS = $(SRCS:.c=.o)

# Regra padrão
all: $(TARGET)

# Regra para compilar o programa
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Regra para compilar os arquivos fonte em arquivos objeto
.c.o:
	$(CC) $(CFLAGS) -c $<

# Regra para limpar arquivos compilados
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

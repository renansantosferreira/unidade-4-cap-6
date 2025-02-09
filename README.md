# Projeto: Controle de Display OLED, LEDs WS2812 e Botões com Raspberry Pi Pico W

## Descrição
Este projeto utiliza o **Raspberry Pi Pico W** para controlar uma matriz de LEDs WS2812, um display OLED SSD1306 e dois botões para interação. A comunicação é feita via **I2C** para o display, **PIO** para os LEDs endereçáveis e **GPIO** para os botões e LEDs individuais.

## Componentes Utilizados
- Raspberry Pi Pico W
- Display OLED SSD1306 (I2C)
- Matriz 5x5 de LEDs WS2812 (endereçáveis)
- LEDs individuais (Verde e Azul)
- Botões (A e B)
- Conexões via GPIO

## Bibliotecas Utilizadas
- `pico/stdlib.h` - Biblioteca padrão para entrada e saída
- `hardware/i2c.h` - Biblioteca para comunicação I2C
- `hardware/gpio.h` - Biblioteca para manipulação de GPIOs
- `ssd1306.h` - Biblioteca para controle do display OLED SSD1306
- `ws2812.h` - Biblioteca para controle dos LEDs WS2812

## Configuração de Pinos
| Componente     | GPIO | Função |
|---------------|------|---------|
| I2C SDA      | 14   | Dados I2C |
| I2C SCL      | 15   | Clock I2C |
| Display OLED | 14, 15 | I2C |
| WS2812       | 7    | Matriz de LEDs |
| LED Verde    | 11   | Saída Digital |
| LED Azul     | 12   | Saída Digital |
| Botão A     | 5    | Entrada Digital |
| Botão B     | 6    | Entrada Digital |

## Funcionalidades Implementadas
### 1. Controle de Números na Matriz de LEDs
Cada número (0-9) é representado em um array de **25 LEDs** (5x5). O usuário pode digitar um número via **UART** (monitor serial), e ele será exibido na matriz de LEDs e no display OLED.

### 2. Interação com os Botões
- **Botão A (GPIO 5):** Alterna o estado do **LED Verde** e exibe no display OLED e monitor serial.
- **Botão B (GPIO 6):** Alterna o estado do **LED Azul** e exibe no display OLED e monitor serial.
- Foi implementado um sistema de **debounce via software** para evitar leituras falsas dos botões.

### 3. Atualização do Display OLED
O display OLED exibe informações sobre:
- Estado dos LEDs
- Números digitados via terminal
- Mensagens de status

## Fluxo do Programa
1. Inicializa os periféricos (I2C, GPIO, UART, PIO para WS2812).
2. Aguarda entrada do usuário via UART para exibir números na matriz.
3. Monitora os botões para alternar o estado dos LEDs.
4. Atualiza o display OLED conforme as mudanças ocorrem.

## Como Executar o Projeto
1. Compilar e carregar o programa no Raspberry Pi Pico W usando o **Pico SDK**.
2. Conectar via **monitor serial** (baudrate: 115200) para enviar entradas.
3. Pressionar os botões para testar o controle de LEDs.

## Melhorias Futuras
- Adição de novos caracteres para a matriz de LEDs.
- Expansão do sistema para permitir entradas mais complexas.
- Integração com comunicação Wi-Fi para controle remoto.

[Clique aqui ⬆️ e assista o vídeo](https://youtu.be/xxxxx)

## Autor
Desenvolvido por [Renan Ferreira].


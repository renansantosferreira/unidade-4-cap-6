#include <stdio.h>       // Biblioteca padrão para entrada e saída
#include <stdint.h>      // Biblioteca para tipos de dados inteiros de largura fixa
#include "pico/stdlib.h" // Biblioteca padrão do Raspberry Pi Pico
#include "hardware/i2c.h" // Biblioteca para comunicação I2C
#include "hardware/gpio.h" // Biblioteca para manipulação de GPIOs
#include "ssd1306.h"     // Biblioteca para controle do display OLED SSD1306
#include "ws2812.h"      // Biblioteca para controle dos LEDs WS2812 (endereçáveis)

// Definição de pinos
#define I2C_SDA_PIN 14     // Pino de dados I2C
#define I2C_SCL_PIN 15     // Pino de clock I2C
#define I2C_ADDR 0x3C      // Endereço do display SSD1306
#define WS2812_PIN 7       // Pino dos LEDs WS2812
#define LED_GREEN_PIN 11   // Pino do LED verde
#define LED_BLUE_PIN 12    // Pino do LED azul
#define BUTTON_A_PIN 5     // Pino do botão A
#define BUTTON_B_PIN 6     // Pino do botão B
#define MATRIX_PIN 7       // Pino da matriz de LEDs (mesmo pino dos WS2812)
#define INTENSIDADE 10     // Intensidade dos LEDs WS2812
#define MATRIX_SIZE 25     // Número total de LEDs na matriz 5x5

// Variáveis globais
volatile bool led_green_state = false; // Estado do LED verde
volatile bool led_blue_state = false;  // Estado do LED azul
ssd1306_t disp; // Estrutura para o display OLED
PIO pio = pio0; // PIO usado para controlar os LEDs WS2812
int sm; // Máquina de estado PIO
uint32_t leds[MATRIX_SIZE]; // Buffer para armazenar os valores dos LEDs da matriz
int numero_atual = 0; // Número atualmente exibido

// Matriz de representação dos números de 0 a 9 (5x5)
const uint32_t numeros[10][25] = {
   // 0 
    {1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},

    // 1 
    {1, 1, 1, 1, 1,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     0, 1, 1, 0, 0,
     0, 0, 1, 0, 0},

    // 2 
    {1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     0, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     0, 1, 1, 1, 1},

    // 3 
    {1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 0, 0,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1},

    // 4 
    {1, 0, 0, 0, 0,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 0, 0, 0, 1},

    // 5 
    {1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1},

    // 6 
    {1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 0,
     1, 1, 1, 1, 1},

    // 7 
    {0, 0, 0, 0, 1,
     0, 1, 0, 0, 0,
     0, 0, 1, 0, 0,
     0, 0, 0, 1, 0,
     1, 1, 1, 1, 1},

    // 8 
    {1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1},

    // 9 
    {1, 1, 1, 1, 1,
     0, 0, 0, 0, 1,
     1, 1, 1, 1, 1,
     1, 0, 0, 0, 1,
     1, 1, 1, 1, 1}
};

// Função para exibir um número na matriz de LEDs WS2812 e no display SSD1306
void exibir_numero(int num) {
    // Atualiza a matriz de LEDs com o número selecionado
    for (int i = 0; i < MATRIX_SIZE; i++) {
        leds[i] = numeros[num][i] ? (INTENSIDADE << 8 | INTENSIDADE << 4) : 0x000000;
    }
    // Envia os valores para os LEDs WS2812 via PIO
    for (int i = 0; i < MATRIX_SIZE; i++) {
        pio_sm_put_blocking(pio, sm, leds[i]);
    }

    // Atualiza o display OLED para mostrar o número
    ssd1306_clear(&disp);
    ssd1306_draw_string(&disp, 1, 24, 2, "Numero: ");
    ssd1306_draw_char(&disp, 100, 24, 2, num + '0'); // Converte para caractere ASCII
    ssd1306_show(&disp);
}

// Função de debounce para os botões
bool debounce_callback(struct repeating_timer *t) {
    int pin = (int)t->user_data;
    if (gpio_get(pin) == 0) { // Verifica se o botão ainda está pressionado
        if (pin == BUTTON_A_PIN) {
            led_green_state = !led_green_state; // Alterna o estado do LED verde
            gpio_put(LED_GREEN_PIN, led_green_state);
            ssd1306_clear(&disp);
            ssd1306_draw_string(&disp, 10, 24, 2, led_green_state ? "Verde ON" : "Verde OFF");
            ssd1306_show(&disp);
            printf("LED Verde: %s\n", led_green_state ? "Ligado" : "Desligado");
        } else if (pin == BUTTON_B_PIN) {
            led_blue_state = !led_blue_state; // Alterna o estado do LED azul
            gpio_put(LED_BLUE_PIN, led_blue_state);
            ssd1306_clear(&disp);
            ssd1306_draw_string(&disp, 10, 24, 2, led_blue_state ? "Azul ON" : "Azul OFF");
            ssd1306_show(&disp);
            printf("LED Azul: %s\n", led_blue_state ? "Ligado" : "Desligado");
        }
    }
    return false;
}

// Função de interrupção dos botões
void gpio_callback(uint gpio, uint32_t events) {
    static struct repeating_timer debounce_timer;
    add_repeating_timer_ms(50, debounce_callback, (void*)gpio, &debounce_timer); // Evita múltiplos acionamentos rápidos
}

// Função para ler caracteres do terminal via UART
void ler_caracteres_terminal() {
    char c;
    if (stdio_usb_connected()) { // Verifica se o USB está conectado
        if (scanf("%c", &c) == 1) { // Lê um caractere do terminal
            if (c >= '0' && c <= '9') { // Verifica se é um número
                numero_atual = c - '0';  // Converte o caractere para um número inteiro
                exibir_numero(numero_atual);  // Atualiza a matriz de LEDs e o display
                printf("Tecla digitada: %c\n", c); // Exibe no monitor serial
            }
        }
    }
}

// Função de configuração inicial do sistema
void setup() {
    stdio_init_all(); // Inicializa a comunicação serial
    i2c_init(i2c1, 400000); // Inicializa o barramento I2C com 400 kHz
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    ssd1306_init(&disp, 128, 64, I2C_ADDR, i2c1); // Inicializa o display SSD1306
    ssd1306_clear(&disp);
    ssd1306_show(&disp);

    // Configuração dos LEDs
    gpio_init(LED_GREEN_PIN);
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);

    // Configuração dos botões
    gpio_init(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_pull_up(BUTTON_B_PIN);

    // Configuração da UART0 para comunicação serial
    uart_init(uart0, 115200);  // Inicializa a UART0 com taxa de 115200 bps
    gpio_set_function(0, GPIO_FUNC_UART); // Configura GPIO 0 como RX
    gpio_set_function(1, GPIO_FUNC_UART); // Configura GPIO 1 como TX

    // Configuração das interrupções dos botões
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Inicializa os LEDs WS2812 usando PIO
    uint offset = pio_add_program(pio, &ws2812_program); // Adiciona o programa PIO
    sm = pio_claim_unused_sm(pio, true); // Obtém uma máquina de estado PIO livre
    ws2812_program_init(pio0, sm, offset, MATRIX_PIN, 800000, false); // Configura o WS2812
}

// Função principal
int main() {
    setup(); // Configura os periféricos e inicializa o sistema
    while (1) {
        ler_caracteres_terminal();  // Lê números digitados via terminal serial
        sleep_ms(100);  // Pequeno delay para reduzir uso da CPU
    }
}

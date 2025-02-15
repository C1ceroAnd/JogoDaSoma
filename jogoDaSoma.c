#include <stdio.h>           // Biblioteca padrão de entrada e saída
#include <stdlib.h>          // Biblioteca para manipulação de memória e funções rand()
#include <string.h>          // Biblioteca para manipulação de strings
#include "pico/stdlib.h"     // Biblioteca padrão do Raspberry Pi Pico
#include "hardware/timer.h"  // Biblioteca para manipulação de timers no RP2040
#include "hardware/pwm.h"    // Biblioteca para controle de PWM
#include "hardware/clocks.h" // Biblioteca para manipulação de clocks
#include "hardware/gpio.h"   // Biblioteca para manipulação de GPIOs
#include "hardware/i2c.h"    // Biblioteca para comunicação I2C
#include "inc/ssd1306.h"     // Biblioteca para comunicação com o display OLED

// Definição dos pinos utilizados no projeto
#define BUTTON_A 5  // Botão A - Resposta "Correto"
#define BUTTON_B 6  // Botão B - Resposta "Incorreto"
#define BUZZER 21   // Buzzer para emitir som ao errar
#define I2C_SDA 14  // Pino SDA para o display OLED
#define I2C_SCL 15  // Pino SCL para o display OLED

// Variáveis globais para controle do jogo
bool game_running = false;  // Indica se o jogo está em execução
int score = 0;              // Placar de acertos
volatile bool buzzer_active = false; // Indica se o buzzer está ativo
volatile bool both_buttons_pressed = false; // Indica se ambos os botões foram pressionados juntos

/**
 * Exibe um texto no display OLED, quebrando linhas automaticamente.
 * Cada linha comporta até 15 caracteres.
 *
 * @param text Mensagem a ser exibida no display
 */
void display_text(const char *text) {
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1};

    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    int y = 0;
    int line_len = 15;
    char line_buffer[16];
    int text_len = strlen(text);

    for (int i = 0; i < text_len; i += line_len) {
        strncpy(line_buffer, text + i, line_len);
        line_buffer[line_len] = '\0';
        ssd1306_draw_string(ssd, 2, y, line_buffer);
        y += 8;
        if (y >= ssd1306_height)
            break;
    }

    render_on_display(ssd, &frame_area);
}

/**
 * Inicializa o PWM no pino do buzzer.
 *
 * @param pin Pino do buzzer
 */
void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);
}

/**
 * Callback chamado para desligar o buzzer após um tempo determinado.
 *
 * @return Retorna 0 (sem efeito sobre o sistema de alarmes)
 */
int64_t stop_buzzer(alarm_id_t id, void *user_data) {
    pwm_set_gpio_level(BUZZER, 0);
    buzzer_active = false;
    return 0;
}

/**
 * Emite um som curto no buzzer.
 *
 * @param frequency Frequência do som (Hz)
 * @param duration_ms Duração do som (ms)
 */
void buzzer_beep(uint frequency, uint duration_ms) {
    if (buzzer_active)
        return;

    uint slice_num = pwm_gpio_to_slice_num(BUZZER);
    uint32_t clock_freq = clock_get_hz(clk_sys);
    uint32_t top = clock_freq / frequency - 1;

    pwm_set_wrap(slice_num, top);
    pwm_set_gpio_level(BUZZER, top / 2);
    buzzer_active = true;

    add_alarm_in_ms(duration_ms, stop_buzzer, NULL, false);
}

/**
 * Gera uma nova pergunta de soma aleatória.
 *
 * @param num1 Primeiro número da operação
 * @param num2 Segundo número da operação
 * @param result Resultado gerado
 * @param is_correct Indica se o resultado gerado é correto
 */
void generate_question(int *num1, int *num2, int *result, bool *is_correct) {
    *num1 = rand() % 10 + 1;
    *num2 = rand() % 10 + 1;
    *is_correct = rand() % 2;

    if (*is_correct) {
        *result = *num1 + *num2;
    } else {
        *result = *num1 + *num2 + (rand() % 5 - 2); // Resultado incorreto
    }
}

/**
 * Inicia uma nova rodada do jogo.
 */
void start_game() {
    game_running = true;
    score = 0;
}

/**
 * Verifica a resposta do jogador.
 *
 * @param is_correct Indica se a resposta correta é "sim"
 * @param button_pressed Botão pressionado pelo jogador
 */
void check_answer(bool is_correct, int button_pressed) {
    if ((is_correct && button_pressed == BUTTON_A) || (!is_correct && button_pressed == BUTTON_B)) {
        score++;
        display_text("Correto!");
        sleep_ms(1000);
    } else {
        char buffer[20];
        sprintf(buffer, "Errou!         Acertos: %d", score);
        display_text(buffer);
        buzzer_beep(4000, 500);
        sleep_ms(2000);
        game_running = false;
        display_text("Pressione   A  para reiniciar!");
    }
}

/**
 * Callback de interrupção para os botões.
 *
 * @param gpio Pino do botão pressionado
 * @param events Evento ocorrido
 */
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_A || gpio == BUTTON_B) {
        // Verifica se ambos os botões estão pressionados
        if (gpio_get(BUTTON_A) == 0 && gpio_get(BUTTON_B) == 0) {
            both_buttons_pressed = true;
        }
    }
}

/**
 * Função principal do jogo.
 */
int main() {
    stdio_init_all();

    // Inicializa a comunicação I2C para o display OLED
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();

    // Exibe mensagem inicial no display
    display_text("JOGO DA SOMA");
    sleep_ms(2000);
    display_text("A PARA CERTO   B PARA ERRADO  Pressione   A!");

    // Configuração dos botões
    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B);

    // Inicializa o buzzer
    pwm_init_buzzer(BUZZER);

    // Configura interrupções para os botões A e B
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    while (true) {
        if (!game_running && gpio_get(BUTTON_A) == 0) {
            start_game();
            sleep_ms(300);
        }

        if (game_running) {
            int num1, num2, result;
            bool is_correct;
            generate_question(&num1, &num2, &result, &is_correct);

            char buffer[20];
            sprintf(buffer, "%d + %d = %d", num1, num2, result);
            display_text(buffer);

            int button_pressed = 0;
            while (button_pressed == 0) {
                if (both_buttons_pressed) {
                    display_text("Nao pressione   A e B juntos!");
                    sleep_ms(2000);
                    game_running = false;
                    display_text("Pressione   A  para reiniciar!");
                    both_buttons_pressed = false;
                    break;
                }

                if (gpio_get(BUTTON_A) == 0) button_pressed = BUTTON_A;
                else if (gpio_get(BUTTON_B) == 0) button_pressed = BUTTON_B;
            }

            check_answer(is_correct, button_pressed);
        }
    }
}

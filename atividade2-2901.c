#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Definição dos pinos dos LEDs e do botão
#define vermelho 11
#define azul 12
#define verde 13
#define push_button 5

// Variáveis globais para controle do estado
volatile bool botao_pressionado = false;
volatile int estado_leds = 0; // 0: todos desligados, 1: todos ligados, 2: dois ligados, 3: um ligado

// Função de callback para mudar o estado dos LEDs
int64_t mudar_estado_leds(alarm_id_t id, void *user_data) {
    switch (estado_leds) {
        case 1:
            // Primeira mudança: dois LEDs ligados
            gpio_put(vermelho, 1);
            gpio_put(azul, 1);
            gpio_put(verde, 0);
            estado_leds = 2;
            break;
        case 2:
            // Segunda mudança: um LED ligado
            gpio_put(vermelho, 1);
            gpio_put(azul, 0);
            gpio_put(verde, 0);
            estado_leds = 3;
            break;
        case 3:
            // Terceira mudança: todos os LEDs desligados
            gpio_put(vermelho, 0);
            gpio_put(azul, 0);
            gpio_put(verde, 0);
            estado_leds = 0;
            break;
    }

    // Se o estado não for 0, agendar a próxima mudança
    if (estado_leds != 0) {
        add_alarm_in_ms(3000, mudar_estado_leds, NULL, false);
    }

    return 0; // Retorno necessário para a função de callback
}

// Função de interrupção do botão
void callback_botao(uint gpio, uint32_t events) {
    if (gpio == push_button && estado_leds == 0) {
        botao_pressionado = true;
    }
}

int main() {
    // Inicialização dos pinos
    gpio_init(vermelho);
    gpio_init(azul);
    gpio_init(verde);
    gpio_init(push_button);

    gpio_set_dir(vermelho, GPIO_OUT);
    gpio_set_dir(azul, GPIO_OUT);
    gpio_set_dir(verde, GPIO_OUT);
    gpio_set_dir(push_button, GPIO_IN);

    gpio_pull_up(push_button); // Habilita o pull-up no botão

    // Configura a interrupção do botão
    gpio_set_irq_enabled_with_callback(push_button, GPIO_IRQ_EDGE_FALL, true, &callback_botao);

    while (1) {
        if (botao_pressionado && estado_leds == 0) {
            // Liga todos os LEDs
            gpio_put(vermelho, 1);
            gpio_put(azul, 1);
            gpio_put(verde, 1);
            estado_leds = 1;

            // Agenda a primeira mudança de estado após 3 segundos
            add_alarm_in_ms(3000, mudar_estado_leds, NULL, false);

            botao_pressionado = false; // Reseta o estado do botão
        }

        tight_loop_contents(); // Mantém o loop principal ocupado
    }

    return 0;
}

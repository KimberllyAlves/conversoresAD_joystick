#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"     
#include "hardware/pwm.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

// Definições para a comunicação I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C // Endereço do display OLED

// Definições dos pinos do joystick
#define VRX_PIN 27  // Eixo X do joystick
#define VRY_PIN 26  // Eixo Y do joystick
#define SW_PIN 22   // Botão do joystick

// Definições dos pinos dos LEDs
#define LED_PIN_RED 13  
#define LED_PIN_BLUE 12 
#define LED_PIN_GREEN 11 

#define BUTTON_A 5 // Botão de controle

#define pos_central 2250 // Valor central do joystick

// Variáveis globais
int estado_led_verde = false; // Estado do LED verde
int tipo_borda = 0; // Tipo de borda a ser desenhada no display
uint32_t last_time = 0; // Variável para controle de tempo do printf
bool estado_leds_pwm = true; // Controle dos LEDs em PWM

// Função para inicializar PWM em um pino específico
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);  
    return slice_num;  
}

// Função de interrupção para o botão com debouncing
void gpio_irq_handler(uint gpio, uint32_t events){
  uint32_t current_time = to_us_since_boot(get_absolute_time());
  if (current_time - last_time > 200000) { // 200 ms de debouncing
    last_time = current_time;
    estado_leds_pwm = !estado_leds_pwm;
  }
}

int main()
{
  stdio_init_all(); // Inicializa a comunicação serial

  // Configuração do ADC (Conversor Analógico-Digital)
  adc_init(); 
  adc_gpio_init(VRX_PIN); 
  adc_gpio_init(VRY_PIN); 

  // Configuração do botão do joystick
  gpio_init(SW_PIN);
  gpio_set_dir(SW_PIN, GPIO_IN);
  gpio_pull_up(SW_PIN); 

  // Configuração dos LEDs
  gpio_init(LED_PIN_RED);
  gpio_set_dir(LED_PIN_RED, GPIO_OUT);
  gpio_init(LED_PIN_BLUE);
  gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
  gpio_init(LED_PIN_GREEN);
  gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);

  // Configuração do botão A
  gpio_init(BUTTON_A);
  gpio_set_dir(BUTTON_A, GPIO_IN);
  gpio_pull_up(BUTTON_A);

  // Inicializa a comunicação I2C a 400 kHz
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  // Inicialização do display OLED
  ssd1306_t ssd;
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
  ssd1306_config(&ssd);
  ssd1306_send_data(&ssd);
  ssd1306_fill(&ssd, false); // Limpa o display
  ssd1306_send_data(&ssd);

  bool cor = true;

  // Inicializa os LEDs com PWM
  uint pwm_wrap = 4096;  
  uint pwm_slice_red = pwm_init_gpio(LED_PIN_RED, pwm_wrap);   
  uint pwm_slice_blue = pwm_init_gpio(LED_PIN_BLUE, pwm_wrap);
  uint32_t last_print_time = 0;

  // Configuração da interrupção do botão A
  gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

  while (true)
  {
    // Leitura do joystick
    adc_select_input(1);  
    uint16_t vrx_value = adc_read(); 
    adc_select_input(0); 
    uint16_t vry_value = adc_read(); 
    bool sw_value = gpio_get(SW_PIN) == 0;

    // Controle dos LEDs com base no joystick
    if (estado_leds_pwm) {
      if (vrx_value > pos_central) {
        pwm_set_gpio_level(LED_PIN_RED, vrx_value);
      } else pwm_set_gpio_level(LED_PIN_RED, 0);

      if (vry_value > pos_central) {
        pwm_set_gpio_level(LED_PIN_BLUE, vry_value); 
      } else pwm_set_gpio_level(LED_PIN_BLUE, 0);
    }
    else {
      pwm_set_gpio_level(LED_PIN_RED, 0);
      pwm_set_gpio_level(LED_PIN_BLUE, 0);
    }
    
    // Controle do LED verde e das bordas do display
    if (sw_value) { 
      estado_led_verde = !estado_led_verde;
      gpio_put(LED_PIN_GREEN, estado_led_verde);
      tipo_borda++; 
      if (tipo_borda >= 3) tipo_borda = 0;
      sleep_ms(200);
    } 

    // Exibição dos valores do joystick a cada segundo
    uint32_t current_time = to_ms_since_boot(get_absolute_time());  
    if (current_time - last_print_time >= 1000) {  
        printf("VRX: %u\n", vrx_value);  
        printf("VRY: %u\n", vry_value); 
        last_print_time = current_time;  
    }

    // Atualização do display OLED
    int x_pos = (vrx_value * (128 - 8)) / 4095;
    int y_pos = (64 - 8) - (vry_value * (64 - 8)) / 4095;
    ssd1306_fill(&ssd, !cor);
    if (tipo_borda == 1) ssd1306_rect(&ssd, 0, 0, 128, 64, cor, !cor);
    if (tipo_borda == 2) {
      ssd1306_rect(&ssd, 0, 0, 128, 64, cor, !cor);
      ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);
    }
    ssd1306_rect(&ssd, y_pos, x_pos, 8, 8, cor, cor);
    ssd1306_send_data(&ssd);

    sleep_ms(100);
  }

  return 0;
}

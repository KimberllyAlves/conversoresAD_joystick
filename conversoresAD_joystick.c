#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"     
#include "hardware/pwm.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

//PINOS JOYSTICK
#define VRX_PIN 27  
#define VRY_PIN 26
#define SW_PIN 22

//PINOS LEDS
#define LED_PIN_RED 13  
#define LED_PIN_BLUE 12 
#define LED_PIN_GREEN 11 

#define pos_central 2250

int estado_led_verde = false;
int tipo_borda = 0;

uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    
    pwm_set_enabled(slice_num, true);  
    return slice_num;  
}

int main()
{
  stdio_init_all();
  
  adc_init(); 
  adc_gpio_init(VRX_PIN); 
  adc_gpio_init(VRY_PIN); 
  gpio_init(SW_PIN);
  gpio_set_dir(SW_PIN, GPIO_IN);
  gpio_pull_up(SW_PIN); 
  gpio_init(LED_PIN_RED);
  gpio_set_dir(LED_PIN_RED, GPIO_OUT);
  gpio_init(LED_PIN_BLUE);
  gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
  gpio_init(LED_PIN_GREEN);
  gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA); // Pull up the data line
  gpio_pull_up(I2C_SCL); // Pull up the clock line
  ssd1306_t ssd; // Inicializa a estrutura do display
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd); // Configura o display
  ssd1306_send_data(&ssd); // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  bool cor = true;

  uint pwm_wrap = 4096;  
  uint pwm_slice_red = pwm_init_gpio(LED_PIN_RED, pwm_wrap);   
  uint pwm_slice_blue = pwm_init_gpio(LED_PIN_BLUE, pwm_wrap);   
  uint32_t last_print_time = 0;

  while (true)
  {
    adc_select_input(1);  
    uint16_t vrx_value = adc_read(); 
    adc_select_input(0); 
    uint16_t vry_value = adc_read(); 
    bool sw_value = gpio_get(SW_PIN) == 0;

    if (vrx_value > pos_central) {
      pwm_set_gpio_level(LED_PIN_RED, vrx_value);
    }
    else  pwm_set_gpio_level(LED_PIN_RED, 0);

    if (vry_value > pos_central) {
      pwm_set_gpio_level(LED_PIN_BLUE, vry_value); 
    }
    else  pwm_set_gpio_level(LED_PIN_BLUE, 0);

    if (sw_value) { 
      estado_led_verde = !estado_led_verde;
      gpio_put(LED_PIN_GREEN, estado_led_verde);
      tipo_borda++; 
      if (tipo_borda >= 3) tipo_borda = 0;
      sleep_ms(200);
    } 

    uint32_t current_time = to_ms_since_boot(get_absolute_time());  
    if (current_time - last_print_time >= 1000) {  
        printf("VRX: %u\n", vrx_value);  
        printf("VRY: %u\n", vry_value); 
        last_print_time = current_time;  
    }
    // Mapeia os valores do joystick para as coordenadas do display (128x64 pixels)
    int x_pos = (vrx_value * (128 - 8)) / 4095;              // Ajusta X para a largura do display (0 a 120)
    int y_pos = (64 - 8) - (vry_value * (64 - 8)) / 4095;   // Ajusta Y para a altura do display (0 a 56), invertendo para corresponder ao display

    // Atualiza o conteúdo do display com animações
    ssd1306_fill(&ssd, !cor); // Limpa o display
    if (tipo_borda == 1) ssd1306_rect(&ssd, 0, 0, 128, 64, cor, !cor); // Desenha um retângulo
    if (tipo_borda == 2) {
      ssd1306_rect(&ssd, 0, 0, 128, 64, cor, !cor); // Desenha um retângulo
      //ssd1306_rect(&ssd, 2, 2, 127, 63, cor, !cor); // Desenha um retângulo
      ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);  // Retângulo preenchido
    }
    ssd1306_rect(&ssd, y_pos, x_pos, 8, 8, cor, cor);      
    ssd1306_send_data(&ssd); // Atualiza o display

    sleep_ms(100);
  }

  return 0;
}
/*---------------------------------------------------------------------
                            INCLUDE FILES
---------------------------------------------------------------------*/
#include "stm32f10x.h"
/*---------------------------------------------------------------------
                        GLOBAL DEFINITIONS
---------------------------------------------------------------------*/
char received_cmd[20] = {0};
char received_cmd_handle = 0;
/*---------------------------------------------------------------------
                       FUNCTION DEFINITIONS
---------------------------------------------------------------------*/
int string_cmp(const char* str1, const char* str2)
{
    int i = 0;

    // Compare each character of both strings
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return 0;  // Strings are different
        }
        i++;
    }

    // If both strings end at the same time, they are equal
    if (str1[i] == '\0' && str2[i] == '\0') {
        return 1;  // Strings are the same
    } else {
        return 0;  // Strings are different in length
    }
}

void uart_putc(USART_TypeDef *uart, uint8_t byte)
{
  while((uart->SR & (1 << 7)) == 0); // Wait until uart transmit buffer is free
  uart->DR = byte; // Write byte
}

void uart_puts(USART_TypeDef *uart, char *str)
{
    while(*str) {
       uart_putc(uart, *str);
       str++; 
    }
}

void USART1_IRQHandler(void)
{
    char ch = '0';
    if (USART1->SR & (1 << 5)) {
        ch = USART1->DR; // Read to clear RXNE
        if (ch != '#') {
            received_cmd[received_cmd_handle] = ch;
            received_cmd_handle++;
        } else {
            received_cmd[received_cmd_handle] = '\0';
            if (string_cmp(received_cmd, "light on")) {
                GPIOA->BSRR = (1 << 0);
            } else if (string_cmp(received_cmd, "off")) {
                GPIOA->BSRR = (1 << 16);
            }
            received_cmd_handle = 0;
        }
        
    }
}

void uart_init(void)
{
    USART1->BRR = 0x341;
    //USART1->BRR = 0x45; // Set baud rate, TRM 27.3.4
    USART1->CR1 |= (1 << 13);
    USART1->CR1 |= (1 << 2);
    USART1->CR1 |= (1 << 3);
    USART1->CR1 |= (1 << 5);
}

void nvic_init(void)
{
    // For USART1 IRQ number 37, we need to set bit 5 of NVIC ISER1
    NVIC->ISER[1] = (1 << (37 - 32));  // Enable interrupt 37 in NVIC (ISER1 for IRQ numbers >= 32)
}

void clock_init(void)
{
    // Enabling Clocks which are hanging at AHB Bus
    RCC->AHBENR |= (1 << 2); //Enable SRAM interface clock
    RCC->AHBENR |= (1 << 4); //Enable Flash memory interface (FLITF) clock

    // Enabling Clocks which are hanging at APB1 Bus
    //RCC->APB1ENR

    // Enabling Clocks which are hanging at APB2 Bus
    RCC->APB2ENR |= (1 << 0);  //Enable Alternate function IO clock
    RCC->APB2ENR |= (1 << 2);  //Enable GPIOA clock
    RCC->APB2ENR |= (1 << 3);  //Enable GPIOB clock
    RCC->APB2ENR |= (1 << 14); //Enable USART1 clock
}

void gpio_init(void)
{
    /*  - GPIOA pin 9  <--> UART TX     <--> o/p max speed 2Mhz, AF o/p push pull   <--> 1010 (0xA)
        - GPIOA pin 10 <--> UART RX     <--> i/p mode, Floating input               <--> 0100 (0x4)
        - GPIOB pin 6  <--> I2C SCL
        - GPIOB pin 7  <--> I2C SDA 
    */
    
    // GPIO Port A
    GPIOA->CRL = 0x2222;
    GPIOA->CRH = 0x4A0;
    GPIOA->BSRR |= (1 << 16); // Turn off the LED connected to the PA0

    // GPIO Port B
    GPIOB->CRL = 0x22;
    GPIOB->ODR = 0x00;
}

void device_init(void)
{
    clock_init();
    gpio_init();
    uart_init();
    nvic_init();
}

int main(void)
{
    device_init();
    uart_puts(USART1, "PARANDHAMAN\r\n");
    while(1) {
        if (GPIOB->IDR & (1 << 0)) {
            //GPIOA->ODR = GPIOA->ODR ^ 0xFF;
            //soft_delay();
            //GPIOA->ODR = GPIOA->ODR ^ 0xFF;
        }
    }
}

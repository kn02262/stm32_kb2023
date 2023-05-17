#include <stdint.h>
#include <stm32f10x.h>

void delay(uint32_t ticks) {
	for (int i=0; i<ticks; i++) {
		__NOP();
	}
}

int p;
void EXTI15_10_IRQHandler(void){
	p++;
	NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
	EXTI->PR |= (1<<11);
}

int __attribute((noreturn)) main(void) {
	// Enable clock for AFIO
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	// Enable clock for GPIOC
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	// Enable clock for GPIOA
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	// Enable PC13 push-pull mode
	GPIOC->CRH &= ~GPIO_CRH_CNF13; //clear cnf bits
	GPIOC->CRH |= GPIO_CRH_MODE13_0; //Max speed = 10Mhz
	// Enable PA11 input with pull-up (Button on interrupt)
	GPIOA->CRH &= ~GPIO_CRH_MODE11;
	GPIOA->CRH &= ~GPIO_CRH_CNF11;
	GPIOA->CRH |= GPIO_CRH_CNF11_1;
	GPIOA->ODR |= GPIO_ODR_ODR11;
	// Configure A0 (WKUP) as input with internal pull-down
	GPIOA->CRL &= ~GPIO_CRL_CNF0;
	GPIOA->CRL &= ~GPIO_CRL_MODE0;
	GPIOA->CRL |= GPIO_CRL_CNF0_1;
	GPIOA->ODR &= ~GPIO_ODR_ODR0;

	// Enable interrupt on PA11 falling edge
	EXTI->FTSR |= EXTI_FTSR_TR11;
	EXTI->IMR |= EXTI_IMR_MR11;
	EXTI->PR |= (1<<11);
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	int i=0;
	p = 2;
    while (1) {
	    GPIOC->ODR |= (1U<<13U); // LED is OFF (PC13=3V3)
		delay(10000000>>p);
	    GPIOC->ODR &= ~(1U<<13U); // LED is ON (PC13=GND)
	    delay(10000000>>p);
		if(i==6){ // Sleep
			RCC->APB1ENR |= RCC_APB1ENR_PWREN;
			PWR->CR |= PWR_CR_LPDS;
			PWR->CR |= PWR_CR_PDDS;
			uint32_t *scr = 0xE000ED10; // Cortex-M3 SCR
			*scr |= (1U << 2U); // Set SLEEPDEEP bit
			PWR->CSR |= PWR_CSR_EWUP;
			PWR->CSR &= ~PWR_CSR_WUF;
			__asm("WFI");
		}
	i++;
    }
}

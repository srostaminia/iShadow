#define GPIOC_PDOR_0 *(volatile uint8_t *)0x400FF080
//#define GPIOC_PDOR_0 *(volatile uint8_t *)(*(volatile uint32_t *)0x400FF080)
//#define GPIOC_PDOR_0 *(volatile uint8_t *)(GPIOC_PDOR)
//#define GPIOC_PDOR_1 *(volatile uint8_t *)(GPIOC_PDOR+2)
//#define GPIOC_PDOR_2 *(volatile uint8_t *)(GPIOC_PDOR+2)
//#define GPIOC_PDOR_3 *(volatile uint8_t *)(GPIOC_PDOR+3)

void setup()
{
//  DDRC |= 0x00000000ul;
//  GPIOC_PDDR |= 0x00000020ul;
  pinMode(13, OUTPUT);
}

void loop()
{
/*  digitalWrite(13,0);
  delay(450);
  digitalWrite(13,1);*/
//  GPIOC_PDOR ^= 0x00000020ul;
  GPIOC_PDOR_0 &= (uint8_t)(~0x20);
  delay(25);
  GPIOC_PDOR_0 |= 0x20;
  delay(775);
}


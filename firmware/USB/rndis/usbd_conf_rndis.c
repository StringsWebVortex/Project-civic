/* usbd_conf_rndis.c - HAL PCD <-> ST USB Device core link for OTG_FS
 * RNDIS network adapter (MB997 CN5: PA11 DM, PA12 DP). */
#include "stm32f4xx_hal.h"
#include "usbd_core.h"

#include <stdbool.h>
#include <string.h>

PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern volatile uint32_t system_millis;

/* Pool: single static allocation for the class handle (no heap). */
static union { uint64_t alignment; uint8_t bytes[512]; } usbd_pool;
static bool usbd_pool_allocated;

/* HAL shims */
uint32_t HAL_GetTick(void)         { return system_millis; }
void     HAL_Delay(uint32_t ms)    { const uint32_t t0 = system_millis; while ((uint32_t)(system_millis - t0) < ms) { __asm volatile("nop"); } }
uint32_t HAL_RCC_GetHCLKFreq(void) { return SystemCoreClock; }
void     assert_failed(uint8_t *f, uint32_t l) { (void)f; (void)l; }

static void otg_fs_pin(uint8_t pin)
{
    GPIOA->MODER   = (GPIOA->MODER & ~(3u << (2u * pin))) | (2u << (2u * pin));
    GPIOA->OTYPER &= ~(1u << pin);
    GPIOA->OSPEEDR |= (3u << (2u * pin));
    GPIOA->PUPDR   &= ~(3u << (2u * pin));
    const uint8_t sh = (uint8_t)((pin - 8u) * 4u);
    GPIOA->AFR[1] = (GPIOA->AFR[1] & ~(15u << sh)) | (10u << sh);  /* AF10 */
}

void HAL_PCD_MspInit(PCD_HandleTypeDef *h)
{
    if (h->Instance != USB_OTG_FS) return;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; (void)RCC->AHB1ENR;
    otg_fs_pin(11u); otg_fs_pin(12u);
    RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN; (void)RCC->AHB2ENR;
    NVIC_SetPriority(OTG_FS_IRQn, 6u);
    NVIC_EnableIRQ(OTG_FS_IRQn);
}
void HAL_PCD_MspDeInit(PCD_HandleTypeDef *h)
{
    if (h->Instance != USB_OTG_FS) return;
    NVIC_DisableIRQ(OTG_FS_IRQn);
    RCC->AHB2ENR &= ~RCC_AHB2ENR_OTGFSEN;
}

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *h)      { USBD_LL_SetupStage((USBD_HandleTypeDef *)h->pData, (uint8_t *)h->Setup); }
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *h, uint8_t e) { USBD_LL_DataOutStage((USBD_HandleTypeDef *)h->pData, e, h->OUT_ep[e].xfer_buff); }
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *h, uint8_t e)  { USBD_LL_DataInStage((USBD_HandleTypeDef *)h->pData, e, h->IN_ep[e].xfer_buff); }
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *h)              { USBD_LL_SOF((USBD_HandleTypeDef *)h->pData); }
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *h)            { USBD_LL_SetSpeed((USBD_HandleTypeDef *)h->pData, USBD_SPEED_FULL); USBD_LL_Reset((USBD_HandleTypeDef *)h->pData); }
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *h)          { USBD_LL_Suspend((USBD_HandleTypeDef *)h->pData); }
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *h)           { USBD_LL_Resume((USBD_HandleTypeDef *)h->pData); }
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *h)          { USBD_LL_DevConnected((USBD_HandleTypeDef *)h->pData); }
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *h)       { USBD_LL_DevDisconnected((USBD_HandleTypeDef *)h->pData); }

void OTG_FS_IRQHandler(void) { HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS); }

USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
    hpcd_USB_OTG_FS.pData = pdev;
    pdev->pData = &hpcd_USB_OTG_FS;
    hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
    hpcd_USB_OTG_FS.Init.dev_endpoints = 4;
    hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
    hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
    hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
    if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK) return USBD_FAIL;
    /* FIFO allocation (320 words total): RX 128w, TX0 (ctrl) 64w, TX1 (notif) 16w, TX2 (bulk) 112w */
    if (HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x80) != HAL_OK ||
        HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 0x40) != HAL_OK ||
        HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 0x10) != HAL_OK ||
        HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 2, 0x70) != HAL_OK)
        return USBD_FAIL;
    return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev) { (void)pdev; return (HAL_PCD_DeInit(&hpcd_USB_OTG_FS) == HAL_OK) ? USBD_OK : USBD_FAIL; }
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)  { (void)pdev; return (HAL_PCD_Start(&hpcd_USB_OTG_FS) == HAL_OK) ? USBD_OK : USBD_FAIL; }
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)   { (void)pdev; return (HAL_PCD_Stop(&hpcd_USB_OTG_FS) == HAL_OK) ? USBD_OK : USBD_FAIL; }

USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep, uint8_t type, uint16_t sz)
{ (void)pdev; return (HAL_PCD_EP_Open(&hpcd_USB_OTG_FS, ep, sz, type) == HAL_OK) ? USBD_OK : USBD_FAIL; }
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep)
{ (void)pdev; return (HAL_PCD_EP_Close(&hpcd_USB_OTG_FS, ep) == HAL_OK) ? USBD_OK : USBD_FAIL; }
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep)
{ (void)pdev; return (HAL_PCD_EP_Flush(&hpcd_USB_OTG_FS, ep) == HAL_OK) ? USBD_OK : USBD_FAIL; }
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep)
{ (void)pdev; return (HAL_PCD_EP_SetStall(&hpcd_USB_OTG_FS, ep) == HAL_OK) ? USBD_OK : USBD_FAIL; }
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep)
{ (void)pdev; return (HAL_PCD_EP_ClrStall(&hpcd_USB_OTG_FS, ep) == HAL_OK) ? USBD_OK : USBD_FAIL; }
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep)
{ (void)pdev; PCD_HandleTypeDef *h = &hpcd_USB_OTG_FS; return (ep & 0x80) ? h->IN_ep[ep & 0x7F].is_stall : h->OUT_ep[ep].is_stall; }
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t addr)
{ (void)pdev; return (HAL_PCD_SetAddress(&hpcd_USB_OTG_FS, addr) == HAL_OK) ? USBD_OK : USBD_FAIL; }
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep, uint8_t *buf, uint32_t sz)
{ (void)pdev; return (HAL_PCD_EP_Transmit(&hpcd_USB_OTG_FS, ep, buf, sz) == HAL_OK) ? USBD_OK : USBD_FAIL; }
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep, uint8_t *buf, uint32_t sz)
{ (void)pdev; return (HAL_PCD_EP_Receive(&hpcd_USB_OTG_FS, ep, buf, sz) == HAL_OK) ? USBD_OK : USBD_FAIL; }
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep)
{ (void)pdev; return HAL_PCD_EP_GetRxCount(&hpcd_USB_OTG_FS, ep); }

void *USBD_static_malloc(uint32_t size)
{
    (void)size;
    if (usbd_pool_allocated) return NULL;
    usbd_pool_allocated = true;
    return usbd_pool.bytes;
}
void USBD_static_free(void *p) { if (p == usbd_pool.bytes) usbd_pool_allocated = false; }

//--------------------------------------------------------------
// File     : stm32_ub_usb_cdc.c
// Datum    : 23.06.2013
// Version  : 1.4
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO, MISC
// Funktion : USB-COM-Verbindung per USB-OTG-Port am Discovery
//
// ����������:  �� ��������� ���������� ���������� 
// 		"VirtualComportDriver" ������ (V 1.3.1) ������ � 
// 		��� ������ ����������� COM-���� ����� 
// 		������������ � ���������� ���������
//
//		������ Discovery �������� ������ USB Full ������ 
//		(USB-High-Speed �� ��������)
//
//--------------------------------------------------------------
//              PA8   -> USB_OTG_SOF (�� ������������)
//              PA9   -> USB_OTG_VBUS
//              PA10  -> USB_OTG_ID
//              PA11  -> USB_OTG_DM
//              PA12  -> USB_OTG_DP
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_usb_cdc.h"

//--------------------------------------------------------------
// ���������� ����������
//--------------------------------------------------------------
USB_OTG_CORE_HANDLE  USB_OTG_dev;





//--------------------------------------------------------------
// ������������� USB-OTG-����� ��� CDC-����������
// (����������� COM ����)
//--------------------------------------------------------------
void UB_USB_CDC_Init(void)
{
  USB_CDC_STATUS=USB_CDC_DETACHED;
  USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);
}

//--------------------------------------------------------------
// ������ ��������� ���������� USB
// ������������ �������� :
// ->  USB_CDC_NO_INIT =0, // ��������� USB �� ��������������� 
// ->  USB_CDC_DETACHED,   // USB-���������� ����������������
// ->  USB_CDC_CONNECTED   // USB-���������� �����������
//--------------------------------------------------------------
USB_CDC_STATUS_t UB_USB_CDC_GetStatus(void)
{
  return(USB_CDC_STATUS);
}


//--------------------------------------------------------------
// ������� ������ ����� ��������� OTG USB
// ������� � ����� : [NONE, LFCR, CRLF, LF, CR]
// ������������ �������� :
//  -> ERROR   , ������ �� ����������
//  -> SUCCESS , ������ ����������
//--------------------------------------------------------------
ErrorStatus UB_USB_CDC_SendString(char *ptr, USB_CDC_LASTBYTE_t end_cmd)
{

  if(USB_CDC_STATUS!=USB_CDC_CONNECTED) {
  // ���������� ������, ����� ���������� �����������
    return(ERROR);
  }

  // ��������� ������ �������
  while (*ptr != 0) {
    UB_VCP_DataTx(*ptr);
    ptr++;
  }
  // �������� ��������� ��������������
  if(end_cmd==LFCR) {
    UB_VCP_DataTx(0x0A); // ������� ������
    UB_VCP_DataTx(0x0D); // ������� �������
  }
  else if(end_cmd==CRLF) {
    UB_VCP_DataTx(0x0D); // ������� �������
    UB_VCP_DataTx(0x0A); // ������� ������
  }
  else if(end_cmd==LF) {
    UB_VCP_DataTx(0x0A); // ������� ������
  }
  else if(end_cmd==CR) {
    UB_VCP_DataTx(0x0D); // ������� �������
  }

  return(SUCCESS);
}



//--------------------------------------------------------------
// ��������� ������ ����� ��������� OTG USB
// (����� ����������� � ������� ����������)
// ��� ������� ������ ������������ ����������
// ������������ �������� :
//  -> ���� USB �� ����� = RX_USB_ERR
//  -> ���� ������ �� �������� = RX_EMPTY
//  -> ���� ������ �������� = RX_READY -> ������ � *ptr
//--------------------------------------------------------------
USB_CDC_RXSTATUS_t UB_USB_CDC_ReceiveString(char *ptr)
{
  uint16_t check;

  if(USB_CDC_STATUS!=USB_CDC_CONNECTED) {
    // ����� ������ �����, ����� ���������� �����������
    return(RX_USB_ERR);
  }

  check=UB_VCP_StringRx(ptr);
  if(check==0) {
    ptr[0]=0x00;
    return(RX_EMPTY);
  }

  return(RX_READY);
}

//------------------------------------���������� ������� ��� ������ ���� �������� ASCII------------------------------------
//--------------------------------------------------------------
// ��������� ������ ����� ��������� OTG USB
// (����� ����������� � ������� ����������)
// ��� ������� ������ ������������ ����������
// ������������ �������� :
//  -> ���� USB �� ����� = RX_USB_ERR
//  -> ���� ������ �� �������� = RX_EMPTY
//  -> ���� ������ �������� = RX_READY -> ������ � *ptr
//--------------------------------------------------------------
USB_CDC_RXSTATUS_t UB_USB_CDC_ReceiveString_NEW(char *ptr)
{
  uint16_t check;

  if(USB_CDC_STATUS!=USB_CDC_CONNECTED) {
    // ����� ������ �����, ����� ���������� �����������
    return(RX_USB_ERR);
  }

  check=UB_VCP_StringRx_NEW(ptr);
  if(check==0) {
    ptr[0]=0x00;
    return(RX_EMPTY);
  }

  return(RX_READY);
}
//-------------------------------------------------------------------------------------------------------------------------

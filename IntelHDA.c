/** @file
  This sample application bases on HelloWorld PCD setting
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/PciIo.h>
#include <IndustryStandard/Pci.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciIo.h>
#include <limits.h>
#include "./jedi.h"
#include <Library/MemoryAllocationLib.h>
//Extract the OSS count from the controller's Global
//capabilities register
#define HDA_OSS_COUNT(GCAP) \
	((GCAP >> 12) & 0xF)

//Extract the ISS count from the controller's Global
//capabilities register
#define HDA_ISS_COUNT(GCAP) \
	((GCAP >> 8) & 0xF)

//Extract the BSS count from the controller's Global
//capabilities register
#define HDA_BSS_COUNT(GCAP) \
	((GCAP >> 3) & 0x1F)

//
// String token ID of help message text.
// Shell supports to find help message in the resource section of an application image if
// .MAN file is not found. This global variable is added to make build tool recognizes
// that the help string is consumed by user and then build tool will add the string into
// the resource section. Thus the application can use '-?' option to show help message in
// Shell.
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID mStringHelpTokenId = STRING_TOKEN (STR_HELLO_WORLD_HELP_INFORMATION);

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_GUID gPciIo = EFI_PCI_IO_PROTOCOL_GUID;
EFI_PCI_IO_PROTOCOL* PciIo;


Framebuffer* fb;
EFI_STATUS
EFIAPI
UefiMain(
	IN EFI_HANDLE        ImageHandle,
	IN EFI_SYSTEM_TABLE* SystemTable
)
{


	SystemTable->ConOut->ClearScreen(SystemTable->ConOut);



		EFI_HANDLE *Buffer;
		UINTN NoHandles;

		gBS->LocateHandleBuffer(ByProtocol, &gPciIo, NULL, &NoHandles, &Buffer);

		UINTN Segment;
		UINTN Bus;
		UINTN Device;
		UINTN Function;
		VOID *VendorId;
		VOID* DeviceId;
		VOID *Class;
		VOID *SubClass;

		// Scan all found handles

		unsigned int Index;
		unsigned int Count;
#pragma warning(disable: 4305)
		for (Index = 0, Count = 0; Index < NoHandles; Index++) {
			// Get PciIo protocol for current handle
			gBS->HandleProtocol(Buffer[Index], &gPciIo, (VOID**)&PciIo);
			PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, 0x00, 1, &VendorId);
			PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, 0x02, 1, &DeviceId);

			PciIo->Pci.Read(PciIo, EfiPciIoWidthUint8, 0x0a, 1, &SubClass);
			PciIo->Pci.Read(PciIo, EfiPciIoWidthUint8, 0x0b, 1, &Class);

			PciIo->GetLocation(PciIo, &Segment, &Bus, &Device, &Function);

			Print(L"Class : 0x%x , SubClass : 0x%x , Vendor : 0x%x Device : 0x%x,Segment : 0x%x, Bus : 0x%x, Device : 0x%x, Function 0x%x\n", (unsigned char)Class,(unsigned char )SubClass,(unsigned short) VendorId,(unsigned short) DeviceId, Segment, Bus, Device, Function);
#pragma warning(disable:4312)

			if ((unsigned short)DeviceId == 0x43C8||(unsigned short)DeviceId == 0x2668) {
				Print(L"Found Audio, Dumping Config Space\n");
				#pragma warning(disable:4311)
		
				UINT16 Command = 0x0006;
				VOID* COMMAND_BUFFER = (VOID*)&Command;
				PciIo->Pci.Write(PciIo, EfiPciWidthUint16, 0x04, 1, COMMAND_BUFFER);
				VOID* COMMAND_CHECK = (VOID*)AllocateZeroPool(sizeof(UINT16));
				PciIo->Pci.Read(PciIo, EfiPciWidthUint16, 0x04, 1, COMMAND_CHECK);
				Print(L"Checking Command : 0x%x", *(UINT16*)COMMAND_CHECK);

				VOID* GCAP_MEM = (VOID*)AllocateZeroPool(sizeof(UINT16));
				status = PciIo->Mem.Read(PciIo, EfiPciWidthUint16, 0, 0x0, 1, GCAP_MEM);

				if (EFI_ERROR(status)) {
					Print(L" GCAP  Error : %r\n", status);
				}
				UINT16 GCAP = *(UINT16 *)GCAP_MEM;
				
				Print(L"GCAP : 0x%x\n", GCAP);

				VOID* GTCL_MEM = (VOID*)AllocateZeroPool(sizeof(UINT32));
				status = PciIo->Mem.Read(PciIo, EfiPciWidthUint32, 0, 0x08, 1, GTCL_MEM);
				if (EFI_ERROR(status)) {
					Print(L" GTCL  Error : %r\n", status);
				}
				UINT32 GTCL = *(UINT32*)GTCL_MEM;
				Print(L"GTCL : 0x%x\n", GTCL);

				gBS->Stall(200000);

				UINT16 STS_CLRBGR = 0xffff;
				VOID* STS_CLR_MEMBGR = (VOID*)&STS_CLRBGR;
				status = PciIo->Mem.Write(PciIo, EfiPciWidthUint16, 0, 0x0e, 1, STS_CLR_MEMBGR);
				Print(L"Claering STS REG before GCTL enable : %r\n", status);
				VOID* STS_RESETBGR = (VOID*)AllocateZeroPool(sizeof(UINT16));
        
				status = PciIo->Mem.Read(PciIo, EfiPciWidthUint16, 0, 0x0e, 1, STS_RESETBGR);
				UINT16 STS_RBGR = *(UINT16*)STS_RESETBGR;
				Print(L"Reading STS after sending 0xffff  before GCTL enable : 0x%x\n", STS_RBGR);


				VOID* VMIN = (VOID*)AllocateZeroPool(sizeof(UINT8));
				status = PciIo->Mem.Read(PciIo, EfiPciWidthUint8, 0, 0x02, 1, VMIN);

				VOID* VMAX = (VOID*)AllocateZeroPool(sizeof(UINT8));
				status = PciIo->Mem.Read(PciIo, EfiPciWidthUint8, 0, 0x03, 1, VMAX);
				
				Print(L"VMIN : 0x%x, VMAx : 0x%x\n", *(UINT8*)VMIN, *(UINT8*)VMAX);


				UINT32 GTCL_BIT = 0x1;
				VOID* GTCL_START = (VOID*)&GTCL_BIT;
				
				status = PciIo->Mem.Write(PciIo, EfiPciWidthUint32, 0, 0x08, 1, GTCL_START);



				UINT32 changed = 1;
				while (changed) {
					VOID* GCTL_READ = (VOID*)AllocateZeroPool(sizeof(UINT32));
					status = PciIo->Mem.Read(PciIo, EfiPciWidthUint32, 0, 0x08, 1, GCTL_READ);
					UINT32 GTCL_FINAL = *(UINT32*)GCTL_READ;
					Print(L"GTCL Final : 0x%x\n", GTCL_FINAL);
					if (GTCL_FINAL > 0) { changed = 0; }
				}
				changed = 1;
				gBS->Stall(2000000);

				
				VOID* STS_RESET = (VOID*)AllocateZeroPool(sizeof(UINT16));
				status = PciIo->Mem.Read(PciIo, EfiPciWidthUint16, 0, 0x0e, 1, STS_RESET);
				UINT16 STS_R = *(UINT16*)STS_RESET;
				Print(L"Reading STS immediate after GTCL is started: 0x%x\n", STS_R);


				VOID* FVMIN = (VOID*)AllocateZeroPool(sizeof(UINT8));
				status = PciIo->Mem.Read(PciIo, EfiPciWidthUint8, 0, 0x02, 1, FVMIN);

				VOID* FVMAX = (VOID*)AllocateZeroPool(sizeof(UINT8));
				status = PciIo->Mem.Read(PciIo, EfiPciWidthUint8, 0, 0x03, 1, FVMAX);

				Print(L"FVMIN : 0x%x, FVMAx : 0x%x\n", *(UINT8*)FVMIN, *(UINT8*)FVMAX);

				VOID* OUTPAY = (VOID*)AllocateZeroPool(sizeof(UINT16));
				status = PciIo->Mem.Read(PciIo, EfiPciWidthUint8, 0, 0x04, 1, OUTPAY);
				VOID* INPAY = (VOID*)AllocateZeroPool(sizeof(UINT16));
				status = PciIo->Mem.Read(PciIo, EfiPciWidthUint8, 0, 0x06, 1, INPAY);

				Print(L"OUTPAY : 0x%x, INPAY : 0x%x\n", *(UINT16*)OUTPAY, *(UINT16*)INPAY);
				while (changed) {
					VOID* STS_FINAL = (VOID*)AllocateZeroPool(sizeof(UINT16));
					status = PciIo->Mem.Read(PciIo, EfiPciWidthUint16, 0, 0x0e, 1, STS_FINAL);
					UINT16 STS_F = *(UINT16*)STS_FINAL;
					Print(L"STS : 0x%x\n", STS_F);
					if (EFI_ERROR(status)) {
						Print(L" STATESTS  Error : %r", status);
					}
					if (STS_F > 0) { changed = 0; }
					gBS->Stall(2000000);
				}
				


/*   Will Finish the rest when STATESTS is working fine 
				*(UINT32*)HDA_MEM = 0;
				PciIo->Mem.Read(PciIo, EfiPciWidthUint32, 0, 0x08, 1, &HDA_MEM);
				GTCL = *(UINT32*)HDA_MEM;
				gBS->Stall(20000);
				PciIo->Mem.Read(PciIo, EfiPciWidthUint16, 0, 0x0e, 1, &HDA_MEM);
				STATESTS = *(UINT16*)(HDA_MEM);
				Print(L"STATESTS Init : 0x%x", HDA_MEM);
				*(UINT32*)HDA_MEM =  0x1;
				PciIo->Mem.Write(PciIo, EfiPciWidthUint32, 0, 0x08, 1, &HDA_MEM);

				*(UINT32*)HDA_MEM = 0;
				
				GTCL = *(UINT32*)HDA_MEM;
				*(UINT32*)HDA_MEM = 0;
				gBS->Stall(20000);
				PciIo->Mem.Read(PciIo, EfiPciWidthUint16, 0, 0x0e, 1, &HDA_MEM);
				STATESTS = *(UINT16*)(HDA_MEM);
				*(UINT32*)HDA_MEM = 0;
				Print(L"Via PciIoMem \n");
				Print(L"GCAP 0x%x, GTCL : 0x%x, STATESTS : 0x%x\n", GCAP, GTCL, STATESTS);
				Print(L"OSS : 0x%x, ISS : 0x%x, BSS : 0x%x\n", HDA_OSS_COUNT(GCAP), HDA_ISS_COUNT(GCAP), HDA_BSS_COUNT(GCAP));
				
				*(UINT32*)HDA_MEM = 0x02;
				PciIo->Mem.Write(PciIo, EfiPciWidthUint32, 0, 0x68,1, &HDA_MEM);
				*(UINT32*)HDA_MEM = 0;
				UINT32 cAD = 0x00000001;
				UINT32 NID = 0x00000000;
				UINT32 CMD = 0x00000f00;
				UINT32 PRM = 0x00000004;

				UINT32 VERB = (cAD << 28) + (NID << 20) + (CMD << 8) + PRM;
				Print(L"VERB : 0x%x\n", VERB);

				*(UINT32*)HDA_MEM = VERB;
				PciIo->Mem.Write(PciIo, EfiPciWidthUint32, 0,0x60,1, &HDA_MEM);
				*(UINT32*)HDA_MEM = 0x01;
				*(UINT32*)HDA_MEM = 0;
				PciIo->Mem.Write(PciIo,EfiPciWidthUint32, 0,0x68,1, &HDA_MEM);
				gBS->Stall(20000);	
				*(UINT32*)HDA_MEM = 0;
				PciIo->Mem.Read(PciIo, EfiPciWidthUint32, 0, 0x68,1, &HDA_MEM);
				Print(L"ICS : 0x%x\n", HDA_MEM);
				*(UINT32*)HDA_MEM = 0;
				PciIo->Mem.Read(PciIo, EfiPciWidthUint32, 0, 0x64, 1, &HDA_MEM);
				Print(L"Codec Input : 0x%x\n", HDA_MEM);
				*/
			}
		
		
		}
/*
unsigned int y = 50;
  unsigned int BBP = 4;
  for (unsigned int x = 0; x < fb->Width / 2 * BBP; x++) {
	  *(unsigned int*)(x + (y * fb->PixelsPerScanLine * BBP) +fb->BaseAddress) = 0xffffffff;
  }
*/


  //NOW WE SHOULD READ SOME KEYS
  //YOU CAN ADD ANY OTHER OPTION HERE, WHEN CAPSLOCK ON THAT WILL PRINT FIRST CHAR CAPITALIZED BUT NOT REST OF CHARACTERS SINCE WE RESET...
  //IF YOU WANT MORE KEYS SIMPLY IN VISUAL STUDIO TYPE SCAN_ AND YOU WILL SEE OTHER KEYS

  /*...OTHER...*/
/*
  while ((UINTN)Key.ScanCode != SCAN_ESC)
  {
      SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &KeyEvent);
      SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);
      SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
      Print(L"%c", Key.UnicodeChar);
  }
  */
  return EFI_SUCCESS;
}

// usb_descriptors.c
// Authors: Jacob Mealey <jacob.mealey@maine.edu>, 
//          Landyn Francis <landyn.francis@maine.edu>
// USB descriptor based on tinyusb docs + from 
// "Universal Serial Bus Device Class Definitions for MIDI Devices"
#include "tusb.h"


#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
        _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN)
#define EPNUM_MIDI 0x01

// This header is based on the usb midi document:
// "Universal Serial Bus Device Class Definitions for MIDI Devices"
// Appendix B.1
tusb_desc_device_t const descriptor = 
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200, // usb 2.0
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0xECEE,
    .idProduct          = USB_PID, 
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x00,
    .bNumConfigurations = 0x01
};

static uint16_t _desc_str[32];

enum
{
    ITF_NUM_MIDI = 0,
    ITF_NUM_MIDI_STREAMING,
    ITF_NUM_TOTAL
};

uint8_t const desc_fs_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 0, EPNUM_MIDI, 0x80 | EPNUM_MIDI, 64)
};

// returns the device descriptor for tiny usb to use
uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &descriptor;
}

// returns the fullspeed configurations
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    return desc_fs_configuration;
}


// Return a string representation for the usb descriptions
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void) langid;

  static const char *manufacturer = "Umaine ECE Capstone";
  static const char *product_name = "Hall Effect MIDI Keyboard";
  static const char *serials = "JL2040";
  static const uint16_t support_language  = 0x0409;   

  uint8_t chr_count;

  // index is related to what data is going to be sent over
  if(index == 0) {
      memcpy(&_desc_str[1], &support_language, 2);
      chr_count= 1;
  } else if(index == 1) {
      chr_count= strlen(manufacturer);
      for(uint8_t i = 0; (i < chr_count) && (i < 31); i++) {
          _desc_str[1 + i] = manufacturer[i];
      }
  } else if(index == 2) {
      chr_count= strlen(product_name);
      for(uint8_t i = 0; (i < chr_count) && (i < 31); i++) {
          _desc_str[1 + i] = product_name[i];
      }
  } else if(index == 3) {
      chr_count= strlen(serials);
      for(uint8_t i = 0; (i < chr_count) && (i < 31); i++) {
          _desc_str[1 + i] = serials[i];
      }
  } else {
      return NULL;
  }

  // we only have 32 words so we don't want to send over more
  if(chr_count > 31)
      chr_count = 31;

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);

  return _desc_str;
}



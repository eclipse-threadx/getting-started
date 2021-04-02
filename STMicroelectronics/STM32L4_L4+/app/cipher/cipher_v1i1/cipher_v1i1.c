// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "cipher_v1i1.h"
#include "az_ulib_result.h"
#include "interfaces/cipher_v1i1_interface.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define NUMBER_OF_KEYS 1
#define KEY_SIZE 21
static const char key[NUMBER_OF_KEYS][KEY_SIZE] = { "12345678912345678901" };

#define splitInt(intVal, bytePos) (char)((intVal >> (bytePos << 3)) & 0xFF)
#define joinChars(a, b, c, d) \
  (uint32_t)((uint32_t)a + ((uint32_t)b << 8) + ((uint32_t)c << 16) + ((uint32_t)d << 24))

az_result cipher_v1i1_create(void)
{
  az_result result;

  (void)printf("Create producer for cipher v1i1...\r\n");

  if ((result = publish_cipher_v1i1_interface()) != AZ_OK)
  {
    (void)printf("Publish interface cipher 1 failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Interface cipher 1 published with success\r\n");
  }

  return result;
}

az_result cipher_v1i1_destroy(void)
{
  (void)printf("Destroy producer for cipher 1.\r\n");

  return unpublish_cipher_v1i1_interface();
}

static inline uint32_t next_key_pos(uint32_t cur)
{
  uint32_t next = cur + 1;
  if (next == KEY_SIZE)
  {
    next = 0;
  }
  return next;
}

static uint32_t encoded_len(uint32_t size)
{
  return (size == 0) ? (0) : (((((size - 1) / 3) + 1) * 4) + 1);
}

static uint32_t decoded_len(const char* encodedString, uint32_t size)
{
  uint32_t result;
  uint32_t sourceLength = size;

  if (sourceLength == 0)
  {
    result = 0;
  }
  else
  {
    result = sourceLength / 4 * 3;
    if (encodedString[sourceLength - 1] == '=')
    {
      if (encodedString[sourceLength - 2] == '=')
      {
        result--;
      }
      result--;
    }
  }
  return result;
}

static char base64char(unsigned char val)
{
  char result;

  if (val < 26)
  {
    result = (char)(val + 'A');
  }
  else if (val < 52)
  {
    result = (char)(val - 26 + 'a');
  }
  else if (val < 62)
  {
    result = (char)(val - 52 + '0');
  }
  else if (val == 62)
  {
    result = (char)('+');
  }
  else
  {
    result = (char)('/');
  }

  return result;
}

static char base64b16(unsigned char val)
{
  const uint32_t base64b16values[4] = { joinChars('A', 'E', 'I', 'M'),
                                        joinChars('Q', 'U', 'Y', 'c'),
                                        joinChars('g', 'k', 'o', 's'),
                                        joinChars('w', '0', '4', '8') };
  return splitInt(base64b16values[val >> 2], (val & 0x03));
}

static char base64b8(unsigned char val)
{
  const uint32_t base64b8values = joinChars('A', 'Q', 'g', 'w');
  return splitInt(base64b8values, val);
}

static int base64toValue(char base64character, unsigned char* value)
{
  int result = 0;
  if (('A' <= base64character) && (base64character <= 'Z'))
  {
    *value = (unsigned char)(base64character - 'A');
  }
  else if (('a' <= base64character) && (base64character <= 'z'))
  {
    *value = (unsigned char)(('Z' - 'A') + 1 + (base64character - 'a'));
  }
  else if (('0' <= base64character) && (base64character <= '9'))
  {
    *value = (unsigned char)(('Z' - 'A') + 1 + ('z' - 'a') + 1 + (base64character - '0'));
  }
  else if ('+' == base64character)
  {
    *value = 62;
  }
  else if ('/' == base64character)
  {
    *value = 63;
  }
  else
  {
    *value = 0;
    result = -1;
  }
  return result;
}

static size_t numberOfBase64Characters(const char* encodedString)
{
  size_t length = 0;
  unsigned char junkChar;
  while (base64toValue(encodedString[length], &junkChar) != -1)
  {
    length++;
  }
  return length;
}

az_result cipher_v1i1_encrypt(
    uint32_t context,
    const char* const src,
    uint32_t src_size,
    uint32_t dst_buffer_size,
    char* dst,
    uint32_t* dst_size)
{
  AZ_ULIB_TRY
  {
    AZ_ULIB_THROW_IF_ERROR((context < NUMBER_OF_KEYS), AZ_ERROR_NOT_SUPPORTED);
    AZ_ULIB_THROW_IF_ERROR(
        (encoded_len(src_size) + 1 <= dst_buffer_size), AZ_ERROR_NOT_ENOUGH_SPACE);

    uint32_t key_pos = 0;

    dst[0] = (char)(context + '0');

    uint32_t destinationPosition = 1;
    uint32_t currentPosition = 0;
    char src_char[3];
    while (src_size - currentPosition >= 3)
    {
      src_char[0] = src[currentPosition] ^ key[context][key_pos];
      key_pos = next_key_pos(key_pos);
      src_char[1] = src[currentPosition + 1] ^ key[context][key_pos];
      key_pos = next_key_pos(key_pos);
      src_char[2] = src[currentPosition + 2] ^ key[context][key_pos];
      key_pos = next_key_pos(key_pos);

      char c1 = base64char((unsigned char)(src_char[0] >> 2));
      char c2 = base64char((unsigned char)(((src_char[0] & 3) << 4) | (src_char[1] >> 4)));
      char c3 = base64char((unsigned char)(((src_char[1] & 0x0F) << 2) | ((src_char[2] >> 6) & 3)));
      char c4 = base64char((unsigned char)(src_char[2] & 0x3F));
      currentPosition += 3;
      dst[destinationPosition++] = c1;
      dst[destinationPosition++] = c2;
      dst[destinationPosition++] = c3;
      dst[destinationPosition++] = c4;
    }
    if (src_size - currentPosition == 2)
    {
      src_char[0] = src[currentPosition] ^ key[context][key_pos];
      key_pos = next_key_pos(key_pos);
      src_char[1] = src[currentPosition + 1] ^ key[context][key_pos];
      char c1 = base64char((unsigned char)(src_char[0] >> 2));
      char c2 = base64char((unsigned char)(((src_char[0] & 0x03) << 4) | (src_char[1] >> 4)));
      char c3 = base64b16(src_char[1] & 0x0F);
      dst[destinationPosition++] = c1;
      dst[destinationPosition++] = c2;
      dst[destinationPosition++] = c3;
      dst[destinationPosition++] = '=';
    }
    else if (src_size - currentPosition == 1)
    {
      src_char[0] = src[currentPosition] ^ key[context][key_pos];
      char c1 = base64char((unsigned char)(src_char[0] >> 2));
      char c2 = base64b8(src_char[0] & 0x03);
      dst[destinationPosition++] = c1;
      dst[destinationPosition++] = c2;
      dst[destinationPosition++] = '=';
      dst[destinationPosition++] = '=';
    }

    dst[destinationPosition] = '\0';

    *dst_size = destinationPosition + 1;
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}

az_result cipher_v1i1_decrypt(
    const char* const src,
    uint32_t src_size,
    uint32_t dst_buffer_size,
    char* dst,
    uint32_t* dst_size)
{
  AZ_ULIB_TRY
  {
    AZ_ULIB_THROW_IF_ERROR((src_size > 1), AZ_ERROR_ARG);
    uint32_t context = (uint32_t)(src[0] - '0');
    AZ_ULIB_THROW_IF_ERROR((context < NUMBER_OF_KEYS), AZ_ERROR_NOT_SUPPORTED);
    AZ_ULIB_THROW_IF_ERROR(
        (decoded_len(&src[1], src_size) <= dst_buffer_size), AZ_ERROR_NOT_ENOUGH_SPACE);

    size_t numberOfEncodedChars;
    size_t indexOfFirstEncodedChar;
    size_t decodedIndex;

    numberOfEncodedChars = numberOfBase64Characters(&src[1]);
    indexOfFirstEncodedChar = 1;
    decodedIndex = 0;
    while (numberOfEncodedChars >= 4)
    {
      unsigned char c1;
      unsigned char c2;
      unsigned char c3;
      unsigned char c4;
      (void)base64toValue(src[indexOfFirstEncodedChar], &c1);
      (void)base64toValue(src[indexOfFirstEncodedChar + 1], &c2);
      (void)base64toValue(src[indexOfFirstEncodedChar + 2], &c3);
      (void)base64toValue(src[indexOfFirstEncodedChar + 3], &c4);
      dst[decodedIndex] = (char)((c1 << 2) | (c2 >> 4));
      decodedIndex++;
      dst[decodedIndex] = (char)(((c2 & 0x0f) << 4) | (c3 >> 2));
      decodedIndex++;
      dst[decodedIndex] = (char)(((c3 & 0x03) << 6) | c4);
      decodedIndex++;
      numberOfEncodedChars -= 4;
      indexOfFirstEncodedChar += 4;
    }
    if (numberOfEncodedChars == 2)
    {
      unsigned char c1;
      unsigned char c2;
      (void)base64toValue(src[indexOfFirstEncodedChar], &c1);
      (void)base64toValue(src[indexOfFirstEncodedChar + 1], &c2);
      dst[decodedIndex++] = (char)((c1 << 2) | (c2 >> 4));
    }
    else if (numberOfEncodedChars == 3)
    {
      unsigned char c1;
      unsigned char c2;
      unsigned char c3;
      (void)base64toValue(src[indexOfFirstEncodedChar], &c1);
      (void)base64toValue(src[indexOfFirstEncodedChar + 1], &c2);
      (void)base64toValue(src[indexOfFirstEncodedChar + 2], &c3);
      dst[decodedIndex++] = (char)((c1 << 2) | (c2 >> 4));
      dst[decodedIndex++] = (char)(((c2 & 0x0f) << 4) | (c3 >> 2));
    }

    uint32_t key_pos = 0;
    for (uint32_t i = 0; i < decodedIndex; i++)
    {
      dst[i] ^= key[context][key_pos];
      key_pos = next_key_pos(key_pos);
    }

    dst[decodedIndex] = '\0';

    *dst_size = (uint32_t)decodedIndex + 1;
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}

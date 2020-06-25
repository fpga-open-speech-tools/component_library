/** @file custom_functions.h

    File containing fixed point to/from string conversions and custom strcat for drivers.

    A few functions were copy pasted into multiple device drivers. Now as long as this file
    is included, those functions can be used without the need for copy paste. Also any changes made
    in this file apply anywhere that function is used, vs needing to copy paste that change to all
    files. These functions are based off Ray's code, but changed to be (hopefully) more readable
    and better commented.

    @author Aaron Koenigsberg based off Ray's code
    @date 2019
    @copyright 2019 Audio Logic

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
    PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
    FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    DeveloperName
    Audio Logic
    985 Technology Blvd
    Bozeman, MT 59718
    openspeech@flatearthinc.com
*/


#ifndef UTIL_FUNCS_H_
#define UTIL_FUNCS_H_

#include <linux/types.h>


#define ARBITRARY_CUTOFF_LEN 9

/** fp_to_string: Based off Ray's fp_to_string. Turns a uint32_t interpreted as a fixed point into a string.

@param buf, buffer in which to fill the string. It is assumed to have enough space. If a buflen parameter were passed it would
be simple to add a check to make sure that no writing goes past the bound of the array.

@param fp_num, the uint32_t to be interpreted as a fixed point to be translated into a string

@param fractional_bits, the number of fractional bits for the interpretation. No consideration is made to check that it
is a sensible number. i.e. less than 32, more than 0.

@param is_signed, whether or not to interpret the first bit as a sign. If true, the first bit is inspected then dumped.

@param num_decimals, number of decimals to write to the string.

@returns the length of the buffered string.
*/
int fp_to_string(char * buf, uint32_t fp_num, size_t fractional_bits, bool is_signed, uint8_t num_decimals) {
  int buf_index = 0;
  int int_mag = 1;
  int int_part;
  int frac_part;
  int i = 0;
  int32_t int_mask = 0x00000000;  // intMask turns into a bitstring with 0's in the location of the integer part of fp_num
  for (i = 0; i < fractional_bits; i++) {
    int_mask = int_mask << 1;
    int_mask += 1;
  }
  if (is_signed) {  // if it signed, need to add '-' to the buffer as well as remove that bit from the bitstring
    if (fp_num & 0x80000000) {
      buf[buf_index++] = '-';
    }
    fp_num = fp_num & 0x7fffffff;
  }
  int_part = (fp_num >> fractional_bits);  // shift away the fractional bits
  while (int_part / int_mag > 9) {  // find the magnitude of the integer part
    int_mag *= 10;
  }
  while (int_mag > 0) {  // decrement the magnitude as we move one digit at a time from 'intPart' to the string
    // common int to ascii conversion, since ints are sequential in ascii
    buf[buf_index++] = (char)(int_part / int_mag + '0');
    int_part %= int_mag;  // remove the first digit
    int_mag /= 10;       // decrease by one order of magnitude
  }
  buf[buf_index++] = '.';
  // get rid of the integer part. Also drop the last bit, not sure why this has to happen but if it doesn't there are some errors.
  // I believe that this results in ever so slightly incorrectly translated nums, but idk
  frac_part = (fp_num & int_mask) >> 1;
  for (i = 0; i < num_decimals; ++i) {
    frac_part *= 10;  // shift the digit up, (maybe related to why dropping the last bit above?)
    buf[buf_index++] = (frac_part >> (fractional_bits - 1)) + '0'; // inspect the digit that moved past the point
    frac_part &= int_mask >> 1;  // get rid of any bits that move past the point
  }
  buf[buf_index] = '\0';
  return buf_index;
}

/** set_fixed_num. Based off Ray's set_fixed_num. Converts a given string to a uint32_t interpreted as a fixed point.

@param s, the string to be converted.

@param num_fractional_bits, the number of fractional bits in the fixed point. No consideration is made to validate.

@param is_signed, whether or not to interpret the string as a signed or not.

@returns returns a uint32_t that is a fixed point representation based on the num_fractional_bits and is_signed params.
*/
uint32_t set_fixed_num(const char * s, int num_fractional_bits, bool is_signed) {
  int int_part_decimal = 0;
  int frac_part_decimal = 0;
  int frac_len = 0;
  int frac_comp = 1;
  int string_index = 0;
  int point_index = 0;  // point index will keep track of where the point is
  bool seen_point = false;
  uint32_t accumulator = 0;
  int i;
  // get the info from the string
  while (s[string_index] != '\0') {
    if (s[string_index] == '.') {  // if the point is found, need to switch from int accumulating to fraction
      point_index = string_index;
      seen_point = true;
    }
    else if (string_index == 0 && s[0] == '-') {  // if its the first char and its a negative sign, don't sweat
      // I don't think anything needs to happen here.
    }
    else if (!seen_point) {
      int_part_decimal *= 10;  // shift digits left, then add the new digit
      // common ascii to int conversion trick, since ints are sequential in ascii
      int_part_decimal += (int)(s[string_index] - '0');
    }
    else if (frac_len < ARBITRARY_CUTOFF_LEN) {  // do not allow the len of the fraction to exceed 9.
      frac_part_decimal *= 10;  // shift digits left, then add the new digit
      frac_part_decimal += (int)(s[string_index] - '0');
      // common ascii to int conversion trick, since ints are sequential in ascii
      frac_len++;  // need to keep track of the length
      frac_comp *= 10;
    }
    else {
      break;
    }
    string_index++;
  }

  while (frac_len < ARBITRARY_CUTOFF_LEN) {  // if the fraction len < 9 we want to make it 9
    frac_part_decimal *= 10;
    frac_len++;
    frac_comp *= 10;
  }
  // convert the decimal fraction to binary info. 32 is arbitrary, it is the precision of the conversion. extra
  // precision beyond the number of fractional bits in the fixed point num will be truncated off.
  for (i = 0; i < num_fractional_bits; i++) {
    // if frac part divided by frac comp is greater than 1, a 1 should be appended to bitstring
    if (frac_part_decimal / frac_comp) {
      accumulator += 0x00000001;
      frac_part_decimal -= frac_comp;
    }
    frac_part_decimal *= 2;
    accumulator = accumulator << 1;
  }
  accumulator += int_part_decimal << num_fractional_bits;
  if (is_signed && s[0] == '-') {
    accumulator |= 0x80000000; // if its a signed int and theres a negative sign flip the first bit of accumulator
  }
  return accumulator;
}

char *strcat2(char *dst, char *src) {
  char *cp = dst;
  while (*cp != '\0') {
    cp++;
  }
  // copies from src to cp, until the null char of src is copied.
  while ((*cp++ = *src++) != '\0');
  return dst;
}

#endif

#include "uint256.h"
#include <stdio.h>
#include <string.h>

#include "str_encodings.h"
template <unsigned int BITS>
base_blob<BITS>::base_blob(const std::vector<unsigned char>& valueChar)
{
	assert(valueChar.size() == sizeof(data));
	memcpy(data, &valueChar[0], sizeof(data));
}
template <unsigned int BITS>
std::string base_blob<BITS>::ToString() const
{
	return (GetHex());
}

template <unsigned int BITS>
void base_blob<BITS>::SetHex(const std::string& str)
{
	SetHex(str.c_str());
}

template <unsigned int BITS>
void base_blob<BITS>::SetHex(const char* psz)
{
	memset(data, 0, sizeof(data));
	while (isspace(*psz))
		psz++;
	if (psz[0] == '0' && tolower(psz[1]) == 'x')
		psz += 2;
	const char* pbegin = psz;
	while (::HexDigit(*psz) != -1)
		psz++;
	psz--;
	unsigned char* p1 = (unsigned char*)data;
	unsigned char* pend = p1 + WIDTH;
	while (psz >= pbegin && p1 < pend) {
		*p1 = ::HexDigit(*psz--);
		if (psz >= pbegin) {
			*p1 |= ((unsigned char)::HexDigit(*psz--) << 4);
			p1++;
		}
	}
}

template <unsigned int BITS>
std::string base_blob<BITS>::GetHex() const
{
	return HexStr(std::reverse_iterator<const uint8_t*>(data + sizeof(data)), std::reverse_iterator<const uint8_t*>(data));
}

template std::string base_blob<160>::ToString() const;
template std::string base_blob<160>::GetHex() const;
template base_blob<160>::base_blob(const std::vector<unsigned char>&);
template void base_blob<160>::SetHex(const std::string&);
template void base_blob<160>::SetHex(const char*);

template base_blob<256>::base_blob(const std::vector<unsigned char>&);
template void base_blob<256>::SetHex(const std::string&);
template void base_blob<256>::SetHex(const char*);
template std::string base_blob<256>::ToString() const;
template std::string base_blob<256>::GetHex() const;
      
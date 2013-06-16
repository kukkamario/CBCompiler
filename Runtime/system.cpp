#include <stdio.h>
#include <time.h>
#include "cbstring.h"
#include <iostream>
#include <locale>
#include <stdint.h>

typedef u_int32_t uint32_t;
typedef u_int8_t uint8_t;

static
std::codecvt_base::result
ucs4_to_utf8(const char32_t* frm, const char32_t* frm_end, const char32_t*& frm_nxt,
			 uint8_t* to, uint8_t* to_end, uint8_t*& to_nxt,
			 unsigned long Maxcode = 0x10FFFF)
{
	frm_nxt = frm;
	to_nxt = to;
	/*if (mode & generate_header)
	{
		if (to_end-to_nxt < 3)
			return codecvt_base::partial;
		*to_nxt++ = static_cast<uint8_t>(0xEF);
		*to_nxt++ = static_cast<uint8_t>(0xBB);
		*to_nxt++ = static_cast<uint8_t>(0xBF);
	}*/
	for (; frm_nxt < frm_end; ++frm_nxt)
	{
		uint32_t wc = *frm_nxt;
		if ((wc & 0xFFFFF800) == 0x00D800 || wc > Maxcode)
			return std::codecvt_base::error;
		if (wc < 0x000080)
		{
			if (to_end-to_nxt < 1)
				return std::codecvt_base::partial;
			*to_nxt++ = static_cast<uint8_t>(wc);
		}
		else if (wc < 0x000800)
		{
			if (to_end-to_nxt < 2)
				return std::codecvt_base::partial;
			*to_nxt++ = static_cast<uint8_t>(0xC0 | (wc >> 6));
			*to_nxt++ = static_cast<uint8_t>(0x80 | (wc & 0x03F));
		}
		else if (wc < 0x010000)
		{
			if (to_end-to_nxt < 3)
				return std::codecvt_base::partial;
			*to_nxt++ = static_cast<uint8_t>(0xE0 |  (wc >> 12));
			*to_nxt++ = static_cast<uint8_t>(0x80 | ((wc & 0x0FC0) >> 6));
			*to_nxt++ = static_cast<uint8_t>(0x80 |  (wc & 0x003F));
		}
		else // if (wc < 0x110000)
		{
			if (to_end-to_nxt < 4)
				return std::codecvt_base::partial;
			*to_nxt++ = static_cast<uint8_t>(0xF0 |  (wc >> 18));
			*to_nxt++ = static_cast<uint8_t>(0x80 | ((wc & 0x03F000) >> 12));
			*to_nxt++ = static_cast<uint8_t>(0x80 | ((wc & 0x000FC0) >> 6));
			*to_nxt++ = static_cast<uint8_t>(0x80 |  (wc & 0x00003F));
		}
	}
	return std::codecvt_base::ok;
}

extern "C" void CBF_printI(int i) {
	printf("%i\n", i);
}

extern "C" void CBF_printF(float f) {
	printf("%f\n", f);
}

extern "C" void CBF_printS(CBString s) {
#ifdef _WIN //sizeof(wchar_t) == 2
	std::cout << "Fix printS\n";
#else //wchar_t == char32_t
	if (s == 0) {
		printf("\n");
		return;
	}
	std::string utf8_str(s->mString.size() * 4, '\0');
	const char32_t* fromNext;
	uint8_t* toNext;
	ucs4_to_utf8(&s->mString[0], &s->mString[s->mString.size()], fromNext, (uint8_t*)&utf8_str[0], (uint8_t*)&utf8_str[utf8_str.size()], toNext);
	utf8_str.resize((char*)toNext - &utf8_str[0]);
	std::cout << utf8_str << "\n";
#endif
}

extern "C" void CBF_print() {
	printf("\n");
}

extern "C" int CBF_timer() {
	return clock() * 1000 / CLOCKS_PER_SEC;
}

#pragma once

#include <type_traits>
#include <utility>
#include <string>
#include <iterator>
#include "visibility.h"

namespace pv {

class DSO_LOCAL urldec;
class urldec {
private:
	template<typename Iterator>
	struct helper {
		typedef typename std::conditional<
			std::is_array<
				typename std::remove_reference<Iterator>::type
			>::value,
			typename std::decay<Iterator>::type,
			Iterator
		>::type type;
	};
public:
	urldec() : sl(0) {}

	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	static auto dec(II1&&, const II2&, OI&&) -> typename helper<OI>::type;

	template<typename II1, typename II2,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
				 >::value, int>::type = 0>
	static std::string dec(II1&& i1, const II2& i2);

	template<typename Sequence>
	static std::string dec(Sequence&& s);

	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	auto chunk_dec(II1&&, const II2&, OI&&)
		-> typename helper<OI>::type;

	template<typename OI>
	auto end_dec(OI&&) -> typename helper<OI>::type;
private:
	template<typename II1, typename II2, typename OI>
	static OI decode(II1&&, const II2&, OI&&);

	template<typename II1, typename II2, typename OI>
	OI chunk_decode(II1&&, const II2&, OI&&);

	template<typename OI>
	OI end_decode(OI&&);

	unsigned int sl;
	char state;
};

template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
urldec::dec(II1&& s, const II2& e, OI&& o)
	-> typename urldec::template helper<OI>::type {
	return decode(std::forward<typename helper<II1>::type>(s), e,
		      std::forward<typename helper<OI>::type>(o));
}

template<typename II1, typename II2,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value, int>::type>
std::string
urldec::dec(II1&& i1, const II2& i2) {
	std::string r;
	dec(std::forward<II1>(i1), i2, std::back_inserter(r));
	return r;
}

template<typename Sequence>
std::string
urldec::dec(Sequence&& s) {
	return dec(s.begin(), s.end());
}


template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
urldec::chunk_dec(II1&& s, const II2& e, OI&& o)
	-> typename urldec::template helper<OI>::type {
	return chunk_decode(std::forward<typename helper<II1>::type>(s), e,
			    std::forward<typename helper<OI>::type>(o));
}

template<typename OI>
auto
urldec::end_dec(OI&& o)
	-> typename urldec::template helper<OI>::type {
	return end_decode(std::forward<typename helper<OI>::type>(o));
}

template<typename II1, typename II2, typename OI>
inline OI
urldec::decode(II1&& i1, const II2& i2, OI&& o) {
	while (i1 != i2) {
		char c = *i1;
		++i1;
		switch (c) {
		case '+':
			*o = ' ';
			++o;
			break;
		case '%': {
			if (i1 == i2) {
				*o = '%';
				++o;
				return std::forward<OI>(o);
			}
			char x = *i1;
			++i1;
			if (!isxdigit(x)) {
				*o = '%';
				++o;
				*o = x;
				++o;
			} else {
				if (i1 == i2) {
					*o = '%';
					++o;
					*o = x;
					++o;
					return std::forward<OI>(o);
				}
				char y = *i1;
				++i1;
				if (!isxdigit(y)) {
					*o = '%';
					++o;
					*o = x;
					++o;
					*o = y;
					++o;
				} else {
					/*
					  if x >= [Aa] and x <= [Ff]
					  then 0x41 <= x <= 0x46 or
					       0x61 <= x <= 0x66
					  x - '0' is then x - 0x30
					  0x11 <= x <= 0x16 or
					  0x31 <= x <= 0x36
					  x & 0x0f is then
					  0x01 <= x <= 0x06
					  0x0a <= (x & 0x0f) + 9 <= 0x0f
					 */
					x -= '0';
					if (x > 9)
						x = (x & 0x0f) + 9;
					y -= '0';
					if (y > 9)
						y = (y & 0x0f) + 9;
					*o = (x << 4) | y;
					++o;
				}
			}
			break;
		}
		default:
			*o = c;
			++o;
			break;			
		}
	}
	return std::forward<OI>(o);
}

template<typename II1, typename II2, typename OI>
inline OI
urldec::chunk_decode(II1&& i1, const II2& i2, OI&& o) {
	while (i1 != i2) {
		char c = *i1;
		++i1;
		if (0 == sl)
			switch (c) {
			case '+':
				*o = ' ';
				++o;
				break;
			case '%':
				sl = 1;
				break;
			default:
				*o = c;
				++o;
				break;
			}
		else {
			// sl is 1 or 2
			// store = "%" or "%x"
			if (!isxdigit(c)) {
				*o = '%';
				++o;
				if (sl > 1) {
					*o = state;
					++o;
				}
				*o = c;
				++o;
				sl = 0;
			} else if (1 == sl) {
				state = c;
				sl = 2;
			} else {
				// sl is 2
				sl = 0;
				char x = state - '0';
				if (x > 9)
					x = (x & 0x0f) + 9;
				char y = c - '0';
				if (y > 9)
					y = (y & 0x0f) + 9;
				*o = (x << 4) | y;
				++o;
			}
		}
	}
	return std::forward<OI>(o);
}

template<typename OI>
inline OI
urldec::end_decode(OI&& o) {
	if (0 != sl) {
		*o = '%';
		++o;
		if (2 == sl) {
			*o = state;
			++o;
		}
		sl = 0;
	}
	return std::forward<OI>(o);
}

class DSO_LOCAL urlenc;
class urlenc {
public:
private:
	template<typename Iterator>
	struct helper {
		typedef typename std::conditional<
			std::is_array<
				typename std::remove_reference<Iterator>::type
			>::value,
			typename std::decay<Iterator>::type,
			Iterator
		>::type type;
	};
public:
	template<typename II1, typename II2, typename OI,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
			 >::value,
		 int>::type = 0>
	static auto enc(II1&&, const II2&, OI&&) -> typename helper<OI>::type;

	template<typename II1, typename II2,
		 typename std::enable_if<
			 std::is_same<
				 typename std::decay<II1>::type,
				 typename std::decay<II2>::type
				 >::value, int>::type = 0>
	static std::string enc(II1&& i1, const II2& i2);

	template<typename Sequence>
	static std::string enc(Sequence&& s);

private:
	template<typename II1, typename II2, typename OI>
	static OI encode(II1&&, const II2&, OI&&);
};


template<typename II1, typename II2, typename OI,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value,
	 int>::type>
auto
urlenc::enc(II1&& s, const II2& e, OI&& o)
	-> typename urlenc::template helper<OI>::type {
	return encode(std::forward<typename helper<II1>::type>(s), e,
		      std::forward<typename helper<OI>::type>(o));
}

template<typename II1, typename II2,
	 typename std::enable_if<
		 std::is_same<
			 typename std::decay<II1>::type,
			 typename std::decay<II2>::type
		 >::value, int>::type>
std::string
urlenc::enc(II1&& i1, const II2& i2) {
	std::string r;
	enc(std::forward<II1>(i1), i2, std::back_inserter(r));
	return r;
}

template<typename Sequence>
std::string
urlenc::enc(Sequence&& s) {
	return enc(s.begin(), s.end());
}


template<typename II1, typename II2, typename OI>
inline OI
urlenc::encode(II1&& i1, const II2& i2, OI&& o) {
	const char *h = "0123456789abcdef";
	while (i1 != i2) {
		char c = *i1;
		++i1;

		if ((('A' <= c && 'Z' >= c) ||
		     ('a' <= c && 'z' >= c) ||
		     ('0' <= c && '9' >= c))
		    || '-' == c || '_' == c || '.' == c || '~' == c) {
			*o = c;
			++o;
		} else {
			*o = '%';
			++o;
			*o = h[(c >> 4) & 0x0f];
			++o;
			*o = h[c & 0x0f];
			++o;
		}
	}
	return std::forward<OI>(o);
}

class DSO_LOCAL url_codec;
class url_codec : public urlenc, public urldec {
};

// common instantiations
// dec
extern template std::string
url_codec::dec<std::string>(std::string&&);
extern template std::string
url_codec::dec<std::string&>(std::string&);
extern template std::string
url_codec::dec<const std::string&>(const std::string&);

extern template std::string
url_codec::dec<std::string::iterator,
	       std::string::iterator, 0>(
		       std::string::iterator&&,
		       const std::string::iterator&);
extern template std::string
url_codec::dec<std::string::iterator&,
	       std::string::iterator, 0>(
		       std::string::iterator&,
		       const std::string::iterator&);
extern template std::string
url_codec::dec<std::string::const_iterator,
	       std::string::const_iterator, 0>(
		       std::string::const_iterator&&,
		       const std::string::const_iterator&);
extern template std::string
url_codec::dec<std::string::const_iterator&,
	       std::string::const_iterator, 0>(
		       std::string::const_iterator&,
		       const std::string::const_iterator&);

// extern template std::string
// url_codec::dec<std::string_view>(std::string_view&&);
// extern template std::string
// url_codec::dec<std::string_view&>(std::string_view&);
// extern template std::string
// url_codec::dec<const std::string_view&>(const std::string_view&);

// extern template std::string
// url_codec::dec<std::string_view::const_iterator,
// 	       std::string_view::const_iterator, 0>(
// 		       std::string_view::const_iterator&&,
// 		       const std::string_view::const_iterator&);
// extern template std::string
// url_codec::dec<std::string_view::const_iterator&,
// 	       std::string_view::const_iterator, 0>(
// 		       std::string_view::const_iterator&,
// 		       const std::string_view::const_iterator&);

extern template std::string
url_codec::dec<char const *, char const *, 0>(
	char const * &&, char const * const &);
extern template std::string
url_codec::dec<char *, char *, 0>(char * &&, char * const &);
extern template std::string
url_codec::dec<char const * &, char const *, 0>(
	char const * &, char const * const &);
extern template std::string
url_codec::dec<char * &, char *, 0>(char * &, char * const &);

// enc
extern template std::string
url_codec::enc<std::string>(std::string&&);
extern template std::string
url_codec::enc<std::string&>(std::string&);
extern template std::string
url_codec::enc<const std::string&>(const std::string&);

extern template std::string
url_codec::enc<std::string::iterator,
	       std::string::iterator, 0>(std::string::iterator&&,
					 const std::string::iterator&);
extern template std::string
url_codec::enc<std::string::iterator&,
	       std::string::iterator, 0>(std::string::iterator&,
					 const std::string::iterator&);
extern template std::string
url_codec::enc<std::string::const_iterator,
	       std::string::const_iterator, 0>(
		       std::string::const_iterator&&,
		       const std::string::const_iterator&);
extern template std::string
url_codec::enc<std::string::const_iterator&,
	       std::string::const_iterator, 0>(
		       std::string::const_iterator&,
		       const std::string::const_iterator&);

// extern template std::string
// url_codec::enc<std::string_view>(std::string_view&&);
// extern template std::string
// url_codec::enc<std::string_view&>(std::string_view&);
// extern template std::string
// url_codec::enc<const std::string_view&>(const std::string_view&);

// extern template std::string
// url_codec::enc<std::string_view::const_iterator,
// 	       std::string_view::const_iterator, 0>(
// 		       std::string_view::const_iterator&&,
// 		       const std::string_view::const_iterator&);
// extern template std::string
// url_codec::enc<std::string_view::const_iterator&,
// 	       std::string_view::const_iterator, 0>(
// 		       std::string_view::const_iterator&,
// 		       const std::string_view::const_iterator&);

extern template std::string
url_codec::enc<char const *, char const *, 0>(
	char const * &&, char const * const &);
extern template std::string
url_codec::enc<char *, char *, 0>(char * &&, char * const &);
extern template std::string
url_codec::enc<char const * &, char const *, 0>(
	char const * &, char const * const &);
extern template std::string
url_codec::enc<char * &, char *, 0>(char * &, char * const &);

}

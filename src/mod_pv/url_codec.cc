#include "url_codec.hh"
#include <string>
// #include "string_view.hpp"

namespace pv {

template std::string
url_codec::dec<std::string>(std::string&&);
template std::string
url_codec::dec<std::string&>(std::string&);
template std::string
url_codec::dec<const std::string&>(const std::string&);

template std::string
url_codec::dec<std::string::iterator,
	       std::string::iterator, 0>(
		       std::string::iterator&&,
		       const std::string::iterator&);
template std::string
url_codec::dec<std::string::iterator&,
	       std::string::iterator, 0>(
		       std::string::iterator&,
		       const std::string::iterator&);
template std::string
url_codec::dec<std::string::const_iterator,
	       std::string::const_iterator, 0>(
		       std::string::const_iterator&&,
		       const std::string::const_iterator&);
template std::string
url_codec::dec<std::string::const_iterator&,
	       std::string::const_iterator, 0>(
		       std::string::const_iterator&,
		       const std::string::const_iterator&);

// template std::string
// url_codec::dec<std::string_view>(std::string_view&&);
// template std::string
// url_codec::dec<std::string_view&>(std::string_view&);
// template std::string
// url_codec::dec<const std::string_view&>(const std::string_view&);

// template std::string
// url_codec::dec<std::string_view::const_iterator,
// 	       std::string_view::const_iterator, 0>(
// 		       std::string_view::const_iterator&&,
// 		       const std::string_view::const_iterator&);
// template std::string
// url_codec::dec<std::string_view::const_iterator&,
// 	       std::string_view::const_iterator, 0>(
// 		       std::string_view::const_iterator&,
// 		       const std::string_view::const_iterator&);

template std::string
url_codec::dec<char const *, char const *, 0>(
	char const * &&, char const * const &);
template std::string
url_codec::dec<char *, char *, 0>(char * &&, char * const &);
template std::string
url_codec::dec<char const * &, char const *, 0>(
	char const * &, char const * const &);
template std::string
url_codec::dec<char * &, char *, 0>(char * &, char * const &);

// enc
template std::string
url_codec::enc<std::string>(std::string&&);
template std::string
url_codec::enc<std::string&>(std::string&);
template std::string
url_codec::enc<const std::string&>(const std::string&);

template std::string
url_codec::enc<std::string::iterator,
	       std::string::iterator, 0>(std::string::iterator&&,
					 const std::string::iterator&);
template std::string
url_codec::enc<std::string::iterator&,
	       std::string::iterator, 0>(std::string::iterator&,
					 const std::string::iterator&);
template std::string
url_codec::enc<std::string::const_iterator,
	       std::string::const_iterator, 0>(
		       std::string::const_iterator&&,
		       const std::string::const_iterator&);
template std::string
url_codec::enc<std::string::const_iterator&,
	       std::string::const_iterator, 0>(
		       std::string::const_iterator&,
		       const std::string::const_iterator&);

// template std::string
// url_codec::enc<std::string_view>(std::string_view&&);
// template std::string
// url_codec::enc<std::string_view&>(std::string_view&);
// template std::string
// url_codec::enc<const std::string_view&>(const std::string_view&);

// template std::string
// url_codec::enc<std::string_view::const_iterator,
// 	       std::string_view::const_iterator, 0>(
// 		       std::string_view::const_iterator&&,
// 		       const std::string_view::const_iterator&);
// template std::string
// url_codec::enc<std::string_view::const_iterator&,
// 	       std::string_view::const_iterator, 0>(
// 		       std::string_view::const_iterator&,
// 		       const std::string_view::const_iterator&);

template std::string
url_codec::enc<char const *, char const *, 0>(
	char const * &&, char const * const &);
template std::string
url_codec::enc<char *, char *, 0>(char * &&, char * const &);
template std::string
url_codec::enc<char const * &, char const *, 0>(
	char const * &, char const * const &);
template std::string
url_codec::enc<char * &, char *, 0>(char * &, char * const &);

}


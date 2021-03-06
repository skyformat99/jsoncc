/*
   Copyright (c) 2015, 2016, Andreas Fett. All rights reserved.
   Use of this source code is governed by a BSD-style
   license that can be found in the LICENSE file.
*/

#include <jsoncc.h>
#include <cassert>
#include <iomanip>

namespace {

class indent : public std::streambuf {
public:
	explicit indent(std::ostream &, std::string indent = "\t");
	~indent();

protected:
	int overflow(int);

private:
	std::string indent_;
	std::streambuf* dest_;
	bool line_start_;
	std::ostream* owner_;
};

indent::indent(std::ostream& dest, std::string indent)
:
	indent_(indent),
	dest_(dest.rdbuf()),
	line_start_(true),
	owner_(&dest)
{
	owner_->rdbuf(this);
}

indent::~indent()
{
	if (owner_ != nullptr) {
		owner_->rdbuf(dest_);
	}
}

int indent::overflow(int ch)
{
	if (line_start_ && ch != '\n') {
		dest_->sputn(indent_.c_str(), indent_.size());
	}
	line_start_ = (ch == '\n');
	return dest_->sputc(ch);
}


std::ostream & quote(std::ostream & os, std::string const& in)
{
	os << '"';
	for (auto c: in) {
		switch (c) {
/*
   All Unicode characters may be placed within the quotation marks,
   except for the characters that must be escaped: quotation mark,
   reverse solidus, and the control characters (U+0000 through U+001F).
*/
#define ESCAPE(val, sym) case val: os << "\\" sym; break
		ESCAPE(0x00, "u0000"); ESCAPE(0x01, "u0001");
		ESCAPE(0x02, "u0002"); ESCAPE(0x03, "u0003");
		ESCAPE(0x04, "u0004"); ESCAPE(0x05, "u0005");
		ESCAPE(0x06, "u0006"); ESCAPE(0x07, "u0007");
		ESCAPE(0x08, "b");     ESCAPE(0x09, "t");
		ESCAPE(0x0a, "n");     ESCAPE(0x0b, "u000b");
		ESCAPE(0x0c, "f");     ESCAPE(0x0d, "r");
		ESCAPE(0x0e, "u000e"); ESCAPE(0x0f, "u000f");
		ESCAPE(0x10, "u0010"); ESCAPE(0x11, "u0011");
		ESCAPE(0x12, "u0012"); ESCAPE(0x13, "u0013");
		ESCAPE(0x14, "u0014"); ESCAPE(0x15, "u0015");
		ESCAPE(0x16, "u0016"); ESCAPE(0x17, "u0017");
		ESCAPE(0x18, "u0018"); ESCAPE(0x19, "u0019");
		ESCAPE(0x1a, "u001a"); ESCAPE(0x1b, "u001b");
		ESCAPE(0x1c, "u001c"); ESCAPE(0x1d, "u001d");
		ESCAPE(0x1e, "u001e"); ESCAPE(0x1f, "u001f");
		ESCAPE(0x22, "\"");    ESCAPE(0x5c, "\\");
#undef ESCAPE
		default:
			os << c;
		}
	}
	return os << '"';
}

enum IOS_Flags {
	IOS_NOINDENT = 1 << 0,
};

const int xalloc_id = std::ios_base::xalloc();

template <typename C>
std::ostream & container_indent(std::ostream & os, const char delim[3], C const& c)
{
	os << delim[0] << "\n";
	{
		indent in(os);
		std::string sep;
		for (auto item: c) {
			os << sep << item;
			sep = ",\n";
		}
	}
	return os << "\n" << delim[1] ;
}

template <typename C>
std::ostream & container_noindent(std::ostream & os, const char delim[3], C const& c)
{
	os << delim[0];
	std::string sep;
	for (auto item: c) {
		os << sep << item;
		sep = ", ";
	}
	return os << delim[1] ;
}

template <typename C>
std::ostream & stream_container(std::ostream & os, const char delim[3], C const& c)
{
	if (c.empty()) {
		return os << delim;
	}

	return (os.iword(xalloc_id) & ::IOS_NOINDENT) ?
		container_noindent(os, delim, c) : container_indent(os, delim, c);
}

}

namespace Json {

std::ios_base & indent(std::ios_base & os)
{
	os.iword(xalloc_id) &= ~::IOS_NOINDENT;
	return os;
}

std::ios_base & noindent(std::ios_base & os)
{
	os.iword(xalloc_id) |= ::IOS_NOINDENT;
	return os;
}

std::ostream & operator<<(std::ostream & os, Null const&)
{
	return os << "null";
}

std::ostream & operator<<(std::ostream & os, True const&)
{
	return os << "true";
}

std::ostream & operator<<(std::ostream & os, False const&)
{
	return os << "false";
}

std::ostream & operator<<(std::ostream & os, Number const& number)
{
	switch (number.type()) {
	case Number::TYPE_INVALID:
		assert(false);
		break;
	case Number::TYPE_INT:
		return os << number.int_value();
	case Number::TYPE_UINT:
		return os << number.uint_value();
	case Number::TYPE_FP:
		return os << std::fixed << number.fp_value();
	}

	return os;
}

std::ostream & operator<<(std::ostream & os, String const& string)
{
	return ::quote(os, string.value());
}

std::ostream & operator<<(std::ostream & os, Array const& array)
{
	return stream_container(os, "[]", array.elements());
}

std::ostream & operator<<(std::ostream & os, Member const& member)
{
	return os << member.key() << ": " << member.value();
}

std::ostream & operator<<(std::ostream & os, Object const& object)
{
	return stream_container(os, "{}", object.members());
}

std::ostream & operator<<(std::ostream & os, Value const& value)
{
	switch (value.tag()) {
	case Value::TAG_INVALID:
		assert(false);
		break;
	case Value::TAG_TRUE:
		return os << value.true_value();
	case Value::TAG_FALSE:
		return os << value.false_value();
	case Value::TAG_NULL:
		return os << value.null();
	case Value::TAG_NUMBER:
		return os << value.number();
	case Value::TAG_STRING:
		return os << value.string();
	case Value::TAG_OBJECT:
		return os << value.object();
	case Value::TAG_ARRAY:
		return os << value.array();
	}

	return os;
}

}

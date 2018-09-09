/*
	Helper class to debug some MPFloat, reflects an abbreviated class of mpfloat.
	PERSONAL IMPROVE: Improve this system. Somehow setup a way to copy-paste mpfloat.h with minimal edits and achieve the same 'setup'?
*/
#ifndef MPTEST
	#define MPTEST

namespace boost { namespace multiprecision {

template <class T>
class mpBase {
public:
	string str(std::streamsize digits = 0, std::ios_base::fmtflags f = 0) const
	{
		string result;
		result = "0.0";
		return result;
	}

	std::string debugStr() const { return std::string(str(0, 0)); }
};

class mpTest : public mpBase<mpTest> {
	public:
		ILINE constexpr mpTest()					: mpBase()	{}
		using mpBase::mpBase;
		using mpBase::operator=;
};

// Just to get debugStr() accessible in debugger memory regardless of whether it was called in the scope or not!
extern std::string IgnoreThis_MPSTUFF2(const mpBase<mpTest>& in);

}}  // END boost::multiprecision namespaces
using boost::multiprecision::mpTest;

#endif // MPTEST
#include <cppunit/extensions/HelperMacros.h>

namespace unittests {
namespace empty {

class test : public CppUnit::TestCase {
public:
	test();
	void setUp();
	void tearDown();

private:

	CPPUNIT_TEST_SUITE(test);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(test);

test::test()
{ }

void test::setUp()
{ }

void test::tearDown()
{ }

}}
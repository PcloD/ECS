#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ECSTest
{
	TEST_CLASS(UnitTest01)
	{
	public:
		TEST_METHOD(TestMethod01)
		{
			Assert::IsTrue(true);
		}
	};
}
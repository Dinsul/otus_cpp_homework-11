#define BOOST_TEST_MODULE test_bulk

#include "async.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(test_suite_mtbulk)

BOOST_AUTO_TEST_CASE(test_1)
{
    std::size_t bulk = 5;
    auto h  = async::connect(bulk);
    auto h2 = async::connect(bulk);
    async::receive(h, "1", 1);
    async::receive(h, "1", 1);
    async::receive(h, "\n", 1);
    async::receive(h, "1", 1);
    async::receive(h2, "2\n", 2);
    async::receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15);
    async::receive(h, "b\nc\nd\n}\n89\n", 11);
    async::disconnect(h);
    async::disconnect(h2);

    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "MegaBI.hpp"


#include <sstream>


TEST_CASE("test parser", "[parser]")
{
    using namespace std::string_literals;

    MegaBI::Accumulator acc;

    acc.parse("{\"ts_fact\": 946684824, \"fact_name\": \"fact10\", \"actor_id\": 111222, \"props\": {\"prop1\": 2, \"prop2\": 1}}");

    std::stringstream ss;
    ss << acc;

    REQUIRE(ss.str() == "{\"ts_fact\":\"2000-01-01\",\"fact_name\":\"fact10\",\"props\":[2,1],\"count\":1}\n"s);

    MegaBI::Accumulator acc2;
    std::stringstream().swap(ss);

    acc2.parse("{\"ts_fact\": 946684824, \"fact_name\": \"fact10\", \"actor_id\": 111222, \"props\": {\"prop1\": 2, \"prop2\": 1}}");
    acc2.parse("{\"ts_fact\": 946684824, \"fact_name\": \"fact11\", \"actor_id\": 111222, \"props\": {\"prop1\": 2, \"prop2\": 1}}");
    acc += acc2;

    ss << acc;

    REQUIRE(ss.str() == 
        "{\"ts_fact\":\"2000-01-01\",\"fact_name\":\"fact10\",\"props\":[2,1],\"count\":2}\n"s + 
        "{\"ts_fact\":\"2000-01-01\",\"fact_name\":\"fact11\",\"props\":[2,1],\"count\":1}\n");
}

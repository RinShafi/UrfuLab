#pragma once

#include <boost/log/trivial.hpp>     // для BOOST_LOG_TRIVIAL
#include <boost/log/expressions.hpp> // для выражения, задающего фильтр
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/json.hpp>

namespace boost_logger
{

namespace json = boost::json;

BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", json::value)

void InitLogging();

} // namespace boost_logger

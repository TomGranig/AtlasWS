#pragma once

#include <iostream>
#include <algorithm>

#include "../type.hpp"
#include "../handler/connection.hpp"
#include "../handler/request.hpp"
#include "../error/error-message.hpp"
#include "../http-parse/http-parse.hpp"

namespace atlas {

    void receive_tick(http_session& sess);

}
#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>

namespace atlas {
    constexpr uint16_t HTTP_CONTINUE = 100;
    constexpr uint16_t HTTP_SWITCHING_PROTOCOLS = 101;
    constexpr uint16_t HTTP_PROCESSING = 102;

    constexpr uint16_t HTTP_OK = 200;
    constexpr uint16_t HTTP_CREATED = 201;
    constexpr uint16_t HTTP_ACCEPTED = 202;
    constexpr uint16_t HTTP_NON_AUTHORITATIVE_INFORMATION = 203;
    constexpr uint16_t HTTP_NO_CONTENT = 204;
    constexpr uint16_t HTTP_RESET_CONTENT = 205;
    constexpr uint16_t HTTP_PARTIAL_CONTENT = 206;
    constexpr uint16_t HTTP_MULTI_STATUS = 207;
    constexpr uint16_t HTTP_ALREADY_REPORTED = 208;
    constexpr uint16_t HTTP_IM_USED = 226;

    constexpr uint16_t HTTP_MULTIPLE_CHOICES = 300;
    constexpr uint16_t HTTP_MOVED_PERMANENTLY = 301;
    constexpr uint16_t HTTP_FOUND = 302;
    constexpr uint16_t HTTP_SEE_OTHER = 303;
    constexpr uint16_t HTTP_NOT_MODIFIED = 304;
    constexpr uint16_t HTTP_USE_PROXY = 305;
    constexpr uint16_t HTTP_SWITCH_PROXY = 306;
    constexpr uint16_t HTTP_TEMPORARY_REDIRECT = 307;
    constexpr uint16_t HTTP_PERMANENT_REDIRECT = 308;

    // 4xx Client Errors
    constexpr uint16_t HTTP_BAD_REQUEST = 400;
    constexpr uint16_t HTTP_UNAUTHORIZED = 401;
    constexpr uint16_t HTTP_PAYMENT_REQUIRED = 402;
    constexpr uint16_t HTTP_FORBIDDEN = 403;
    constexpr uint16_t HTTP_NOT_FOUND = 404;
    constexpr uint16_t HTTP_METHOD_NOT_ALLOWED = 405;
    constexpr uint16_t HTTP_NOT_ACCEPTABLE = 406;
    constexpr uint16_t HTTP_PROXY_AUTHENTICATION_REQUIRED = 407;
    constexpr uint16_t HTTP_REQUEST_TIMEOUT = 408;
    constexpr uint16_t HTTP_CONFLICT = 409;
    constexpr uint16_t HTTP_GONE = 410;
    constexpr uint16_t HTTP_LENGTH_REQUIRED = 411;
    constexpr uint16_t HTTP_PRECONDITION_FAILED = 412;
    constexpr uint16_t HTTP_PAYLOAD_TOO_LARGE = 413;
    constexpr uint16_t HTTP_URI_TOO_LONG = 414;
    constexpr uint16_t HTTP_UNSUPPORTED_MEDIA_TYPE = 415;
    constexpr uint16_t HTTP_RANGE_NOT_SATISFIABLE = 416;
    constexpr uint16_t HTTP_EXPECTATION_FAILED = 417;
    constexpr uint16_t HTTP_IM_A_TEAPOT = 418;
    constexpr uint16_t HTTP_MISDIRECTED_REQUEST = 421;
    constexpr uint16_t HTTP_UNPROCESSABLE_ENTITY = 422;
    constexpr uint16_t HTTP_LOCKED = 423;
    constexpr uint16_t HTTP_FAILED_DEPENDENCY = 424;
    constexpr uint16_t HTTP_UPGRADE_REQUIRED = 426;
    constexpr uint16_t HTTP_PRECONDITION_REQUIRED = 428;
    constexpr uint16_t HTTP_TOO_MANY_REQUESTS = 429;
    constexpr uint16_t HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE = 431;
    constexpr uint16_t HTTP_UNAVAILABLE_FOR_LEGAL_REASONS = 451;

    // Server Error responses
    constexpr uint16_t HTTP_INTERNAL_SERVER_ERROR = 500;
    constexpr uint16_t HTTP_NOT_IMPLEMENTED = 501;
    constexpr uint16_t HTTP_BAD_GATEWAY = 502;
    constexpr uint16_t HTTP_SERVICE_UNAVAILABLE = 503;
    constexpr uint16_t HTTP_GATEWAY_TIMEOUT = 504;
    constexpr uint16_t HTTP_VERSION_NOT_SUPPORTED = 505;
    constexpr uint16_t HTTP_VARIANT_ALSO_NEGOTIATES_EXPERIMENTAL = 506;
    constexpr uint16_t HTTP_INSUFFICIENT_STORAGE = 507;
    constexpr uint16_t HTTP_LOOP_DETECTED = 508;
    constexpr uint16_t HTTP_NOT_EXTENDED = 510;
    constexpr uint16_t HTTP_NETWORK_AUTHENTICATION_REQUIRED = 511;

    std::string get_status_str(uint16_t status);
}
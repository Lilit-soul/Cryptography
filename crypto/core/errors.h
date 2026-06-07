#ifndef ERRORS_H
#define ERRORS_H

#include <string>

enum class ErrorCode : int {
    SUCCESS = 0,
    ERR_NO_CIPHER = 1,
    ERR_FILE_NOT_FOUND = 2,
    ERR_FILE_EMPTY = 3,
    ERR_FILE_CORRUPTED = 4,
    ERR_CANNOT_CREATE = 5,
    ERR_DECRYPT_FAIL = 6,
    ERR_INVALID_KEY = 7,
    ERR_NO_MEMORY = 8,
    ERR_PERMISSION_DENIED = 9,
    ERR_INVALID_FORMAT = 10,
    ERR_AUTH_FAILED = 11,
    ERR_CONFIG_CORRUPTED = 12
};


std::string errorToString(ErrorCode code);
void safeShowError(ErrorCode code, const std::string& details = "");

#endif

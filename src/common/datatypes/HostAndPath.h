/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#ifndef COMMON_DATATYPES_HOSTANDPATH_H_
#define COMMON_DATATYPES_HOSTANDPATH_H_

#include "common/base/Base.h"
#include "common/datatypes/HostAddr.h"

namespace nebula {

struct HostAndPath {
    HostAddr host;
    std::string path;

    HostAndPath(HostAddr h, std::string p) : host(std::move(h)), path(p) {}

    std::string toString() const {
        std::stringstream os;
        os << "host: " << host << ", path: " << path;
        return os.str();
    }
};

inline std::ostream& operator <<(std::ostream& os, const HostAndPath& hostPath) {
    return os << hostPath.toString();
}

}  // namespace nebula

#endif  // COMMON_DATATYPES_HOSTANDPATH_H_

/* Copyright (c) 2021 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

namespace nebula {

bool HostAndPath::operator==(const HostAndPath& rhs) const {
    return host == rhs.host && path == rhs.path;
}

bool HostAndPath::operator!=(const HostAndPath& rhs) const {
    return !(*this == rhs);
}

}  // namespace nebula

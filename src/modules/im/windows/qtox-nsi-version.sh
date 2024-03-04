#!/bin/bash

#
# Copyright (c) 2022 船山信息 chuanshaninfo.com
# The project is licensed under Mulan PubL v2.
# You can use this software according to the terms and conditions of the Mulan
# PubL v2. You may obtain a copy of Mulan PubL v2 at:
#          http://license.coscl.org.cn/MulanPubL-2.0
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PubL v2 for more details.
#


# script to change qTox version in `.nsi` files to supplied one
#
# requires:
#  * files `qtox.nsi` and `qtox64.nsi` in working dir
#  * GNU sed

# usage:
#
#   ./$script $version
#
# $version has to be composed of at least one number/dot

set -eu -o pipefail


# change version in .nsi files in the right line
change_version() {
    for nsi in *.nsi
    do
        sed -i -r "/DisplayVersion/ s/\"[0-9\\.]+\"$/\"$@\"/" "$nsi"
    done
}

# exit if supplied arg is not a version
is_version() {
    if [[ ! $@ =~ [0-9\\.]+ ]]
    then
        echo "Not a version: $@"
        exit 1
    fi
}

main() {
    is_version "$@"

    change_version "$@"
}
main "$@"

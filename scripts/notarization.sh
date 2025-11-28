#!/bin/bash
# 
# The running machine must have Apple ID credentials pre-registered in the key chain under
# the KC item AC_PASSWORD

# Note: this only works on macOS, since it relies on Apple tools.

# Usage:
# notarize.sh some/path/file.zip some/other/path/binary staple
# Put file "binary" from "some/other/path/" in zip file "file.zip" in directory "some/path/" and staple the result.
#
# notarize.sh file.zip /some/path/to/binary
# Put file "binary" from "/some/path/to/" in "file.zip" in the current working directory, and notarize it. Do not staple.

# Definitely don't call this without at least two arguments. It does not do much (any...) checking for correctness.

# Both these should be full paths, or relative from the current working directory.
zip_path=$1
app_path=$2

# If this equals "staple", do staple. If it's missing or is anything else, do not.
staple=$3

rm "$zip_path"
zip -rq --symlink "$zip_path" "$app_path"

if [ -z $APP_SPECIFIC_PASSWORD ]; then
    echo "APP_SPECIFIC_PASSWORD or APPLE_TEAM_ID is not set. Using Keychain for credentials"
    xcrun notarytool submit "$zip_path" \
                   --keychain-profile "AC_PASSWORD" \
                   --wait \
                   --timeout 10m
else
    echo "Using username, password and team-id for credentials"
    xcrun notarytool submit "$zip_path" \
                   --apple-id "${APPLE_ID}" \
                   --team-id "${APPLE_TEAM_ID}" \
                   --password "${APP_SPECIFIC_PASSWORD}" \
                   --wait \
                   --timeout 10m
fi

if [ $staple = "staple" ]
then
    xcrun stapler staple "$app_path"
fi

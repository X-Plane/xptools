#!/bin/bash
#
# Notarizes (and optionally staples) a mac binary.
#
# Credentials, in order of preference:
#   1. App Store Connect API key via env: APPLE_NOTARY_KEY (the .p8 contents),
#      APPLE_NOTARY_KEY_ID, APPLE_NOTARY_ISSUER_ID. This is what CI uses --
#      API keys survive Apple ID password rotations, app-specific passwords don't.
#   2. A keychain profile named AC_PASSWORD (for manual runs on a dev Mac,
#      set up once with `xcrun notarytool store-credentials "AC_PASSWORD" ...`).

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

if [ -n "$APPLE_NOTARY_KEY_ID" ]; then
    echo "Using App Store Connect API key for credentials"
    key_file="${RUNNER_TEMP:-${TMPDIR:-/tmp}}/notary_key.p8"
    printf '%s' "$APPLE_NOTARY_KEY" > "$key_file"
    xcrun notarytool submit "$zip_path" \
                   --key "$key_file" \
                   --key-id "$APPLE_NOTARY_KEY_ID" \
                   --issuer "$APPLE_NOTARY_ISSUER_ID" \
                   --wait \
                   --timeout 10m
    result=$?
    rm -f "$key_file"
    [ $result -eq 0 ] || exit $result
else
    echo "APPLE_NOTARY_KEY_ID is not set. Using Keychain for credentials"
    xcrun notarytool submit "$zip_path" \
                   --keychain-profile "AC_PASSWORD" \
                   --wait \
                   --timeout 10m
fi

if [ $staple = "staple" ]
then
    xcrun stapler staple "$app_path"
fi

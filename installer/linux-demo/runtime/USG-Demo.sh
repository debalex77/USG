#!/usr/bin/env bash

set -Eeuo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"$here/demo-tools/configure-demo-profile.sh" "$here" --if-missing
exec "$here/USG.sh" "$@"

#!/usr/bin/env bash

set -Eeuo pipefail

target_dir="${1:-}"
mode="${2:---force}"

if [[ -z "$target_dir" ]]; then
    printf 'Utilizare: %s DIRECTOR_INSTALARE [--force|--if-missing]\n' "$0" >&2
    exit 2
fi

target_dir="$(readlink -f "$target_dir")"
main_database="$target_dir/demo/usg_demo.sqlite3"
image_database="$target_dir/demo/usg_demo_images.sqlite3"

if [[ ! -f "$main_database" || ! -f "$image_database" ]]; then
    printf 'Bazele demo nu au fost găsite în %s/demo.\n' "$target_dir" >&2
    exit 1
fi

config_root="${XDG_CONFIG_HOME:-$HOME/.config}"
config_dir="$config_root/USG"
profile="$config_dir/usg_demo.conf"

mkdir -p "$config_dir" "$target_dir/logs" "$target_dir/reports" "$target_dir/video"

if [[ "$mode" == "--if-missing" && -f "$profile" ]]; then
    exit 0
fi

if [[ -f "$profile" && ! -f "$profile.pre-demo.bak" ]]; then
    cp -p -- "$profile" "$profile.pre-demo.bak"
fi

temporary_profile="$(mktemp "$config_dir/.usg_demo.conf.XXXXXX")"
cleanup_profile() {
    rm -f -- "$temporary_profile"
}
trap cleanup_profile EXIT

# idUserApp și nameUserApp folosesc codificarea profilurilor legacy:
# 1 -> Cg==, admin -> Wl9WUlU=. Parola nu este memorată și este goală.
printf '%s\n' \
    '[index_init]' \
    'indexLangApp=1' \
    'indexTypeSQL=2' \
    'indexUnitMeasure=0' \
    '' \
    '[path_app]' \
    "docsTemplatesPath=$target_dir/templets" \
    "reportsPath=$target_dir/reports" \
    "videoDirectory=$target_dir/video" \
    '' \
    '[connect]' \
    'MySQL_host=' \
    'MySQL_name_base=' \
    'MySQL_port=3306' \
    'MySQL_user=' \
    'MySQL_passwd_user=' \
    'MySQL_option_connect=' \
    'sqliteDatabaseName=usg_demo.sqlite3' \
    "sqliteDatabasePath=$main_database" \
    "imageDatabasePath=$image_database" \
    "logPath=$target_dir/logs/usg_demo.log" \
    '' \
    '[on_start]' \
    'idUserApp=Cg==' \
    'nameUserApp=Wl9WUlU=' \
    'memoryUser=true' \
    'numSavedFilesLog=10' \
    'initialSetupComplete=true' \
    > "$temporary_profile"

chmod 0600 "$temporary_profile"
mv -f -- "$temporary_profile" "$profile"
trap - EXIT

printf 'Profilul demo a fost creat: %s\n' "$profile"

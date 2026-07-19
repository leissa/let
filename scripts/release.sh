#!/usr/bin/env bash
set -euo pipefail

# Release fe and let in tandem with the same version number.
#
# For each repo (fe first, then let):
#   1. set project(... VERSION <version>) in CMakeLists.txt (skipped if already set)
#   2. commit the bump (in let: also the updated submodules/fe pointer)
#   3. tag v<version> and push branch + tag
#   4. create a GitHub release (FE <version> / Let <version>) with generated notes
#
# Usage: scripts/release.sh <version>
#   e.g. scripts/release.sh 0.9.3

die() { echo "error: $*" >&2; exit 1; }

[[ $# -eq 1 ]] || { echo "usage: $0 <version>  (e.g. $0 0.9.3)" >&2; exit 1; }
ver=$1
[[ $ver =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || die "version must look like X.Y.Z, got '$ver'"
tag=v$ver

let_dir=$(cd "$(dirname "$0")/.." && pwd)
fe_dir=$let_dir/submodules/fe

[[ -f $fe_dir/CMakeLists.txt ]] || die "fe submodule not found at $fe_dir (run: git submodule update --init --recursive)"
command -v gh >/dev/null || die "gh CLI not found"

check_repo() {
    local dir=$1
    local branch
    branch=$(git -C "$dir" rev-parse --abbrev-ref HEAD)
    [[ $branch == main ]] || die "$dir is on branch '$branch', expected 'main'"
    [[ -z $(git -C "$dir" status --porcelain) ]] || die "$dir has uncommitted changes"
    [[ -z $(git -C "$dir" tag -l "$tag") ]] || die "$dir already has tag $tag"
    git -C "$dir" fetch origin main
    git -C "$dir" merge-base --is-ancestor origin/main HEAD \
        || die "$dir is behind origin/main, pull first"
}

# Set the version in CMakeLists.txt; return 0 if the file changed.
bump_version() {
    local dir=$1 proj=$2
    sed -i -E "s/^project\($proj VERSION [0-9.]+\)/project($proj VERSION $ver)/" "$dir/CMakeLists.txt"
    grep -q "^project($proj VERSION $ver)" "$dir/CMakeLists.txt" \
        || die "failed to set version in $dir/CMakeLists.txt"
    ! git -C "$dir" diff --quiet -- CMakeLists.txt
}

echo "checking fe ($fe_dir) ..."
check_repo "$fe_dir"
echo "checking let ($let_dir) ..."
check_repo "$let_dir"

echo
echo "This will release fe $ver and let $ver:"
echo "  - commit version bumps, tag $tag, push to origin/main"
echo "  - create GitHub releases 'FE $ver' and 'Let $ver'"
read -rp "Continue? [y/N] " answer
[[ $answer == [yY] ]] || die "aborted"

## fe

if bump_version "$fe_dir" fe; then
    git -C "$fe_dir" commit -am "fe: bump version to $ver"
else
    echo "fe already at $ver, no bump commit needed"
fi
git -C "$fe_dir" tag "$tag"
git -C "$fe_dir" push origin main "$tag"
gh release create "$tag" --repo leissa/fe --title "FE $ver" --generate-notes

## let

bump_version "$let_dir" Let || true
git -C "$let_dir" add CMakeLists.txt submodules/fe
if [[ -n $(git -C "$let_dir" status --porcelain) ]]; then
    git -C "$let_dir" commit -m "bump to $ver"
else
    echo "let already at $ver, no bump commit needed"
fi
git -C "$let_dir" tag "$tag"
git -C "$let_dir" push origin main "$tag"
gh release create "$tag" --repo leissa/let --title "Let $ver" --generate-notes

echo
echo "released fe $ver and let $ver"
echo "note: release notes were auto-generated; edit them with:"
echo "  gh release edit $tag --repo leissa/fe --notes '...'"
echo "  gh release edit $tag --repo leissa/let --notes '...'"

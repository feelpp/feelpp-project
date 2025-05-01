#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  cat <<EOF
Usage: $0 <version> [<extra>]
  version  X.Y.Z       (e.g. 4.5.0)
  extra    -rc.1       (optional suffix)
EOF
  exit 1
fi

VERSION=$1
EXTRA="${2:-}"               # e.g. "-rc.1" or empty
TAG="v${VERSION}${EXTRA}"

echo
echo "🔖  Starting release ${TAG}"
echo "----------------------------------------"

echo "1️⃣  Updating CMake project version..."
sed -E -i.bak \
  -e "s#(project\(\s*feelpp-project\s+VERSION\s+)[0-9]+\.[0-9]+\.[0-9]+#\1${VERSION}#i" \
  CMakeLists.txt
rm CMakeLists.txt.bak
echo "   ✔️  project VERSION → ${VERSION}"

echo
echo "2️⃣  Updating CMake EXTRA_VERSION..."
sed -E -i.bak \
  -e "s#(set\s*\(\s*EXTRA_VERSION\s*\")[^\"]*(\"\))#\1${EXTRA}\2#i" \
  CMakeLists.txt
rm CMakeLists.txt.bak
echo "   ✔️  EXTRA_VERSION → \"${EXTRA}\""

echo
echo "3️⃣  Updating package.json version..."
sed -E -i.bak \
  -e "s#(\"version\"[[:space:]]*:[[:space:]]*\")[^\"]*(\")#\1${VERSION}${EXTRA}\2#i" \
  package.json
rm package.json.bak
echo "   ✔️  package.json → ${VERSION}${EXTRA}"

echo
echo "4️⃣  Updating pyproject.toml version..."
sed -E -i.bak \
  -e "s|^[[:space:]]*version[[:space:]]*=[[:space:]]*\".*\"|version = \"${VERSION}${EXTRA}\"|" \
  pyproject.toml
rm pyproject.toml.bak
echo "   ✔️  pyproject.toml → ${VERSION}${EXTRA}"

echo
echo "5️⃣  Committing changes..."
git add CMakeLists.txt package.json pyproject.toml 
git commit -m "Release ${TAG}"
echo "   ✔️  committed"

echo
echo "6️⃣  Creating Git tag ${TAG}..."
git tag -a "${TAG}" -m "Release ${TAG}"
echo "   ✔️  tagged"

echo
echo "7️⃣  Pushing commit + tags to origin..."
git push origin HEAD --follow-tags
echo "   ✔️  pushed"

echo
echo "🎉  Release ${TAG} complete!"
echo "----------------------------------------"
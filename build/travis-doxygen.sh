#!/bin/bash
# Update Doxygen documentation after push to 'master'.
# Author: @pah

set -e

SUDO=sudo
DOXYGEN_VER=doxygen-1.8.7
DOXYGEN_TAR=${DOXYGEN_VER}.linux.bin.tar.gz
DOXYGEN_URL="http://ftp.stack.nl/pub/users/dimitri/${DOXYGEN_TAR}"
DOXYGEN_BIN="/usr/local/bin/doxygen"

GHPAGES_REPO="miloyip/rapidjson"
GHPAGES_URL="https://github.com/${GHPAGES_REPO}"

skip() {
	echo "$@" 1>&2
	echo "Exiting..." 1>&2
	exit 0
}

abort() {
	echo "Error: $@" 1>&2
	echo "Exiting..." 1>&2
	exit 1
}

# TRAVIS_BUILD_DIR not set, exiting
[ -d "${TRAVIS_BUILD_DIR-/nonexistent}" ] || \
	abort '${TRAVIS_BUILD_DIR} not set or nonexistent.'

# check for pull-requests
[ "${TRAVIS_PULL_REQUEST}" = "false" ] || \
	skip "Not running Doxygen for pull-requests."

# check for branch name
[ "${TRAVIS_BRANCH}" = "master" ] || \
	skip "Running Doxygen only for updates on 'master' branch (current: ${TRAVIS_BRANCH})."

# check for job number
[ "${TRAVIS_JOB_NUMBER}" = "${TRAVIS_BUILD_NUMBER}.1" ] || \
	skip "Running Doxygen only on first job of build ${TRAVIS_BUILD_NUMBER} (current: ${TRAVIS_JOB_NUMBER})."

# install doxygen binary distribution
doxygen_install()
{
	wget -O - "${DOXYGEN_URL}" | \
		tar xz -C ${TMPDIR-/tmp} ${DOXYGEN_VER}/bin/doxygen
	$SUDO install -m 755 ${TMPDIR-/tmp}/${DOXYGEN_VER}/bin/doxygen \
		${DOXYGEN_BIN};
}

doxygen_run()
{
	cd "${TRAVIS_BUILD_DIR}";
	doxygen build/Doxyfile;
}

gh_pages_prepare()
{
	cd "${TRAVIS_BUILD_DIR}/doc";
	[ ! -d "html" ] || \
		abort "Doxygen target directory already exists."
	git clone --single-branch -b gh-pages ${GHPAGES_URL} html
	cd html
	# setup git config (with defaults)
	git config user.name "${GIT_NAME-travis}"
	git config user.email "${GIT_EMAIL-"travis@localhost"}"
	# clean working dir
	rm -f .git/index
	git clean -df
}

gh_pages_commit() {
	cd "${TRAVIS_BUILD_DIR}/doc/html";
	git add --all;
	git diff-index --quiet HEAD || git commit -m "Automatic doxygen build";
}

gh_pages_push() {
	# check for secure variables
	[ "${TRAVIS_SECURE_ENV_VARS}" = "true" ] || \
		skip "Secure variables not available, not updating GitHub pages."
	[ "${GH_TOKEN+set}" = set ] || \
		skip "GitHub access token not available, not updating GitHub pages."

	cd "${TRAVIS_BUILD_DIR}/doc/html";
	# setup credentials (hide in "set -x" mode)
	git config core.askpass /bin/true
	( set +x ; git config credential.${GHPAGES_URL}.username "${GH_TOKEN}" )
	# push to GitHub
	git push origin gh-pages
}

doxygen_install
gh_pages_prepare
doxygen_run
gh_pages_commit
gh_pages_push


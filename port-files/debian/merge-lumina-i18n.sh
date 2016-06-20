#!/bin/bash

CURDIR="${PWD}"
LNGDIR="${PWD}/lumina-i18n/ts"

git clone https://github.com/pcbsd/lumina-i18n.git

[[ ! -d "${LNGDIR}" ]] && (echo "something went wrong!" && exit 1)

LANGS=("${LNGDIR}"/*)
LANGS=(${LANGS[@]/*\/ts\/})

UTILS=( config desktop fileinfo fm info open screenshot search textedit xconfig )

for lng in ${LANGS[@]}; do
	for utl in ${UTILS[@]}; do
		if [[ -e "${LNGDIR}"/${lng}/lumina-${utl}.ts ]]; then
			if [[ -d "${CURDIR}"/src-qt5/core/lumina-${utl} ]]; then
				cp "${LNGDIR}"/${lng}/lumina-${utl}.ts \
					"${CURDIR}"/src-qt5/core/lumina-${utl}/i18n/lumina-${utl}_${lng}.ts
			elif [[ -d "${CURDIR}"/src-qt5/core-utils/lumina-${utl} ]]; then
				cp "${LNGDIR}"/${lng}/lumina-${utl}.ts \
					"${CURDIR}"/src-qt5/core-utils/lumina-${utl}/i18n/lumina-${utl}_${lng}.ts
			elif [[ -d "${CURDIR}"/src-qt5/desktop-utils/lumina-${utl} ]]; then
				cp "${LNGDIR}"/${lng}/lumina-${utl}.ts \
					"${CURDIR}"/src-qt5/desktop-utils/lumina-${utl}/i18n/lumina-${utl}_${lng}.ts
			fi
		fi
	done
done

echo "updated lumina translations."

# satisfy dpkg-source when building a GIT snapshot
# first ensure all translations are known to GIT
# then create a commit using only the translation files
# other changes to the source will still be tracked by dpkg-source
[[ -d "${CURDIR}"/.git ]] && ( git add src-qt5/*/*/i18n/*.ts && git commit src-qt5/*/*/i18n/*.ts \
	-m "merge latest translations from https://github.com/pcbsd/lumina-i18n.git" )

rm -rf lumina-i18n

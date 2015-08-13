#!/bin/bash

CURDIR="${PWD}"
LNGDIR="${PWD}/lumina-i18n/ts"

git clone https://github.com/pcbsd/lumina-i18n.git

[[ ! -d "${LNGDIR}" ]] && (echo "something went wrong!" && exit 1)

LANGS=("${LNGDIR}"/*)
LANGS=(${LANGS[@]/*\/ts\/})

UTILS=( config desktop fileinfo fm info open screenshot search xconfig )

echo ${LANGS[@]}

for lng in ${LANGS[@]}; do
	for utl in ${UTILS[@]}; do
		[[ -e "${LNGDIR}"/${lng}/lumina-${utl}.ts ]] && \
			cp "${LNGDIR}"/${lng}/lumina-${utl}.ts \
				"${CURDIR}"/lumina-${utl}/i18n/lumina-${utl}_${lng}.ts
	done
done

echo "updated lumina translations."
git commit */i18n/ -m "merge latest translations from https://github.com/pcbsd/lumina-i18n.git"
rm -rf lumina-i18n

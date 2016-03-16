
# Lumina Desktop Environment Icon Theme #

## Requirements ##

1. White foreground (primarily), with black shadow/outline. 
	The shadow/outline ensures that the icon will still be completely visible on a pure white background (allowing it to be used on any background color/type).
2. Color may be used (and is recommended) to distinguish between similar-looking icons.
	Example: the "folder.svg" icon is plain white/black, while the "folder-video.svg" icon will use the same base folder image, but might overlay a small colorized emblem somewhere on it.
3. All icons need be to be svg formatted, with a base/default size of 128x128 and usable by the stock/generic SVG library .
	Note: Adobe applications tend to use a customized version of the svg library, so make sure to verify the icon with some other SVG application such as inkscape.
4. All icons will be licensed under the Creative Commons Attribution 4.0 International Public License.
	https://creativecommons.org/licenses/by/4.0/legalcode
	Basically, the icons can be used any anyone, anywhere, in anything, as long as the original author still gets attributed for his work. This license is comparable to the BSD license for source code.

## Examples ##

Sample icons to give the general idea (to be updated later as the first icons are submitted):

* (shadowing) http://findicons.com/icon/80883/white18?id=81865
* (General icon style) http://www.gentleface.com/free_icon_set.html

This theme is intended to be high-contrast and function on high-resolution systems (4K+) with a more professional/streamlined appearance. The white foreground with black outline/shadowing will also permit the visibility of these icons on any background the user may utilize.

## Contributing ##

1. Icons which are still needed are listed in the TODO.txt files within each directory.
2. Submit the icon file via git pull request
3. Add an entry for your icon/submission to the AUTHORS file (to ensure each contributor gets proper credit for their work)
4. Remove the icon from the appropriate TODO.txt file/list

## Totals ##

* ~1249 icons total
* ~300 are mimetype icons (low priority initially)
* Many icons are slight variations of each other (arrows pointing different directions, alternate colors/emblems, etc..), so should be quite simple to submit icons in small groups (all single arrows, all double arrows, etc..)
 

### Relevant Links to the FreeDesktop Specifications ###
* Icon Theme Specification: https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
* Icon Naming Specification: https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html

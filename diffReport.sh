#!/bin/sh
echo "Running for Light Directory"
(find ~/lumina/icon-theme/material-design-light/*/*.svg | sed 's/.*material/material/' | sort) > currentFiles
(grep -rn material-design-light ./pkg-plist | sed 's/.*material/material/g' | sed 's/scalable\///' | sort) > darkFiles
diff currentFiles darkFiles
rm currentFiles darkFiles
echo "Running for Dark Directory"
(find ~/lumina/icon-theme/material-design-dark/*/*.svg | sed 's/.*material/material/' | sort) > currentFiles
(grep -rn material-design-dark ./pkg-plist | sed 's/.*material/material/g' | sed 's/scalable\///' | sort) > darkFiles
diff currentFiles darkFiles
rm currentFiles darkFiles

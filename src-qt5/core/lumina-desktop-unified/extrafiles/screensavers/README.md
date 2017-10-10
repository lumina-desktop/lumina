## Screensaver Format
The screensaver system for the Lumina desktop allows for the creation and use of scripts written in the QML language, with a number of screensavers and other examples installed out-of-box. There are only a couple warnings/caveats to consider when developing a new screensaver:

1. The root object in your QML script will be automatically sized to fit the screen as needed. Avoid trying to hard-code specific screen dimensions within your script as it will not work properly.
2. A JSON manifest file must be created (format listed below) and placed into one of the screensaver plugin directories for it to be recognized as a valid screensaver by the desktop.


### JSON Manifest
The manifest file contains all the information needed to actually validate/launch the screensaver, as well as additional information about the author and/or the screensaver itself.

Example JSON manifest file (sample.json):
```
{
  "name" : {
    "default" : "sample",
    "en_US" : "US English localization of the name",
    "en" : "Generic english localization of the name"
  },
 
  "description" : {
    "default" : "sample screensaver",
    "en_US" : "US English Localization of the description"
  },

  "author" : {
    "name" : "Me",
    "email" : "Me@myself.net",
    "website" : "http://mywebsite.net",
    "company" : "iXsystems",
    "company_website" : "http://ixsystems.com"
  },

  "meta" : {
    "license" : "3-clause BSD",
    "license_url" : "https://github.com/trueos/lumina/blob/master/LICENSE",
    "copyright" : "Copyright (c) 2017, Ken Moore (ken@ixsystems.com)",
    "date_created" : "20171010",
    "date_updated" : "20171011",
    "version" : "1.0"
  },

  "qml" : {
    "exec" : "absolute/or/relative/path/to/script.qml",
    "additional_files" : ["file/which/must/exist.png"],
    "qt_min_version" : "5.0",
    "qt_max_version" : "6.0"
  }
}
```

Details of the individual items in the manifest:
* NOTE: for locale codes, both long and short version are acceptable:
  Example 1: If the current locale is "en_GB", but the JSON manifest lists translations for "en_US" and "en", then the "en" translation will be used.
  Example 2: If the current locale is "en_GB", but neither "en_GB" nor "en" translations exist, then the "default" version will be used.

* **name** : (required) This is the official name of the screensaver to show to users
   * *default* : (required) Non-translated name of the screensaver
   * *[locale]* : (optional) Translated name for specific [locale]
* **description** : (required) This is a short description of the screensaver to show to users
   * *default* : (required) Non-translated description of the screensaver
   * *[locale]* : (optional) Translated description for specific [locale]
* **author** : (all optional) Additional information about the author(s) of the screensaver
   * *name* : Name of the author
   * *email* : Email to contact the author (useful for licensing questions and such)
   * *website* : Personal website for the author (github/facebook/twitter profile, etc)
   * *company* : Company for which the author is creating this screensaver
   * *company_website* : Website for the company
* **meta** : (all optional) Additional information about the screensaver itself
   * *license* : License the screensaver is released under
   * *license_url* : Website which contains the full text of the license
   * *copyright* : Copyright notice for this screensaver
   * *date_created* : (yyyyMMdd) Date the screensaver was initially created
   * *date_updated* : (yyyyMMdd) Date the screensaver was last updated
   * *version* : Current version of the screensaver (typically updated every time "date_updated" is changed)
* **qml** : (required) Information about launching the screensaver and checking validity
   * *exec* : (required) Absolute or relative path to the QML script (relative to the directory which contains the JSON manifest)
   * *additional_files* : (optional) Array of paths for other files/scripts which must exist for the screensaver to work properly.
   * *qt_min_version* : (optional) Minimum version of the Qt libraries that this screensaver supports
   * *qt_max_version* : (optional) Maximum version of the Qt libraries that this screensaver supports 

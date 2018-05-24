# JSON Menu Scripts
These are simple shell scripts which print out a block of JSON to standard out which Lumina will use to dynamically generate a menu.

## Syntax
It is an overall Json object containing a collection of additional Json objects which correspond to individual items in the menu.
There are currently two forms of the "item" Json objects:

### Type 1: "item"
This type is a simple action item entry with text, an icon, and some action to be performed when triggered. If the "action" is just the path to a non-executable file, Lumina will automatically open it with lumina-open to select the appropriate tool for the job.

Example:
```
{
  "Open a File" : {
    "type" : "item",
    "icon" : "folder",
    "action" : "/path/to/file/or/folder"
  },
  "Launch an application binary" : {
    "type" : "item",
    "icon" : "application-launch",
    "action" : "myapp --with-flags"
  },
  "Launch XDG application" : {
    "type" : "item",
    "icon" : "application-launch",
    "action" : "myapp.desktop"
  }
}
```

### Type 2: "jsonmenu"
This is a way to recursively put another menu within a menu by simply running another JSON menu script.
These additional menus will only be generated when the user hovers over the item - preventing sub-menus from causing delays in generating the parent menu and keeping things responsive.

Example:
```
{
  "My SubMenu" : {
    "type" : "jsonmenu",
    "icon" : "folder",
    "exec" : "/path/to/my_json_menu.sh"
   }
 }
 ```
 
 Additional types of menu items can be created on request. For those who are interesting in adding new types themselves, the code which parses these JSON objects and generates the menu is located in the Lumina source repository [HERE](https://github.com/lumina-desktop/lumina/blob/master/src-qt5/core/lumina-desktop/JsonMenu.h)

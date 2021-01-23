# WorldEditor Manual Howto

## File format

This WED manual is written in plain basic markdown, plus simple tables as in php-markdown / gfm / [multimarkdown](https://multimarkdown.com)
I
## Editing

Although any basic text edtor can be used for editing - a more specialized editor offer syntax highlighting and real-time previews is strongly preferred. Countless FOSS as well as proprietary option exist for this. Some are lacking some details in the previews - like tables or displaying images with local file paths (Win\ vs Unix/ path separator issues ...), so the following have been tested to work near perfect:

### Windows

* [Notepad++](https://notepad-plus-plus.org/) with the NPPMarkdownPanel plugin. Somewhat complex user interface, though.

* [Markdown Pad 2](https://markdownpad.com) nice and simple, although table previews require the paid for "pro" version

### OSX

* [MacDown](https://macdown.uranusjr.com) - very nice and simple GUI, its FOSS :)

* [MultiMarkdown Composer](https://multimarkdown.com) the free version on the AppStore is sufficient

### Linux

* [reText](https://github.com/retext-project/retext) Very nice and simple GUI, available as standard package under ubuntu, suse and fedora. There are OSX and Windows versions. but installation is complicated for those OS currently.

### All three OS

* [Sublime text](https://sublimetext.com) with MarkdownLive and MarkdownEditing packages.
 
* [Atom](https://atom.io) with language-markdown and markdown-fold plugins. 

Atom is more or less a FOSS clone of Sublime text. Both are VERY complex edit-anything systems with hundreds of plugins. The key challenge is to navigate (or avoid) the countless features. But both can do "folding" of markdown chapters for VERY easy moving around of whole chapters.
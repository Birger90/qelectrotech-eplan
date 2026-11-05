![](logo.png)


> ## This fork: QElectroTech for EPLAN schematics and manufacturer data
>
> 🇩🇪 [Deutsche Fassung](README.de.md)
>
> This fork extends QElectroTech so that EPLAN schematics can be rebuilt as
> real, linked projects — navigating like an EPLAN PDF and carrying their data the way EPLAN does — and so that
> manufacturer parts and cables can be used directly. The upstream
> ([qelectrotech-source-mirror](https://github.com/qelectrotech/qelectrotech-source-mirror)) is merged regularly;
> everything below exists only here.
>
> **Part import** (menu "Part import", headless: `--import-eplan`)
> - EPLAN macros and EDZ packages: graphics from DXF, schematic from `.ema`, terminals and article data; black box
>   frames, channel names and broken lines are cleaned up. Terminal designations are split per line, terminal
>   descriptions come from the right EPLAN property.
> - DWG drawings (AutoCAD 2004–2018) read directly, no converter; mounting views at the scale of the EPLAN macros.
> - Article data from WSCAD packages (`.wspak`).
> - Search and import straight from the EPLAN Data Portal (manufacturer list, filters, picture preview).
>
> **Navigation like EPLAN** (in QET and in the exported PDF)
> - EPLAN's three-way jump on a part: left third forward through the jump sequence of the device, middle to its main
>   function, right third backward (Ctrl+double-click in QET; three link areas in the PDF).
> - Free texts, parts and cables can point to a folio or to a part (`destination`): rows of terminal, cable and parts
>   lists jump to the device; Ctrl+click follows, the PDF link zooms onto it.
> - A linked folio report arrow is itself a link to its partner, not only its text. Filled report arrows in the
>   EPLAN look.
>
> **Data like EPLAN instead of typed text**
> - PLC: a card (PLC master) keeps its I/O table (address, function text, comment, terminals); each channel is bound
>   to its row (`group_index`) and shows those values through its fields — edit the table, the channels follow.
> - Contacts: a placed coil can carry its own contact groups (11/12/14, 21/22/24 …), like the function template of
>   an EPLAN article; the contacts take their terminal names from it and the contact mirror shows them.
> - Cross reference formula `%=M`: the plant is shown only when it differs from the sheet's own (`/42/1`,
>   `=DC/38/2`), as in EPLAN.
>
> **Sheets and title block**
> - Light grey stamps over the sheet like an EPLAN form: title block variables `watermark` (diagonal, e.g. the print
>   date) and `watermark-side` (upright at the right edge); empty = off, per folio or as project default.
> - New title block variables `%{printdate}` and `%{saveddate-de}` (DD.MM.YYYY).
> - Script API: `outsideSheet(folio)` lists what lies outside the sheet.
>
> **Cables**
> - Menu Project > *Cable list (choose types)…*: every cable of the project with type, cores, colours, plant,
>   location and folios; *Choose type…* changes the type in one undo step.
> - Manufacturer catalogues: every CSV in the folder `cable-catalogs` beside the cable type file is read as a
>   read-only catalogue. Filters for catalogue, number of cores, core cross section, energy chain suitability and
>   properties (shielded, halogen free, UL/CSA, torsion …).
> - `tools/kabelkatalog/kabelkatalog_eplan.py` builds such catalogues from the free EPLAN article data of the
>   manufacturers (e.g. igus chainflex, LAPP): cores with colours, cross section, outer diameter, voltage, article
>   number.
>
> **Cabinet layout and panel manufacturing** (menu *Cabinet layout*, the EPLAN Pro Panel 2D workflow)
> - Mounting surfaces (plate, door, wall) to scale, DIN rails, cable ducts and C profiles with their fixing rows
>   (thread or through hole, spacing, end distance, zero cut), cut-outs (round, rectangle, slot, hex, oct) and
>   keep-out areas -- ordinary shapes carrying a `<panel>` role, so they move, copy and undo like any shape.
> - *Place devices from the schematic*: each device becomes a thumbnail of its real size (stays out of the BOM),
>   snapped onto the DIN rails, carrying the drilling patterns of its part.
> - Mounting variants per part (`<drillingpatterns>`): on a DIN rail it leaves no hole, screwed it leaves its
>   pattern; the EDZ import reads width/height/depth, snap height, middle offset and the EPLAN `construction`.
> - Drilling view over the sheet, manufacturing report with warnings (hole in a keep-out area, overlaps, device
>   without pattern), export: one DXF R12 in mm per surface (layers per hole kind) plus hole list and cut list.
> - 3D view (Qt OpenGL) of plate, rails, ducts and devices; STEP models of parts are shown when QElectroTech is
>   built with Open CASCADE (MSYS2 package `opencascade`, found automatically by CMake). A click selects on the sheet.
> - Script API: `setShapePanel`, `shapePanel`, `panelReport`, `exportPanelDxf`, `stepInfo`.
>
> **Tools** (`tools/symbole/`)
> - `pdf_symbol.py` / `pdf_blatt.py`: take symbols or whole sheets 1:1 from a vector PDF (e.g. an EPLAN plan), with
>   real conductors and nodes.
> - `verknuepfung_setzen.py`: set the master/slave link type of imported manufacturer symbols by their role (coil,
>   contact, protection device, PLC module, PLC channel, device pin).
> - `anschluesse_ergaenzen.py`, `nachzeichnen_symbole.py`: add missing terminals, redraw your own symbols
>   (`bauteile/nachgezeichnet/`).
>
> **For AI assistants**: two skills in `.claude/skills/` (Claude Code loads them in a clone; other tools: read the
> `SKILL.md`, it points to its sub files):
> - `qet-schaltplan-zeichnen` – how to draw, check and link EPLAN-style sheets by script.
> - `eplan-wissen` – the EPLAN platform help 2.9 in own words (PDF jumps, cross references, designations, terminals,
>   cables, PLC, reports, macros/articles, property IDs, check messages …), every rule with its source URL and its
>   meaning for QET. Start: `.claude/skills/eplan-wissen/SKILL.md`.
>
> Manufacturer files themselves are not included (terms of use of the portals).

---

# QElectroTech

### What it is

QElectroTech, or QET in short, is a libre and open source desktop application to create diagrams and schematics.
The software is primarily intended to create electrical documentation but it can also be used to draw any kinds of diagrams, such as those made in pneumatics, hydraulics, process industries, electronics...
Generally speaking, QET is a **CAD/CAE editor focusing on schematics drawing features**.

This means that there are no embedded simulating or calculating functionalities and it is not planned to implement them.

The main goal of the developers is to provide a libre, easy to use and effective software for **schematics drawing purposes**.

### Version

The current stable version is 0.100 and was released on 2026.01.25.
Once it has been officially released, the stable version is always frozen and is no longer developed.

New functionalities, bug and issue fixings are further made in the development version (currently 0.100.1 or 0.200.0 if based on new Qt6 port), which can also be [downloaded](https://qelectrotech.org/download.php).

Users who want to test and take benefits from the last software implementations should use the development version. But... use it at your own risk, since things are sometimes broken or only partially implemented until they are done!

### License

The software is licensed under [GNU/GPL](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).
You are free to use, copy, modify and redistribute it under the terms of the license.

Like many other open source software, QElectroTech is provided as is, without any warranty.

### Development / technical choices

The development follows the classical way of free and open source software: the source code, written by a community of users, is freely accessible.
*    Get sources and sub-modules
```txt
git clone --recursive https://github.com/qelectrotech/qelectrotech-source-mirror.git
```

Here are the technical choices made for the software development:

*   Integrated development environment: [Qt Framework](https://www.qt.io/ide/)
*   Libraries: Qt 6.x
*   [KF6 Framework](https://github.com/KDE)
    [Cmake](https://cmake.org/install/)
    [kcoreaddons](https://invent.kde.org/frameworks/kcoreaddons)
    [kwidgetsaddons](https://invent.kde.org/frameworks/kwidgetsaddons).
*   Coding language: [C++](https://en.wikipedia.org/wiki/C%2B%2B)
*   GUI translations: [Qt Linguist](http://doc.qt.io/qt-6/qtlinguist-index.html)
*   Version control: [GIT](https://github.com/qelectrotech/qelectrotech-source-mirror.git)
*   Doxygen documentation :[Doxygen](https://download.qelectrotech.org/qet/doxygen/html/)
*   QtCreator qch doxygen :[QElectroTech.qch](https://download.qelectrotech.org/qet/doxygen/QElectroTech.qch)
*   File format for projects, elements and titleblocks: [XML](http://www.w3schools.com/xml/xml_whatis.asp)
*   Main development platform: [GNU/Linux](http://getgnulinux.org/en/linux/)
*   Targeted platforms: Windows, GNU/Linux, Mac OS X, BSDs
*   [Forum](https://qelectrotech.org/forum/index.php)
*   [Wiki](https://qelectrotech.org/wiki_new/)
*   [Mantis_bugtracker](https://qelectrotech.org/bugtracker/my_view_page.php)

If you wish to be informed of the latest developments, browse the [archive](https://listengine.tuxfamily.org/lists.tuxfamily.org/qet/) of the project mailing list where all commits (changes) are registered. This archive is publicly available, you don't need any account to access it.


# Features

QElectroTech is a free and open source software.
No need to worry about restrictive licensing, privacy violation or dependency on a company.
Zero cost and no licensing fees!
But you are welcome to make a donation to support the development

QElectroTech runs on the 3 most widespread operating systems for desktop computers in the world.
Files that were created on an OS can be edited on another OS without any conversion or restriction.
MS Windows users can even run the "ready-to-use" version of QElectroTech from an external medium with no need to install it on an access restricted computer.


Take advantage of the modern GUI

Toolbars and panels can be enabled/disabled, moved and displayed the way you want to work.
Panels can be stacked on each other (as tabs) or docked on the sides (as docks) or completely separated from the main window (as windows).
The GUI can fit to small or big screens, and even to multi-display configurations.
![](https://download.qelectrotech.org/qet/images-features/0030.png "GUI overview")


The GUI of QElectroTech is translated in 25 languages.
You only need to restart the application for the new selected language to take effect.
![](https://download.qelectrotech.org/qet/images-features/0040.png "Lang menu")

Create technical documentation in professional quality

Size, look and informations of the folios (sheets) are fully configurable.
You can set vertical and horizontal headers (printed rulers) individually on and off, set number of columns and rows, and set width/height of each column/row.

Titlebocks can be created and edited with the embedded titleblock editor to perfectly suit your needs.
Custom variables can be defined to display the informations you wish in the titleblock.
![](https://download.qelectrotech.org/qet/images-features/0055.png "Titleblock editor")

With only 2 mouse clicks you can add a full automatic generated table of content.
Changes in the documentation are updated on the fly.
![](https://download.qelectrotech.org/qet/images-features/0060.png "Table of content")

Choose from more than 8.200 symbols...
The embedded QET collection contains a rich library of electric, logic, pneumatic, hydraulic and fluid symbols.
The library grows at every new release thanks to an active user community.
![](https://download.qelectrotech.org/qet/images-features/0070.png "Collections")

...or create your own collection

The embedded element editor is a nice tool to create your own elements (symbols or anything else).
Your own elements are stored in the user collection.
![](https://download.qelectrotech.org/qet/images-features/0080.png "Element editor")

Quickly find what you need

All collections can quickly be searched with the integrated search engine.
Furthermore, the search request can be restricted to the folder of your choice.
![](https://download.qelectrotech.org/qet/images-features/0090.png "Search engine")

Easily draw complex schematics

To add an element on the drawing area, it only needs a drag & drop from the collection panel.
![](https://download.qelectrotech.org/qet/images-features/0102.png "Drag and drop")

Elements are automatically connected if they are aligned, or connected from point to point by pulling a conductor with the mouse.
![](https://download.qelectrotech.org/qet/images-features/0105.png "Conductor connections")

The path of every conductor can be modified by moving its handles with the mouse.
![](https://download.qelectrotech.org/qet/images-features/0107.png "Conductor handles")
And of course, you can accurately zoom with the mouse wheel over the drawing area to catch the smallest details.

Link elements together to create cross references

Several types of element can be linked together to display a cross reference text.
All types of cross references are automatically updated on the fly, you don't need to think about them if you make changes.
![](https://download.qelectrotech.org/qet/images-features/0112.png "Cross ref elements")
To speed up your work, linkable elements are easily searched and shown.
![](https://download.qelectrotech.org/qet/images-features/0115.png  "Cross ref search")


Export informations to a parts list
Informations of all elements in the project can be exported to a .csv file that can be read and edited by any spreadsheet application.
![](https://download.qelectrotech.org/qet/images-features/0122.png "Element informations")

This way, you can make your own parts list or bill of material using the full power of a spreadsheet program.
![](https://download.qelectrotech.org/qet/images-features/0125.png "Spreadsheet")
Print to pdf and/or export your work to images

Your whole documentation or only selected parts of it can be printed to a real printer or to a pdf file.
Alternatively, you can export to vector (svg) or pixel (png, jpg, bmp) format images.

### And much more:

*   open and edit several projects at the same time
*   import images (.bmp, .jpg, .png, .svg) in your diagrams
*   add basic shapes (lines, rectangles, ellipses, polygons) to your drawings
*   edit the thickness, the line style and the color of conductors
*   define some autonum patterns for conductors, symbols and folios
*   take advantage of the open xml standard of elements and projects to create custom tools
*   search and replace Widget (Ctrl + F) in entire project
*   conductors num can be exported to csv file.
*   ***

Nomenclature

A new nomenclature tool appears in the menu: project -> Add a nomenclature.
The nomenclature is presented in the form of a configurable table separated into two parts: the display (the form) and the content (the background).
- Display: the size and position of the table, the margins between text and the table cell, the alignment of the text in the cells and the font. The configuration of the table headers and the table itself are separate.
- Content: the information to display in the table and the order in which it should be displayed.

![](https://download.qelectrotech.org/qet/images_depeche_linuxfr/08/dialogue_nomenclature.png "nomenclature dialogue")

In order to speed up the establishment of a nomenclature, it is possible to export / import the display and content configurations separately. This is the "Configuration" part that can be seen in the photos above.

Behind the scenes, an SQLite database does the work, so setting up the content is nothing more or less than an SQL query created using a dialog (screenshot by right).
The SQL query is configured as follows (from top to bottom in the screenshot):
- “Available information”: the information to display;
- "Filter": filter the information (is not empty, is empty, contains, does not contain, is equal to, is not equal to) only one filter can be applied per information, it is not possible combine several;
- "Type of elements": allows you to filter on what type of element you want to obtain information.

At the bottom, a checkmark "SQL query" allows you to edit a personalized query, if the basic options are not sufficient.

When a nomenclature is too large to be contained in a single folio, it is possible to separate it on several folios, the tables of each folio are then linked together. When creating a nomenclature, this option is activated by default, which has the effect of adding the necessary number of folios, adding a table in each of them and linking them together.

Finally two buttons are available in the property panel:
- "Fit the table to the folio": positions and adjusts the size and determines the number of rows in the table in relation to the folio;
- "Apply geometry to all tables linked to this one": applies the three properties mentioned above to all linked tables in order to save time and maintain aesthetic consistency.

And to finish a table
![](https://download.qelectrotech.org/qet/images_depeche_linuxfr/08/tableau.png "table")


Summary

The old summary has been completely removed from the code in order to make room for the new one which is exactly the same as the nomenclature (a large amount of the code is common), with the exception of the SQL query (and its dialog to configure it) which offers specific information for editing a summary.

Export of the internal database

The database used by the nomenclature and the summary can be exported in a “.sqlite” file.
Currently this is irrelevant, as the function was created during development for debugging purposes, we left it.
Note that the database will become increasingly important in the future of Qet.


Export of the wiring list

In order to be able to use the wiring number printers more easily, the names of conductors can be exported in CSV format, the export respects the quantity of conductors in order to print the right quantity of numbers, for example a potential numbered 240 composed of 3 wires will give 6 × 240 (2 numbers per wire × 3 wires) in the CSV. 



### Story

The QElectroTech project was founded in 2007 by two french students, Xavier and Benoit.
Xavier developed the base application itself and made all technical choices about the development.
The first version of QET (0.1) was released on 09.03.2008.
However, both Xavier and Benoit do not participate anymore in the project since 2013.

Following this period, new developers and contributors took over the project and kept it alive.
The development and the many translations are actively maintained.
New functionalities and evolutions are planned to make QET ever better.

Nowadays, QET is not only used by many individuals, teachers and students but also by professional electricians and companies all over the world.

  
### Donate Money

If you love QElectroTech, you can help developers to buy new hardware to test
and implement new features. Thanks in advance for your generous donations.

For more information, look at [Paypal](https://www.paypal.com/donate/?cmd=_s-xclick&hosted_button_id=ZZHC9D7C3MDPC&ssrt=1694606609672)




# Licenses and attribution

OrbitWatch application source is licensed under GPL-3.0-or-later; see LICENSE.
Third-party libraries and imagery retain their own terms. This local project is
not affiliated with or endorsed by NASA or The Qt Company. No code is published
or uploaded by the application, and the globe works offline.

## Qt (dynamic linking, Ubuntu packages)

Reviewed the official module licensing documentation before adding the 3D view:

- Qt Quick 3D (including built-in sphere geometry and shaders): **GPLv3** or
  commercial. The open-source GPLv3 option is used for this project; do not
  assume LGPL applies to this module.
  https://doc.qt.io/qt-6/qtquick3d-index.html#licenses-and-attributions
- Qt Quick Widgets: LGPLv3 / GPL options / commercial, per module documentation.
  https://doc.qt.io/qt-6/qtquickwidgets-index.html#licenses-and-attributions
- Qt Quick / QML and Qt Base (Core, Gui, Widgets, Concurrent, OpenGL integration):
  use their compatible open-source licensing options, not a commercial license.
  Qt Test (from Qt Base) is used only by the Earth interaction test executable.
  https://doc.qt.io/qt-6/qtquick-index.html#licenses-and-attributions
  https://doc.qt.io/qt-6/qtqml-index.html#licenses-and-attributions
  https://doc.qt.io/qt-6/licensing.html

Exact package copyright notices are installed under /usr/share/doc/<package>/copyright.
Qt Quick 3D also lists permissively licensed third-party components on its module
page. Retain relevant library notices and corresponding-source obligations if
redistributing binaries. This repository does not bundle the Qt libraries.
The QML scene and atmosphere shaders are original OrbitWatch source, not copied
from Qt examples. No Qt Quick 3D Helpers dependency is used by the source scene.

## Earth surface image

File: assets/earth-day.jpg (original JPEG, unmodified).
Title: Blue Marble: Land Surface, Shallow Water, and Shaded Topography (2002).
Credit: NASA Goddard Space Flight Center / Reto Stöckli (land surface, shallow
water); Robert Simmon (ocean color/compositing). Data: MODIS and supporting teams.

Original NASA asset:
https://eoimages.gsfc.nasa.gov/images/imagerecords/57000/57752/land_shallow_topo_2048.jpg
NASA catalog record (legacy URL, may redirect):
https://visibleearth.nasa.gov/view.php?id=57752
NASA Blue Marble background and credits:
https://science.nasa.gov/earth/earth-observatory/the-blue-marble-true-color-global-imagery-at-1km-resolution/
NASA media usage policy:
https://www.nasa.gov/nasa-brand-center/images-and-media/

NASA-produced imagery is generally not subject to copyright in the United States,
subject to the exceptions in NASA's policy. This NASA-credited asset carries no
identified third-party copyright restriction and is used for informational
visualization with attribution. It is not relicensed as OrbitWatch GPL source.
No NASA logos, insignia, people, or endorsement claims are included.

The star field is procedurally drawn by original QML code; no external star map,
commercial texture, font file, or 3D model is bundled.

## Earth surface with clouds (current globe texture)

File: assets/earth-clouds.jpg, unmodified NASA JPEG.
Title: The Blue Marble: Land Surface, Ocean Color, Sea Ice and Clouds (2002).
Credit: NASA Goddard Space Flight Center / Reto Stöckli; enhancements by
Robert Simmon; MODIS science and supporting teams.
https://eoimages.gsfc.nasa.gov/images/imagerecords/57000/57735/land_ocean_ice_cloud_2048.jpg
https://visibleearth.nasa.gov/images/57735/the-blue-marble-land-surface-ocean-color-sea-ice-and-clouds
The NASA media policy above applies. This is historical composite imagery, not
live weather. The earlier earth-day.jpg is retained but not used by the globe.
The current scene uses a near-black background without the earlier decorative stars.

## Earth city lights (night-side visualization)

File: assets/earth-night.jpg, original unmodified 2400×1200 JPEG.
Credit: C. Mayhew & R. Simmon (NASA/GSFC), NOAA/NGDC, DMSP Digital Archive.
The image is historical composite data, not a live observation or simulation.
Primary credit record: https://apod.nasa.gov/apod/ap020810.html
NASA-hosted asset:
https://eoimages.gsfc.nasa.gov/images/imagerecords/55000/55167/earth_lights_lrg.jpg
Reuse follows the NASA media usage policy linked above; no third-party copyright
restriction is identified for this NASA/NOAA government-produced image.
The shader gives city lights a warm presentation color and blends them only onto
the night side. The source JPEG remains unchanged. No NASA endorsement is implied.

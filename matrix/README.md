# `matrix`

### A simple image and marquee viewer for LED panels.

The `matrix` sketch displays a provided splash image and marquee on a panel of connected single-wire LED strips.

## Basic configuration

In the `matrix.ino` file, change the values of these `#define` fields:
- `PIN_STRIP1` - the pin number that the LED panel is connected to
- `MATRIX_WIDTH` - the width of the LED panel, in pixels
- `MATRIX_HEIGHT` - the height of the LED panel, in pixels
- `BRIGHTNESS` - the global brightness of the LED panel, from 0-255 (warning: the higher you go, the less noticable difference there is between brightness levels, but power consumption is linear)
- `FRAME_DURATION` - how frequently the display should be updated, in milliseconds
- `SPLASH_FRAMES` - how many frames the splash image should show for before changing to marquee mode (-1 will disable marquee mode, 0 will disable the splash image)

## Changing the splash/marquee images

`matrix` accepts PNG images converted into C files with the ImgConv utility.

The `data` folder beside the `matrix.ino` file stores the source PNG files. The image dimensions must match the dimensions of the LED panel. Our 2022 robot's panel dimensions are 9x8.

To convert your image, go to the repository's root directory and open the `libraries/MatrixUtils` folder on the command line or terminal. You will need Windows (because of the way that the tool interprets paths), Python, and pillow (`pip install pillow`) installed to run the converter. The following steps will guide you in structuring the command to input -- don't hit enter until you reach the end:
- Type `python .\imgconv.py`, which will run the ImgConv utility.
- Add a space and append `..\..\matrix\data\<FILENAME>.png`, replacing `<FILENAME>` with the actual file name of your source PNG file.
- Add a space and append `-o ..\images\<TYPE>.h`, replacing `<TYPE>` with either `splash` or `marquee`, depending on the mode you want your image to run in.
- Add a space and append `-f string`.
- If your LED panel has the strips arranged horizontally, add a space and append `-l row`. If your LED panel has the strips arranged vertically, add a space and append `-l column`. Our 2022 robot's panels use column mode.
- If your LED panel is wired in zigzag formation, add a space and append `-Z` to enable zigzag mode. Our 2022 robot's panels are **not** wired in zigzag formation. Zigzag panel configurations string the strips like this:
```
→→→→→→→→↓
↓←←←←←←←←
→→→→→→→→↓
←←←←←←←←←
```
- **VERY IMPORTANT!** If you are running `matrix` on an Arduino, add a space and append `-A` to enable Arduino mode.

Once you have the entire command typed out, go ahead and hit enter. When it completes without error, your image has been successfully converted. It will be automatically bundled into the `matrix` sketch when uploading to the Arduino.
